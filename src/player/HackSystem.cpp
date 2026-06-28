#include "HackSystem.h"
#include "ecs/Components.h"
#include "core/EventBus.h"
#include <cmath>

float HackSystem::hackProgress() const {
    if (!m_minigame) return 0.f;
    // Invert timeLeft to show completion progress
    return 0.f;  // minigame handles its own progress display
}

float HackSystem::hackIntensity() const {
    if (!m_minigame) return 0.f;
    if (m_glitchEntryTimer > 0.f)
        return 1.0f - (1.0f - m_glitchEntryTimer / HackConfig::GLITCH_ENTRY) * 0.8f;
    return 0.2f;
}

void HackSystem::startMinigame(int tier) {
    m_minigame = createHackMinigame(tier, m_storage, sizeof(m_storage));
    m_glitchEntryTimer = HackConfig::GLITCH_ENTRY;
}

void HackSystem::destroyMinigame() {
    if (m_minigame) {
        m_minigame->~IHackMinigame();
        m_minigame = nullptr;
    }
}

void HackSystem::onSuccess(World& world) {
    if (world.isAlive(m_hackTarget) && world.hasComponent<HackableTag>(m_hackTarget)) {
        world.getComponent<HackableTag>(m_hackTarget).hacked = true;
        if (world.hasComponent<Renderable>(m_hackTarget))
            world.getComponent<Renderable>(m_hackTarget).color = sf::Color(0x55, 0x00, 0x88, 0xFF);
        EventBus::emit(HackSuccessEvent{ m_hackTarget });
    }
    destroyMinigame();
    m_hackTarget = static_cast<uint32_t>(-1);
}

void HackSystem::onFailure() {
    destroyMinigame();
    m_hackTarget = static_cast<uint32_t>(-1);
}

void HackSystem::update(float dt, uint32_t playerEntityId, World& world, const InputMap& input) {
    if (m_glitchEntryTimer > 0.f)
        m_glitchEntryTimer = std::max(0.f, m_glitchEntryTimer - dt);

    if (m_minigame) {
        auto result = m_minigame->update(dt);
        if (result == IHackMinigame::Result::SUCCESS)
            onSuccess(world);
        else if (result == IHackMinigame::Result::FAILURE)
            onFailure();
        return;
    }

    if (!input.isPressed(Action::Hack)) return;
    if (!world.isAlive(playerEntityId)) return;
    if (!world.hasComponent<Transform>(playerEntityId)) return;

    const auto& pt = world.getComponent<Transform>(playerEntityId);
    float px = pt.x + 12.f, py = pt.y + 24.f;

    uint32_t nearest   = static_cast<uint32_t>(-1);
    float    nearestD2 = HackConfig::INTERACT_RANGE * HackConfig::INTERACT_RANGE;

    for (uint32_t id = 0; id < MAX_ENTITIES; ++id) {
        if (!world.isAlive(id)) continue;
        if (!world.hasComponent<HackableTag>(id)) continue;
        if (world.getComponent<HackableTag>(id).hacked) continue;
        if (!world.hasComponent<Transform>(id)) continue;
        const auto& t = world.getComponent<Transform>(id);
        float dx = (t.x + 8.f) - px;
        float dy = (t.y + 8.f) - py;
        float d2 = dx*dx + dy*dy;
        if (d2 < nearestD2) { nearestD2 = d2; nearest = id; }
    }

    if (nearest != static_cast<uint32_t>(-1)) {
        m_hackTarget = nearest;
        int tier = 1;
        if (world.hasComponent<HackableTag>(nearest))
            tier = world.getComponent<HackableTag>(nearest).tier;
        startMinigame(tier);
        EventBus::emit(HackActivatedEvent{ nearest });
    }
}
