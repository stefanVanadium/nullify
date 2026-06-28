#pragma once
#include <cstdint>
#include "ecs/World.h"
#include "core/InputMap.h"

namespace HackConfig {
    constexpr float INTERACT_RANGE  = 80.0f;
    constexpr float TIER1_DURATION  = 3.0f;
    constexpr float GLITCH_ENTRY    = 0.3f;  // seconds of full glitch on entry
}

class HackSystem {
public:
    void update(float dt, uint32_t playerEntityId, World& world, const InputMap& input);

    bool  isHacking()     const { return m_hacking; }
    float hackProgress()  const;
    float hackIntensity() const;

private:
    bool     m_hacking    = false;
    uint32_t m_hackTarget = static_cast<uint32_t>(-1);
    float    m_hackTimer  = 0.f;
};
