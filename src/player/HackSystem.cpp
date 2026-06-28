#include "HackSystem.h"
#include "ecs/Components.h"
#include "core/EventBus.h"
#include <cmath>

float HackSystem::hackProgress() const {
    if (!m_hacking) return 0.f;
    return 1.0f - (m_hackTimer / HackConfig::TIER1_DURATION);
}

float HackSystem::hackIntensity() const {
    if (!m_hacking) return 0.f;
    float elapsed = HackConfig::TIER1_DURATION - m_hackTimer;
    // Full glitch on entry, then settle to 0.2
    if (elapsed < HackConfig::GLITCH_ENTRY)
        return 1.0f - (elapsed / HackConfig::GLITCH_ENTRY) * 0.8f;
    return 0.2f;
}

void HackSystem::update(float dt, uint32_t playerEntityId, World& world, const InputMap& input) {
    if (m_hacking) {
        m_hackTimer -= dt;
        if (m_hackTimer <= 0.f) {
            m_hacking = false;
            if (world.isAlive(m_hackTarget) && world.hasComponent<HackableTag>(m_hackTarget)) {
                world.getComponent<HackableTag>(m_hackTarget).hacked = true;
                // Tint hacked terminal dark
                if (world.hasComponent<Renderable>(m_hackTarget)) {
                    world.getComponent<Renderable>(m_hackTarget).color =
                        sf::Color(0x55, 0x00, 0x88, 0xFF);
                }
                EventBus::emit(HackSuccessEvent{ m_hackTarget });
            }
            m_hackTarget = static_cast<uint32_t>(-1);
        }
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
        m_hacking    = true;
        m_hackTarget = nearest;
        m_hackTimer  = HackConfig::TIER1_DURATION;
        EventBus::emit(HackActivatedEvent{ nearest });
    }
}
