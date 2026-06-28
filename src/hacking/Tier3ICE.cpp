#include "Tier3ICE.h"
#include <SFML/Window/Keyboard.hpp>
#include <cstring>
#include <cmath>
#include <algorithm>

static const sf::Color CYAN   = sf::Color(0x00, 0xFF, 0xEE, 0xFF);
static const sf::Color VIOLET = sf::Color(0xAA, 0x00, 0xFF, 0xFF);
static const sf::Color DANGER = sf::Color(0xFF, 0x00, 0x38, 0xFF);
static const sf::Color DIM    = sf::Color(0x1A, 0x28, 0x40, 0xFF);
static const sf::Color UI_BG  = sf::Color(0x0A, 0x10, 0x20, 0xEE);
static const sf::Color YELLOW = sf::Color(0xFF, 0xE6, 0x00, 0xFF);
static const sf::Color WHITE  = sf::Color(0xE0, 0xEE, 0xF8, 0xFF);

void Tier3ICE::init(int /*randomSeed*/) {
    m_timeLeft    = DURATION;
    m_phase       = Phase::CHOOSE;
    m_actionTimer = 0.f;
    m_cursor      = 0;
    m_action      = 0;
    m_lastHit     = -1;
    m_grace       = 0.25f;
    m_leftPrev    = true;
    m_rightPrev   = true;
    m_confPrev    = true;
    for (int i = 0; i < ICE_LAYERS; ++i)
        m_iceHP[i] = ICE_HP_MAX;
}

IHackMinigame::Result Tier3ICE::update(float dt) {
    m_timeLeft -= dt;
    if (m_timeLeft <= 0.f) return Result::FAILURE;

    // Check win condition — all layers destroyed
    bool allDead = true;
    for (int i = 0; i < ICE_LAYERS; ++i)
        if (m_iceHP[i] > 0) { allDead = false; break; }
    if (allDead) return Result::SUCCESS;

    if (m_phase == Phase::RESOLVING) {
        m_actionTimer -= dt;
        if (m_actionTimer <= 0.f) {
            if (m_action == 0) {
                // BRUTE: damage first living layer
                for (int i = 0; i < ICE_LAYERS; ++i) {
                    if (m_iceHP[i] > 0) {
                        m_iceHP[i] = std::max(0, m_iceHP[i] - 1);
                        m_lastHit = i;
                        break;
                    }
                }
            } else {
                // SCAN: damage weakest layer by 2
                int weakest = -1;
                for (int i = 0; i < ICE_LAYERS; ++i) {
                    if (m_iceHP[i] > 0) {
                        if (weakest == -1 || m_iceHP[i] < m_iceHP[weakest])
                            weakest = i;
                    }
                }
                if (weakest >= 0) {
                    m_iceHP[weakest] = std::max(0, m_iceHP[weakest] - 2);
                    m_lastHit = weakest;
                }
            }
            m_phase = Phase::CHOOSE;
        }
        return Result::RUNNING;
    }

    // CHOOSE phase
    if (m_grace > 0.f) {
        m_grace -= dt;
        m_leftPrev  = sf::Keyboard::isKeyPressed(sf::Keyboard::Left)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        m_rightPrev = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
        m_confPrev  = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return);
        return Result::RUNNING;
    }
    bool leftNow  = sf::Keyboard::isKeyPressed(sf::Keyboard::Left)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool rightNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    bool confNow  = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return);

    if (leftNow && !m_leftPrev)   m_cursor = 0;
    if (rightNow && !m_rightPrev) m_cursor = 1;

    if (confNow && !m_confPrev) {
        m_action      = m_cursor;
        m_phase       = Phase::RESOLVING;
        m_actionTimer = (m_action == 0) ? BRUTE_DUR : SCAN_DUR;
    }

    m_leftPrev  = leftNow;
    m_rightPrev = rightNow;
    m_confPrev  = confNow;

    return Result::RUNNING;
}

