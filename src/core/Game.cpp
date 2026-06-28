#include "Game.h"
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "player/Player.h"
#include "player/PlayerStateMachine.h"
#include "player/WeaponSystem.h"
#include "world/TileMap.h"
#include "world/LevelLoader.h"
#include "world/Camera.h"
#include "rendering/Renderer.h"
#include "rendering/ParallaxSystem.h"
#include "rendering/ShaderManager.h"
#include "enemies/EnemyManager.h"
#include "ui/HUD.h"
#include "ecs/Components.h"
#include <memory>
#include <cstdio>
#include <algorithm>

class PlayState : public IGameState {
public:
    PlayState(sf::RenderWindow& window, InputMap& input)
        : m_window(window)
        , m_input(input)
        , m_physics()
        , m_player(m_world, m_physics)
        , m_psm(m_world, m_input, m_physics)
        , m_weapon(m_world, m_physics)
        , m_camera(m_world)
        , m_enemyMgr(m_world, m_physics)
    {}

    void enter() override {
        sf::Vector2u winSize = m_window.getSize();

        // Load level — builds tile vertex array into m_tileMap
        LevelLoader loader;
        auto levelData = loader.load("assets/levels/1-1.json",
                                     m_world, m_physics, m_tileMap);
        if (!levelData) return;

        m_player.spawn(levelData->playerSpawn.x, levelData->playerSpawn.y);

        // Build navmesh + spawn enemies
        m_enemyMgr.buildNavMesh(levelData->collision,
                                 levelData->width, levelData->height, levelData->tileSize);
        for (const auto& es : levelData->enemies)
            m_enemyMgr.spawnScout(es.x, es.y, es.waypoints);

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
        m_psm.update(dt);
        m_player.update(dt, m_input);
        m_physics.update(dt);
        m_camera.update(dt);
        m_enemyMgr.update(dt);

        // Weapon: fire on mouse click
        uint32_t pid = m_world.findPlayer();
        if (pid != static_cast<uint32_t>(MAX_ENTITIES)) {
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

        m_lastDrawCalls = m_renderer.render(window, m_world, alpha, hpRatio,
                                            mouseWorld, m_tileMap,
                                            m_parallax, m_shaders, m_weapon);

        // HUD: drawn in screen space (renderer resets view to default for overlays)
        if (pid != static_cast<uint32_t>(MAX_ENTITIES)
            && m_world.hasComponent<Health>(pid)
            && m_world.hasComponent<Weapon>(pid)) {
            window.setView(window.getDefaultView());
            m_hud.render(window,
                         m_world.getComponent<Health>(pid),
                         m_world.getComponent<Weapon>(pid),
                         m_enemyMgr.alertLevel());
        }
    }

    int lastDrawCalls() const { return m_lastDrawCalls; }

private:
    sf::RenderWindow& m_window;
    InputMap&         m_input;
    World             m_world;
    PhysicsSystem     m_physics;
    Player            m_player;
    PlayerStateMachine m_psm;
    WeaponSystem      m_weapon;
    Camera            m_camera;
    TileMap           m_tileMap;
    EnemyManager      m_enemyMgr;
    ParallaxSystem    m_parallax;
    ShaderManager     m_shaders;
    Renderer          m_renderer;
    HUD               m_hud;
    int               m_lastDrawCalls = 0;
};

// ---- Game ----

Game::Game()
    : m_window(sf::VideoMode(1280, 720), "NULLIFY v0.2",
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
        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::Escape)
            m_window.close();
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

        // Retrieve draw call count from PlayState
        int drawCalls = 0;
        if (auto* ps = dynamic_cast<PlayState*>(m_states.top()))
            drawCalls = ps->lastDrawCalls();

        char title[64];
        std::snprintf(title, sizeof(title),
                      "NULLIFY v0.2 | %d FPS | %d draw calls", m_fps, drawCalls);
        m_window.setTitle(title);
    }
}
