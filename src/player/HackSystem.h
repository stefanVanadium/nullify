#pragma once
#include <cstdint>
#include <cstddef>
#include "ecs/World.h"
#include "core/InputMap.h"
#include "hacking/HackMinigame.h"

namespace HackConfig {
    constexpr float INTERACT_RANGE  = 80.0f;
    constexpr float GLITCH_ENTRY    = 0.3f;
}

class HackSystem {
public:
    ~HackSystem() { destroyMinigame(); }

    void update(float dt, uint32_t playerEntityId, World& world, const InputMap& input);

    bool  isHacking()             const { return m_minigame != nullptr; }
    float hackProgress()          const;
    float hackIntensity()         const;
    const IHackMinigame* activeMinigame() const { return m_minigame; }

private:
    // Placement-new buffer — no heap in game loop
    alignas(std::max_align_t) std::byte m_storage[HACK_MINIGAME_STORAGE]{};
    IHackMinigame* m_minigame  = nullptr;
    uint32_t m_hackTarget      = static_cast<uint32_t>(-1);
    float    m_glitchEntryTimer = 0.f;

    void startMinigame(int tier);
    void destroyMinigame();
    void onSuccess(World& world);
    void onFailure();
};
