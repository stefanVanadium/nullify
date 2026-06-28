#include "Tier1Sequence.h"
#include <SFML/Window/Keyboard.hpp>
#include <cstring>
#include <cmath>

// ASCII symbols — guaranteed to render in any monospace font
static const char SYMBOL_CHARS[4] = { 'A', 'B', 'C', 'D' };

static const sf::Color CYAN   = sf::Color(0x00, 0xFF, 0xEE, 0xFF);
static const sf::Color DIM    = sf::Color(0x1A, 0x28, 0x40, 0xFF);
static const sf::Color VIOLET = sf::Color(0xAA, 0x00, 0xFF, 0xFF);
static const sf::Color DANGER = sf::Color(0xFF, 0x00, 0x38, 0xFF);
static const sf::Color UI_BG  = sf::Color(0x0A, 0x10, 0x20, 0xEE);
static const sf::Color WHITE  = sf::Color(0xE0, 0xEE, 0xF8, 0xFF);
static const sf::Color GREEN  = sf::Color(0x00, 0xFF, 0xEE, 0xAA);

static int pseudoRand(int seed, int i) {
    unsigned v = static_cast<unsigned>((seed * 2654435769u) ^ (static_cast<unsigned>(i) * 2246822519u));
    return static_cast<int>(v & 3u);
}

void Tier1Sequence::init(int randomSeed) {
    m_timeLeft    = DURATION;
    m_step        = 0;
    m_cursor      = 0;
    m_grace       = INPUT_GRACE;
    m_leftPrev    = true;   // start as "held" so we don't false-trigger on carry-over keys
    m_rightPrev   = true;
    m_confirmPrev = true;
    std::memset(m_input, 0, sizeof(m_input));

    for (int i = 0; i < SEQ_LEN; ++i)
        m_target[i] = pseudoRand(randomSeed + i * 7, i);
}

IHackMinigame::Result Tier1Sequence::update(float dt) {
    m_timeLeft -= dt;
    if (m_timeLeft <= 0.f) return Result::FAILURE;
    if (m_step >= SEQ_LEN) return Result::SUCCESS;

    // Grace period — let player release all keys before accepting input
    if (m_grace > 0.f) {
        m_grace -= dt;
        // While in grace, update prev states to current key state (so we don't fire on release)
        m_leftPrev    = sf::Keyboard::isKeyPressed(sf::Keyboard::Left)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
        m_rightPrev   = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
        m_confirmPrev = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return);
        return Result::RUNNING;
    }

    bool leftNow  = sf::Keyboard::isKeyPressed(sf::Keyboard::Left)  || sf::Keyboard::isKeyPressed(sf::Keyboard::A);
    bool rightNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    bool confNow  = sf::Keyboard::isKeyPressed(sf::Keyboard::Space) || sf::Keyboard::isKeyPressed(sf::Keyboard::Return);

    if (leftNow && !m_leftPrev)
        m_cursor = (m_cursor + SYMBOL_CNT - 1) % SYMBOL_CNT;
    if (rightNow && !m_rightPrev)
        m_cursor = (m_cursor + 1) % SYMBOL_CNT;

    if (confNow && !m_confirmPrev) {
        m_input[m_step] = m_cursor;
        if (m_input[m_step] != m_target[m_step])
            return Result::FAILURE;
        ++m_step;
        m_cursor = 0;
        if (m_step >= SEQ_LEN) return Result::SUCCESS;
    }

    m_leftPrev    = leftNow;
    m_rightPrev   = rightNow;
    m_confirmPrev = confNow;

    return Result::RUNNING;
}