void Tier3ICE::render(sf::RenderWindow& window, const sf::Font& font) const {
    float W = static_cast<float>(window.getSize().x);
    float H = static_cast<float>(window.getSize().y);
    float cx = W * 0.5f, cy = H * 0.42f;

    // Panel
    sf::RectangleShape panel({420.f, 260.f});
    panel.setFillColor(UI_BG);
    panel.setOutlineColor(DANGER);
    panel.setOutlineThickness(2.f);
    panel.setPosition(cx - 210.f, cy - 70.f);
    window.draw(panel);

    // Title
    sf::Text title("[ ICE BREAKER — EXECUTIVE SECURITY ]", font, 14);
    title.setFillColor(DANGER);
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(cx - tb.width * 0.5f, cy - 64.f);
    window.draw(title);

    // Timer bar
    float timerFrac = std::max(0.f, m_timeLeft / DURATION);
    sf::RectangleShape timerBg({400.f, 6.f});
    timerBg.setFillColor(DIM);
    timerBg.setPosition(cx - 200.f, cy - 42.f);
    window.draw(timerBg);
    sf::RectangleShape timerFill({400.f * timerFrac, 6.f});
    timerFill.setFillColor(timerFrac < 0.3f ? DANGER : VIOLET);
    timerFill.setPosition(cx - 200.f, cy - 42.f);
    window.draw(timerFill);

    // ICE layer bars
    const char* LAYER_NAMES[ICE_LAYERS] = { "ICE-A", "ICE-B", "ICE-C" };
    for (int i = 0; i < ICE_LAYERS; ++i) {
        float sy = cy - 22.f + i * 36.f;
        float hpFrac = static_cast<float>(m_iceHP[i]) / static_cast<float>(ICE_HP_MAX);

        sf::Text name(LAYER_NAMES[i], font, 13);
        bool isHit = (m_lastHit == i && m_phase == Phase::RESOLVING);
        name.setFillColor(m_iceHP[i] == 0 ? DIM : (isHit ? YELLOW : WHITE));
        name.setPosition(cx - 195.f, sy);
        window.draw(name);

        // HP bar bg
        sf::RectangleShape barBg({280.f, 18.f});
        barBg.setFillColor(sf::Color(0x05, 0x08, 0x10, 0xFF));
        barBg.setOutlineColor(m_iceHP[i] == 0 ? DIM : DANGER);
        barBg.setOutlineThickness(1.f);
        barBg.setPosition(cx - 120.f, sy);
        window.draw(barBg);

        if (m_iceHP[i] > 0) {
            sf::RectangleShape barFill({280.f * hpFrac, 18.f});
            barFill.setFillColor(isHit ? YELLOW : DANGER);
            barFill.setPosition(cx - 120.f, sy);
            window.draw(barFill);
        }

        // HP pips
        char hpBuf[16];
        std::snprintf(hpBuf, sizeof(hpBuf), "%d/%d", m_iceHP[i], ICE_HP_MAX);
        sf::Text hpTxt(hpBuf, font, 11);
        hpTxt.setFillColor(WHITE);
        hpTxt.setPosition(cx + 170.f, sy + 2.f);
        window.draw(hpTxt);
    }

    // Action buttons / resolving state
    float ay = cy + 90.f;
    if (m_phase == Phase::CHOOSE) {
        // BRUTE button
        sf::RectangleShape bBtn({160.f, 40.f});
        bBtn.setFillColor(m_cursor == 0 ? sf::Color(0x28, 0x00, 0x08, 0xFF) : sf::Color(0x05, 0x08, 0x10, 0xFF));
        bBtn.setOutlineColor(m_cursor == 0 ? DANGER : DIM);
        bBtn.setOutlineThickness(m_cursor == 0 ? 2.f : 1.f);
        bBtn.setPosition(cx - 190.f, ay);
        window.draw(bBtn);
        sf::Text bLbl("BRUTE FORCE", font, 13);
        bLbl.setFillColor(m_cursor == 0 ? DANGER : sf::Color(0x60, 0x80, 0xA0, 0xFF));
        sf::FloatRect bl = bLbl.getLocalBounds();
        bLbl.setPosition(cx - 190.f + (160.f - bl.width) * 0.5f, ay + 10.f);
        window.draw(bLbl);

        // SCAN button
        sf::RectangleShape sBtn({160.f, 40.f});
        sBtn.setFillColor(m_cursor == 1 ? sf::Color(0x14, 0x00, 0x28, 0xFF) : sf::Color(0x05, 0x08, 0x10, 0xFF));
        sBtn.setOutlineColor(m_cursor == 1 ? VIOLET : DIM);
        sBtn.setOutlineThickness(m_cursor == 1 ? 2.f : 1.f);
        sBtn.setPosition(cx + 30.f, ay);
        window.draw(sBtn);
        sf::Text sLbl("DEEP SCAN", font, 13);
        sLbl.setFillColor(m_cursor == 1 ? VIOLET : sf::Color(0x60, 0x80, 0xA0, 0xFF));
        sf::FloatRect sl = sLbl.getLocalBounds();
        sLbl.setPosition(cx + 30.f + (160.f - sl.width) * 0.5f, ay + 10.f);
        window.draw(sLbl);

        sf::Text hint("LEFT/RIGHT: SELECT    SPACE: EXECUTE", font, 11);
        hint.setFillColor(sf::Color(0x60, 0x80, 0xA0, 0xFF));
        sf::FloatRect hb = hint.getLocalBounds();
        hint.setPosition(cx - hb.width * 0.5f, ay + 50.f);
        window.draw(hint);
    } else {
        // Resolving — show progress bar
        float progress = 1.f - m_actionTimer / (m_action == 0 ? BRUTE_DUR : SCAN_DUR);
        const char* actionName = (m_action == 0) ? "BRUTE FORCING..." : "SCANNING...";
        sf::Color actionCol    = (m_action == 0) ? DANGER : VIOLET;

        sf::Text act(actionName, font, 16);
        act.setFillColor(actionCol);
        sf::FloatRect ab = act.getLocalBounds();
        act.setPosition(cx - ab.width * 0.5f, ay + 4.f);
        window.draw(act);

        sf::RectangleShape progBg({360.f, 8.f});
        progBg.setFillColor(DIM);
        progBg.setPosition(cx - 180.f, ay + 34.f);
        window.draw(progBg);
        sf::RectangleShape progFill({360.f * progress, 8.f});
        progFill.setFillColor(actionCol);
        progFill.setPosition(cx - 180.f, ay + 34.f);
        window.draw(progFill);
    }
}
