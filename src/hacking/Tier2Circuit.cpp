#include "Tier2Circuit.h"
#include <SFML/Window/Keyboard.hpp>
#include <cstring>
#include <cmath>

static const sf::Color CYAN   = sf::Color(0x00, 0xFF, 0xEE, 0xFF);
static const sf::Color VIOLET = sf::Color(0xAA, 0x00, 0xFF, 0xFF);
static const sf::Color DANGER = sf::Color(0xFF, 0x00, 0x38, 0xFF);
static const sf::Color DIM    = sf::Color(0x1A, 0x28, 0x40, 0xFF);
static const sf::Color UI_BG  = sf::Color(0x0A, 0x10, 0x20, 0xEE);
static const sf::Color YELLOW = sf::Color(0xFF, 0xE6, 0x00, 0xFF);

// A path from (0,0) to (4,4) must exist — generate blocked cells that don't sever all paths.
// Simple approach: pre-bake 3 legal layouts and pick by seed.
static const bool LAYOUTS[3][5][5] = {
    { // Layout 0
        {0,0,0,1,0},
        {0,1,0,0,0},
        {0,1,1,0,1},
        {0,0,0,0,1},
        {1,1,0,0,0},
    },
    { // Layout 1
        {0,0,1,0,0},
        {0,0,1,0,1},
        {1,0,0,0,0},
        {1,0,1,1,0},
        {0,0,0,1,0},
    },
    { // Layout 2
        {0,0,0,0,1},
        {1,0,1,0,0},
        {0,0,1,0,1},
        {0,1,0,0,0},
        {0,1,1,0,0},
    },
};

void Tier2Circuit::init(int randomSeed) {
    m_timeLeft = DURATION;
    m_col = START_C;
    m_row = START_R;
    m_grace = 0.25f;
    m_upPrev = m_downPrev = m_leftPrev = m_rightPrev = true;
    std::memset(m_visited, false, sizeof(m_visited));

    int seed = ((randomSeed ^ (randomSeed >> 4)) & 0x7FFFFFFF) % 3;
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            m_blocked[r][c] = LAYOUTS[seed][r][c];

    m_blocked[START_R][START_C] = false;
    m_blocked[GOAL_R][GOAL_C]   = false;
    m_visited[START_R][START_C] = true;
}

IHackMinigame::Result Tier2Circuit::update(float dt) {
    m_timeLeft -= dt;
    if (m_timeLeft <= 0.f) return Result::FAILURE;

    if (m_grace > 0.f) {
        m_grace -= dt;
        m_upPrev    = sf::Keyboard::isKeyPressed(sf::Keyboard::Up)    || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
        m_downPrev  = sf::Keyboard::isKeyPressed(sf::Keyboard::Down)  || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
        m_leftPrev  = sf::Keyboard::isKeyPressed(sf::Keyboard::Left)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        m_rightPrev = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
        return Result::RUNNING;
    }

    bool upNow    = sf::Keyboard::isKeyPressed(sf::Keyboard::Up)    || sf::Keyboard::isKeyPressed(sf::Keyboard::W);
    bool downNow  = sf::Keyboard::isKeyPressed(sf::Keyboard::Down)  || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    bool leftNow  = sf::Keyboard::isKeyPressed(sf::Keyboard::Left)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool rightNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);

    int nc = m_col, nr = m_row;

    if (leftNow  && !m_leftPrev)  nc--;
    if (rightNow && !m_rightPrev) nc++;
    if (upNow    && !m_upPrev)    nr--;
    if (downNow  && !m_downPrev)  nr++;

    m_leftPrev  = leftNow;
    m_rightPrev = rightNow;
    m_upPrev    = upNow;
    m_downPrev  = downNow;

    if (nc != m_col || nr != m_row) {
        if (nc >= 0 && nc < COLS && nr >= 0 && nr < ROWS
            && !m_blocked[nr][nc]
            && !m_visited[nr][nc]) {
            m_col = nc;
            m_row = nr;
            m_visited[nr][nc] = true;
            if (m_col == GOAL_C && m_row == GOAL_R)
                return Result::SUCCESS;
        }
    }

    return Result::RUNNING;
}