void Tier1Sequence::render(sf::RenderWindow& window, const sf::Font& font) const {
    float W = static_cast<float>(window.getSize().x);
    float H = static_cast<float>(window.getSize().y);
    float cx = W * 0.5f, cy = H * 0.42f;

    sf::RectangleShape panel({440.f, 190.f});
    panel.setFillColor(UI_BG);
    panel.setOutlineColor(VIOLET);
    panel.setOutlineThickness(2.f);
    panel.setPosition(cx - 220.f, cy - 56.f);
    window.draw(panel);

    sf::Text title("[ DECRYPTION SEQUENCE ]", font, 16);
    title.setFillColor(VIOLET);
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(cx - tb.width * 0.5f - tb.left, cy - 50.f);
    window.draw(title);

    // Timer bar
    float timerFrac = std::max(0.f, m_timeLeft / DURATION);
    sf::RectangleShape timerBg({400.f, 6.f});
    timerBg.setFillColor(DIM);
    timerBg.setPosition(cx - 200.f, cy - 26.f);
    window.draw(timerBg);
    sf::RectangleShape timerFill({400.f * timerFrac, 6.f});
    timerFill.setFillColor(timerFrac < 0.3f ? DANGER : VIOLET);
    timerFill.setPosition(cx - 200.f, cy - 26.f);
    window.draw(timerFill);

    // Target row
    sf::Text lblT("TARGET:", font, 12);
    lblT.setFillColor(sf::Color(0x60, 0x80, 0xA0, 0xFF));
    lblT.setPosition(cx - 200.f, cy - 12.f);
    window.draw(lblT);

    constexpr float SLOT_W = 60.f, GAP = 12.f;
    float startX = cx - (SEQ_LEN * SLOT_W + (SEQ_LEN - 1) * GAP) * 0.5f;

    for (int i = 0; i < SEQ_LEN; ++i) {
        float sx = startX + i * (SLOT_W + GAP);
        sf::RectangleShape slot({SLOT_W, 42.f});
        slot.setFillColor(sf::Color(0x05, 0x08, 0x10, 0xFF));
        slot.setOutlineColor(sf::Color(0x1A, 0x28, 0x40, 0xFF));
        slot.setOutlineThickness(1.f);
        slot.setPosition(sx, cy + 2.f);
        window.draw(slot);

        char buf[4] = { SYMBOL_CHARS[m_target[i]], 0 };
        sf::Text sym(buf, font, 24);
        sym.setFillColor(WHITE);
        sf::FloatRect sb = sym.getLocalBounds();
        sym.setPosition(sx + (SLOT_W - sb.width) * 0.5f - sb.left,
                        cy + 2.f + (42.f - sb.height) * 0.5f - sb.top);
        window.draw(sym);
    }

    // Input row
    sf::Text lblI("INPUT:", font, 12);
    lblI.setFillColor(sf::Color(0x60, 0x80, 0xA0, 0xFF));
    lblI.setPosition(cx - 200.f, cy + 52.f);
    window.draw(lblI);

    for (int i = 0; i < SEQ_LEN; ++i) {
        float sx = startX + i * (SLOT_W + GAP);
        float sy = cy + 68.f;
        bool isActive = (i == m_step);

        sf::RectangleShape slot({SLOT_W, 42.f});
        slot.setFillColor(isActive ? sf::Color(0x00, 0x18, 0x18, 0xFF) : sf::Color(0x05, 0x08, 0x10, 0xFF));
        slot.setOutlineColor(isActive ? CYAN : DIM);
        slot.setOutlineThickness(isActive ? 2.f : 1.f);
        slot.setPosition(sx, sy);
        window.draw(slot);

        char dispChar = 0;
        sf::Color symCol = WHITE;
        if (i < m_step) {
            dispChar = SYMBOL_CHARS[m_input[i]];
            symCol = GREEN;
        } else if (isActive && m_grace <= 0.f) {
            dispChar = SYMBOL_CHARS[m_cursor];
            symCol = CYAN;
        }

        if (dispChar) {
            char buf[4] = { dispChar, 0 };
            sf::Text sym(buf, font, 24);
            sym.setFillColor(symCol);
            sf::FloatRect sb = sym.getLocalBounds();
            sym.setPosition(sx + (SLOT_W - sb.width) * 0.5f - sb.left,
                            sy + (42.f - sb.height) * 0.5f - sb.top);
            window.draw(sym);
        }
    }

    // Input legend
    sf::Text hint("A/D or LEFT/RIGHT: cycle    SPACE: confirm", font, 11);
    hint.setFillColor(sf::Color(0x60, 0x80, 0xA0, 0xFF));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setPosition(cx - hb.width * 0.5f - hb.left, cy + 118.f);
    window.draw(hint);

    // Grace period overlay
    if (m_grace > 0.f) {
        sf::Text ready("...", font, 14);
        ready.setFillColor(sf::Color(0x60, 0x80, 0xA0, 0xFF));
        sf::FloatRect rb = ready.getLocalBounds();
        ready.setPosition(cx - rb.width * 0.5f - rb.left, cy + 48.f);
        window.draw(ready);
    }
}
