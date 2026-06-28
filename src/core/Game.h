#pragma once
#include <SFML/Graphics.hpp>
#include "InputMap.h"
#include "GameState.h"

class Game {
public:
    Game();
    ~Game() = default;

    void run();

private:
    static constexpr float FIXED_DT = 1.0f / 60.0f;

    void processEvents();
    void updateFpsTitle(float dt);

    sf::RenderWindow  m_window;
    InputMap          m_input;
    GameStateManager  m_states;
    sf::Clock         m_clock;
    float             m_fpsTimer     = 0.0f;
    int               m_fpsCounter   = 0;
    int               m_fps          = 0;
};
