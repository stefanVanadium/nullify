#pragma once
#include "HackMinigame.h"

// Tier 3: ICE Breaker — 12 second timer, 3 ICE layers.
// LEFT = BRUTE FORCE (hits current layer, 1.5s action, 1 dmg)
// RIGHT = SCAN (reveals weakest, 2.5s action, 2 dmg to weakest)
// SPACE/Enter confirms chosen action.
class Tier3ICE final : public IHackMinigame {
public:
    static constexpr float DURATION     = 12.0f;
    static constexpr int   ICE_LAYERS   = 3;
    static constexpr int   ICE_HP_MAX   = 3;
    static constexpr float BRUTE_DUR    = 1.4f;
    static constexpr float SCAN_DUR     = 2.4f;

    void init(int randomSeed);

    Result update(float dt) override;
    void   render(sf::RenderWindow& window, const sf::Font& font) const override;

private:
    enum class Phase { CHOOSE, RESOLVING };

    int   m_iceHP[ICE_LAYERS]{};  // remaining HP per layer
    Phase m_phase    = Phase::CHOOSE;
    float m_actionTimer = 0.f;
    int   m_action   = 0;   // 0=BRUTE, 1=SCAN
    int   m_cursor   = 0;   // 0=BRUTE, 1=SCAN
    int   m_lastHit  = -1;
    float m_grace    = 0.25f;
    bool  m_leftPrev  = false;
    bool  m_rightPrev = false;
    bool  m_confPrev  = false;
};