void Tier2Circuit::render(sf::RenderWindow& window, const sf::Font& font) const {
    float W = static_cast<float>(window.getSize().x);
    float H = static_cast<float>(window.getSize().y);
    float cx = W * 0.5f, cy = H * 0.42f;

    // Panel
    sf::RectangleShape panel({380.f, 280.f});
    panel.setFillColor(UI_BG);
    panel.setOutlineColor(CYAN);
    panel.setOutlineThickness(2.f);
    panel.setPosition(cx - 190.f, cy - 80.f);
    window.draw(panel);

    // Title
    sf::Text title("[ CIRCUIT ROUTING ]", font, 16);
    title.setFillColor(CYAN);
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(cx - tb.width * 0.5f, cy - 74.f);
    window.draw(title);

    // Timer bar
    float timerFrac = std::max(0.f, m_timeLeft / DURATION);
    sf::RectangleShape timerBg({360.f, 6.f});
    timerBg.setFillColor(DIM);
    timerBg.setPosition(cx - 180.f, cy - 50.f);
    window.draw(timerBg);
    sf::RectangleShape timerFill({360.f * timerFrac, 6.f});
    timerFill.setFillColor(timerFrac < 0.3f ? DANGER : CYAN);
    timerFill.setPosition(cx - 180.f, cy - 50.f);
    window.draw(timerFill);

    // Grid
    constexpr float CELL = 44.f, PAD = 6.f;
    float gridW = COLS * CELL + (COLS - 1) * PAD;
    float gx = cx - gridW * 0.5f;
    float gy = cy - 32.f;

    for (int r = 0; r < ROWS; ++r) {
        for (int c = 0; c < COLS; ++c) {
            float sx = gx + c * (CELL + PAD);
            float sy = gy + r * (CELL + PAD);

            sf::RectangleShape cell({CELL, CELL});
            if (m_blocked[r][c]) {
                cell.setFillColor(sf::Color(0x20, 0x20, 0x28, 0xFF));
                cell.setOutlineColor(sf::Color(0x30, 0x30, 0x40, 0xFF));
            } else if (c == GOAL_C && r == GOAL_R) {
                cell.setFillColor(sf::Color(0x22, 0x00, 0x44, 0xFF));
                cell.setOutlineColor(VIOLET);
            } else if (c == m_col && r == m_row) {
                cell.setFillColor(sf::Color(0x00, 0x22, 0x22, 0xFF));
                cell.setOutlineColor(CYAN);
            } else if (m_visited[r][c]) {
                cell.setFillColor(sf::Color(0x00, 0x18, 0x18, 0xFF));
                cell.setOutlineColor(sf::Color(0x00, 0x88, 0x80, 0xFF));
            } else {
                cell.setFillColor(sf::Color(0x05, 0x08, 0x10, 0xFF));
                cell.setOutlineColor(DIM);
            }
            cell.setOutlineThickness(1.5f);
            cell.setPosition(sx, sy);
            window.draw(cell);

            // Label start/goal
            if (c == START_C && r == START_R) {
                sf::Text lbl("IN", font, 10);
                lbl.setFillColor(YELLOW);
                lbl.setPosition(sx + 4.f, sy + 4.f);
                window.draw(lbl);
            }
            if (c == GOAL_C && r == GOAL_R) {
                sf::Text lbl("OUT", font, 10);
                lbl.setFillColor(VIOLET);
                lbl.setPosition(sx + 4.f, sy + 4.f);
                window.draw(lbl);
            }
        }
    }

    // Hint
    sf::Text hint("ARROW KEYS: NAVIGATE    REACH [OUT]", font, 11);
    hint.setFillColor(sf::Color(0x60, 0x80, 0xA0, 0xFF));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setPosition(cx - hb.width * 0.5f, cy + 170.f);
    window.draw(hint);
}
