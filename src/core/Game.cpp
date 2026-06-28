#include "Game.h"
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "player/Player.h"
#include "player/PlayerStateMachine.h"
#include "player/WeaponSystem.h"
#include "player/HackSystem.h"
#include "world/TileMap.h"
#include "world/LevelLoader.h"
#include "world/Camera.h"
#include "rendering/Renderer.h"
#include "rendering/ParallaxSystem.h"
#include "rendering/ShaderManager.h"
#include "rendering/ParticleSystem.h"
#include "rendering/RagdollSystem.h"
#include "player/StealthSystem.h"
#include "enemies/EnemyManager.h"
#include "ui/HUD.h"
#include "ecs/Components.h"
#include <memory>
#include <string>
#include <cstdio>
#include <algorithm>

class PlayState : public IGameState {
public:
    PlayState(sf::RenderWindow& window, InputMap& input,
              std::string levelPath = "assets/levels/1-1.json")
        : m_window(window)
        , m_input(input)
        , m_levelPath(std::move(levelPath))
        , m_physics()
        , m_player(m_world, m_physics)
        , m_psm(m_world, m_input, m_physics)
        , m_weapon(m_world, m_physics)
        , m_camera(m_world)
        , m_enemyMgr(m_world, m_physics)
        , m_ragdolls(m_physics)
        , m_stealth(m_world, m_physics)
    {
        m_damagedHandle = EventBus::on<PlayerDamagedEvent>([this](const PlayerDamagedEvent&) {
            m_caTimer = 0.8f;
        });
    }

    ~PlayState() {
        EventBus::unsubscribe<PlayerDamagedEvent>(m_damagedHandle);
    }

    void enter() override {
        sf::Vector2u winSize = m_window.getSize();

        LevelLoader loader;
        auto levelData = loader.load(m_levelPath,
                                     m_world, m_physics, m_tileMap);
        if (!levelData) return;

        m_player.spawn(levelData->playerSpawn.x, levelData->playerSpawn.y);

        m_enemyMgr.buildNavMesh(levelData->collision,
                                 levelData->width, levelData->height, levelData->tileSize);
        for (const auto& es : levelData->enemies)
            m_enemyMgr.spawnEnemy(es.type, es.x, es.y, es.waypoints);

        m_camera.setLevelBounds(
            static_cast<float>(levelData->width  * levelData->tileSize),
            static_cast<float>(levelData->height * levelData->tileSize)
        );

        m_parallax.init(winSize);
        m_shaders.loadAll("assets/shaders");
        m_renderer.init(winSize);
        m_hud.init("assets/fonts/ShareTechMono-Regular.ttf");
    }

    void exit() override {}

    void update(float dt) override {
        m_gameTime += dt;

        if (m_caTimer > 0.f)
            m_caTimer = std::max(0.f, m_caTimer - dt);

        uint32_t pid = m_world.findPlayer();

        // Always update hack system (owns minigame lifecycle)
        if (pid != static_cast<uint32_t>(MAX_ENTITIES))
            m_hackSystem.update(dt, pid, m_world, m_input);

        // Freeze physics + AI + weapons while minigame is active
        if (m_hackSystem.isHacking()) {
            m_camera.update(dt);
            return;
        }

        m_psm.update(dt);
        m_player.update(dt, m_input);
        m_physics.update(dt);
        m_camera.update(dt);
        m_enemyMgr.update(dt);
        m_ragdolls.update(dt);
        m_particles.update(dt);

        m_stealth.update(dt, pid, m_input);
        if (pid != static_cast<uint32_t>(MAX_ENTITIES)) {
            // Weapon pickup detection (AABB overlap with player)
            if (m_world.hasComponent<Transform>(pid)) {
                const auto& pt = m_world.getComponent<Transform>(pid);
                for (uint32_t id = 0; id < MAX_ENTITIES; ++id) {
                    if (!m_world.isAlive(id)) continue;
                    if (!m_world.hasComponent<PickupTag>(id)) continue;
                    if (!m_world.hasComponent<Transform>(id)) continue;
                    const auto& it = m_world.getComponent<Transform>(id);
                    float dx = std::abs(it.x - pt.x), dy = std::abs(it.y - pt.y);
                    if (dx < 28.f && dy < 32.f) {
                        auto wt = static_cast<WeaponType>(m_world.getComponent<PickupTag>(id).weaponTypeInt);
                        m_weapon.unlockWeapon(wt);
                        EventBus::emit(WeaponSwitchedEvent{ static_cast<int>(wt) });
                        m_world.destroyEntity(id);
                    }
                }
            }

            if (m_input.isPressed(Action::WeaponNext)) m_weapon.switchWeapon(+1);
            if (m_input.isPressed(Action::WeaponPrev)) m_weapon.switchWeapon(-1);

            if (m_input.isHeld(Action::Fire)) {
                sf::Vector2f mouseW = m_camera.screenToWorld(m_input.mouseScreenPos(), m_window);
                if (m_world.hasComponent<Transform>(pid)) {
                    const auto& t = m_world.getComponent<Transform>(pid);
                    sf::Vector2f playerCenter = {t.x + 12.f, t.y + 24.f};
                    m_weapon.fire(playerCenter, mouseW, pid);
                }
            }
        }
        m_weapon.update(dt);
    }

