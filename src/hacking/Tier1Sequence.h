#pragma once
#include "HackMinigame.h"

// Tier 1: 4-symbol sequence match — 3 second timer.
// LEFT/RIGHT cycles symbol, SPACE/Enter confirms each slot.
class Tier1Sequence final : public IHackMinigame {
public:
    static constexpr float DURATION    = 3.0f;
    static constexpr float INPUT_GRACE = 0.25f;  // ignore input on first 250ms
    static constexpr int   SEQ_LEN     = 4;
    static constexpr int   SYMBOL_CNT  = 4;

    void init(int randomSeed);

    Result update(float dt) override;
    void   render(sf::RenderWindow& window, const sf::Font& font) const override;

private:
    int  m_target[SEQ_LEN]{};
    int  m_input[SEQ_LEN]{};
    int  m_step    = 0;
    int  m_cursor  = 0;
    float m_grace  = INPUT_GRACE;
    bool m_leftPrev    = false;
    bool m_rightPrev   = false;
    bool m_confirmPrev = false;
};
