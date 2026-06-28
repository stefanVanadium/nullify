#include "Game.h"
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "player/Player.h"
#include "player/PlayerStateMachine.h"
#include "world/TileMap.h"
#include "world/LevelLoader.h"
#include "world/Camera.h"
#include "rendering/Renderer.h"
#include <memory>
#include <string>
#include <algorithm>

// PlayState — owns all simulation systems for V0.1
class PlayState : public IGameState {
public:
    PlayState(sf::RenderWindow& window, InputMap& input)
        : m_window(window)
        , m_input(input)
        , m_physics()
        , m_renderer()
        , m_player(m_world, m_physics)
        , m_psm(m_world, m_input, m_physics)
        , m_camera(m_world)
    {}

    void enter() override {
        LevelLoader loader;
        auto spawn = loader.load("assets/levels/1-1.json", m_world, m_physics);
        if (spawn) {
            m_player.spawn(spawn->x, spawn->y);
        }
        m_camera.setLevelBounds(
            60 * 32.0f,   // width in pixels
            20 * 32.0f    // height in pixels
        );
    }

    void exit() override {}

    void update(float dt) override {
        m_psm.update(dt);
        m_player.update(dt, m_input);
        m_physics.update(dt);
        m_camera.update(dt);
    }

    void render(sf::RenderWindow& window, float alpha) override {
        m_camera.apply(window);
        m_renderer.render(window, m_world, alpha);
    }

private:
    sf::RenderWindow&   m_window;
    InputMap&           m_input;
    World               m_world;
    PhysicsSystem       m_physics;
    Renderer            m_renderer;
    Player              m_player;
    PlayerStateMachine  m_psm;
    Camera              m_camera;
};

// ---- Game ----

Game::Game()
    : m_window(sf::VideoMode(1280, 720), "NULLIFY v0.1",
               sf::Style::Titlebar | sf::Style::Close)
{
    m_window.setVerticalSyncEnabled(false);
    m_window.setFramerateLimit(144);

    m_states.pushState(std::make_unique<PlayState>(m_window, m_input));
}

void Game::run() {
    // Apply the initial pending push before the loop checks empty()
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
        m_window.setTitle("NULLIFY v0.1 | " + std::to_string(m_fps) + " FPS");
    }
}