    void render(sf::RenderWindow& window, float alpha) override {
        m_camera.apply(window);

        float hpRatio = 1.0f;
        uint32_t pid  = m_world.findPlayer();
        sf::Vector2f mouseWorld = m_camera.screenToWorld(m_input.mouseScreenPos(), window);

        if (pid != static_cast<uint32_t>(MAX_ENTITIES) && m_world.hasComponent<Health>(pid)) {
            const auto& hp = m_world.getComponent<Health>(pid);
            hpRatio = static_cast<float>(hp.current) / static_cast<float>(std::max(hp.max, 1));
        }

        RenderEffects fx;
        fx.caIntensity   = (m_caTimer > 0.f) ? (m_caTimer / 0.8f) : 0.f;
        fx.hackIntensity = m_hackSystem.hackIntensity();
        fx.gameTime      = m_gameTime;

        m_lastDrawCalls = m_renderer.render(window, m_world, alpha, hpRatio,
                                            mouseWorld, m_tileMap,
                                            m_parallax, m_shaders, m_weapon,
                                            m_enemyMgr, m_particles, m_ragdolls,
                                            m_stealth, fx);

        // Minigame overlay (renders itself in screen space)
        if (m_hackSystem.isHacking() && m_hackSystem.activeMinigame()) {
            if (const sf::Font* f = m_hud.font()) {
                window.setView(window.getDefaultView());
                sf::RectangleShape dimmer(sf::Vector2f(window.getSize()));
                dimmer.setFillColor(sf::Color(0x00, 0x00, 0x00, 0xAA));
                window.draw(dimmer);
                m_hackSystem.activeMinigame()->render(window, *f);
            }
        }

        if (pid != static_cast<uint32_t>(MAX_ENTITIES)
            && m_world.hasComponent<Health>(pid)
            && m_world.hasComponent<Weapon>(pid)) {
            window.setView(window.getDefaultView());
            m_hud.render(window,
                         m_world.getComponent<Health>(pid),
                         m_world.getComponent<Weapon>(pid),
                         m_weapon,
                         m_enemyMgr.alertLevel());
        }
    }

    int lastDrawCalls() const { return m_lastDrawCalls; }

private:
    sf::RenderWindow& m_window;
    InputMap&         m_input;
    std::string       m_levelPath;
    World             m_world;
    PhysicsSystem     m_physics;
    Player            m_player;
    PlayerStateMachine m_psm;
    WeaponSystem      m_weapon;
    HackSystem        m_hackSystem;
    Camera            m_camera;
    TileMap           m_tileMap;
    EnemyManager      m_enemyMgr;
    ParallaxSystem    m_parallax;
    ShaderManager     m_shaders;
    Renderer          m_renderer;
    ParticleSystem    m_particles;
    RagdollSystem     m_ragdolls;
    StealthSystem     m_stealth;
    HUD               m_hud;
    int               m_lastDrawCalls = 0;
    float             m_caTimer       = 0.f;
    float             m_gameTime      = 0.f;
    EventBus::Handle  m_damagedHandle = 0;
};

// ---- Game ----

Game::Game()
    : m_window(sf::VideoMode(1280, 720), "NULLIFY v0.3",
               sf::Style::Titlebar | sf::Style::Close)
{
    m_window.setVerticalSyncEnabled(false);
    m_window.setFramerateLimit(144);

    m_states.pushState(std::make_unique<PlayState>(m_window, m_input));
}

void Game::run() {
    m_states.update(0.0f);

    float accumulator = 0.0f;

    while (m_window.isOpen() && !m_states.empty()) {
        float dt = std::min(m_clock.restart().asSeconds(), 0.25f);
        accumulator += dt;

        processEvents();

        while (accumulator >= FIXED_DT) {
            m_states.update(FIXED_DT);
            accumulator -= FIXED_DT;
        }

        float alpha = accumulator / FIXED_DT;

        m_window.clear(sf::Color(0x05, 0x08, 0x10));
        m_states.render(m_window, alpha);
        m_window.display();

        m_input.flush();
        updateFpsTitle(dt);
    }
}

void Game::processEvents() {
    sf::Event event{};
    while (m_window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            m_window.close();
        if (event.type == sf::Event::KeyPressed) {
            switch (event.key.code) {
            case sf::Keyboard::Escape:
                m_window.close();
                break;
            case sf::Keyboard::F1:
                m_states.replaceState(std::make_unique<PlayState>(m_window, m_input, "assets/levels/1-1.json"));
                break;
            case sf::Keyboard::F2:
                m_states.replaceState(std::make_unique<PlayState>(m_window, m_input, "assets/levels/1-2.json"));
                break;
            case sf::Keyboard::F3:
                m_states.replaceState(std::make_unique<PlayState>(m_window, m_input, "assets/levels/1-3.json"));
                break;
            default: break;
            }
        }
        m_input.processEvent(event);
    }
}

void Game::updateFpsTitle(float dt) {
    m_fpsTimer += dt;
    ++m_fpsCounter;
    if (m_fpsTimer >= 0.5f) {
        m_fps        = static_cast<int>(m_fpsCounter / m_fpsTimer);
        m_fpsTimer   = 0.0f;
        m_fpsCounter = 0;

        int drawCalls = 0;
        if (auto* ps = dynamic_cast<PlayState*>(m_states.top()))
            drawCalls = ps->lastDrawCalls();

        char title[64];
        std::snprintf(title, sizeof(title),
                      "NULLIFY v0.3 | %d FPS | %d draw calls", m_fps, drawCalls);
        m_window.setTitle(title);
    }
}
