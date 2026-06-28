#pragma once
#include "HackMinigame.h"

// Tier 2: 5×5 grid circuit routing — 8 second timer.
// Navigate from top-left to bottom-right avoiding blocked nodes.
// Arrow keys move. Cannot revisit cells. Reach goal → SUCCESS.
class Tier2Circuit final : public IHackMinigame {
public:
    static constexpr float DURATION = 8.0f;
    static constexpr int   COLS = 5, ROWS = 5;
    static constexpr int   START_C = 0, START_R = 0;
    static constexpr int   GOAL_C  = 4, GOAL_R  = 4;

    void init(int randomSeed);

    Result update(float dt) override;
    void   render(sf::RenderWindow& window, const sf::Font& font) const override;

private:
    bool m_blocked[ROWS][COLS]{};
    bool m_visited[ROWS][COLS]{};
    int  m_col = START_C, m_row = START_R;

    float m_grace    = 0.25f;
    bool m_upPrev    = false;
    bool m_downPrev  = false;
    bool m_leftPrev  = false;
    bool m_rightPrev = false;
};
