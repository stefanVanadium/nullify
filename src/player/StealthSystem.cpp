#include "StealthSystem.h"
#include "ecs/Components.h"
#include <cmath>
#include <algorithm>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

static constexpr int   LOS_BUDGET_PER_FRAME = 8;
static constexpr float TAKEDOWN_RANGE       = 48.0f;
static constexpr float TAKEDOWN_DOT         = 0.7f;  // cos(~45°) — must be behind enemy
static constexpr float CORPSE_DETECT_RANGE  = 200.0f;

StealthSystem::StealthSystem(World& world, PhysicsSystem& physics)
    : m_world(world)
    , m_physics(physics)
{
    m_soundHandle = EventBus::on<SoundEmittedEvent>(
        [this](const SoundEmittedEvent& e) { onSoundEmitted(e); });
}

StealthSystem::~StealthSystem() {
    EventBus::unsubscribe<SoundEmittedEvent>(m_soundHandle);
}

void StealthSystem::onSoundEmitted(const SoundEmittedEvent& e) {
    if (e.radius <= 0.f) return;

    for (uint32_t eid = 0; eid < MAX_ENTITIES; ++eid) {
        if (!m_world.isAlive(eid)) continue;
        if (!m_world.hasComponent<HearingRadius>(eid)) continue;
        if (!m_world.hasComponent<Transform>(eid)) continue;

        const auto& tf  = m_world.getComponent<Transform>(eid);
        auto&       hr  = m_world.getComponent<HearingRadius>(eid);

        float dx = tf.x - e.x, dy = tf.y - e.y;
        float alertR = std::min(e.radius, hr.range);
        if (dx*dx + dy*dy > alertR * alertR) continue;

        hr.alertedBySound = true;

        if (m_world.hasComponent<AIState>(eid)) {
            auto& ai = m_world.getComponent<AIState>(eid);
            if (ai.current == AIStateEnum::PATROL) {
                ai.current    = AIStateEnum::ALERT;
                ai.alertX     = e.x;
                ai.alertY     = e.y;
                ai.stateTimer = 0.f;
            }
        }
    }
}

void StealthSystem::checkTakedown(uint32_t playerId, const InputMap& input) {
    if (!input.isPressed(Action::Takedown)) return;
    if (!m_world.hasComponent<Transform>(playerId)) return;

    const auto& pt = m_world.getComponent<Transform>(playerId);
    float px = pt.x + 12.f;
    float py = pt.y + 24.f;

    // Player facing direction from Weapon component
    float facingDir = 1.f;
    if (m_world.hasComponent<Weapon>(playerId))
        facingDir = m_world.getComponent<Weapon>(playerId).facingDir;

    for (uint32_t eid = 0; eid < MAX_ENTITIES; ++eid) {
        if (!m_world.isAlive(eid)) continue;
        if (!m_world.hasComponent<EnemyTag>(eid)) continue;
        if (!m_world.hasComponent<Transform>(eid)) continue;
        if (!m_world.hasComponent<SilentTakedown>(eid)) continue;

        const auto& et = m_world.getComponent<Transform>(eid);
        float ex = et.x + 12.f;
        float ey = et.y + 24.f;

        float dx = ex - px, dy = ey - py;
        float d2 = dx*dx + dy*dy;
        if (d2 > TAKEDOWN_RANGE * TAKEDOWN_RANGE) continue;

        // Enemy must be facing away (dot of player-facing and enemy-facing > threshold)
        float enemyFacing = 1.f;
        if (m_world.hasComponent<AIState>(eid)) {
            const auto& ai = m_world.getComponent<AIState>(eid);
            enemyFacing = (ai.lastSeenX < ex) ? -1.f : 1.f;
        }

        float dotVal = facingDir * enemyFacing;
        if (dotVal < TAKEDOWN_DOT) continue;

        // Instant kill — no sound
        EventBus::emit(TakedownEvent{ eid });
        if (m_world.hasComponent<Health>(eid)) {
            auto& hp = m_world.getComponent<Health>(eid);
            hp.current = 0;
        }
        break;
    }
}

void StealthSystem::checkCorpseDetection() {
    for (uint32_t eid = 0; eid < MAX_ENTITIES; ++eid) {
        if (!m_world.isAlive(eid)) continue;
        if (!m_world.hasComponent<StealthBody>(eid)) continue;
        auto& sb = m_world.getComponent<StealthBody>(eid);
        if (!sb.isCorpse || sb.corpseFound) continue;
        if (!m_world.hasComponent<Transform>(eid)) continue;

        const auto& ct = m_world.getComponent<Transform>(eid);

        // Check if any alert enemy has LOS to this corpse
        for (uint32_t obs = 0; obs < MAX_ENTITIES; ++obs) {
            if (!m_world.isAlive(obs) || obs == eid) continue;
            if (!m_world.hasComponent<EnemyTag>(obs)) continue;
            if (!m_world.hasComponent<AIState>(obs)) continue;
            const auto& ai = m_world.getComponent<AIState>(obs);
            if (ai.current == AIStateEnum::PATROL) continue;  // only alert/combat enemies notice
            if (!m_world.hasComponent<Transform>(obs)) continue;

            const auto& ot = m_world.getComponent<Transform>(obs);
            float dx = ct.x - ot.x, dy = ct.y - ot.y;
            if (dx*dx + dy*dy > CORPSE_DETECT_RANGE * CORPSE_DETECT_RANGE) continue;

            b2Vec2 from = { PhysicsSystem::toMeters(ot.x + 12.f), PhysicsSystem::toMeters(ot.y + 24.f) };
            b2Vec2 to   = { PhysicsSystem::toMeters(ct.x + 12.f), PhysicsSystem::toMeters(ct.y + 24.f) };
            if (!m_physics.rayCastClear(from, to)) continue;

            sb.corpseFound = true;
            EventBus::emit(CorpseFoundEvent{ eid, ct.x, ct.y });
            break;
        }
    }
}

void StealthSystem::update(float dt, uint32_t playerEntityId, const InputMap& input) {
    (void)dt;

    // ── 1. Cone-of-vision LOS checks (round-robin, max LOS_BUDGET_PER_FRAME) ──
    if (playerEntityId != static_cast<uint32_t>(MAX_ENTITIES)) {
        int checked = 0;
        for (uint32_t eid = 0; eid < MAX_ENTITIES && checked < LOS_BUDGET_PER_FRAME; ++eid) {
            // Offset starting point by round-robin to distribute evenly
            uint32_t candidate = static_cast<uint32_t>(
                (static_cast<size_t>(eid) + static_cast<size_t>(m_losRoundRobin)) % MAX_ENTITIES);

            if (!m_world.isAlive(candidate)) continue;
            if (!m_world.hasComponent<ConeOfVision>(candidate)) continue;
            if (!m_world.hasComponent<Transform>(candidate)) continue;

            ++checked;

            auto& cov      = m_world.getComponent<ConeOfVision>(candidate);
            const auto& et = m_world.getComponent<Transform>(candidate);
            const auto& pt = m_world.hasComponent<Transform>(playerEntityId)
                           ? m_world.getComponent<Transform>(playerEntityId)
                           : Transform{};

            float ex = et.x + 12.f, ey = et.y + 24.f;
            float px = pt.x + 12.f, py = pt.y + 24.f;
            float dx = px - ex,     dy = py - ey;
            float dist = std::sqrt(dx*dx + dy*dy);

            cov.playerVisible = false;

            if (dist < cov.range) {
                // Facing direction from AIState
                float facing = 1.f;
                if (m_world.hasComponent<AIState>(candidate))
                    facing = (m_world.getComponent<AIState>(candidate).lastSeenX < ex) ? -1.f : 1.f;

                float dotVal = (dx / std::max(dist, 0.001f)) * facing;
                if (dotVal > std::cos(cov.halfAngle)) {
                    b2Vec2 from2 = { PhysicsSystem::toMeters(ex), PhysicsSystem::toMeters(ey) };
                    b2Vec2 to2   = { PhysicsSystem::toMeters(px), PhysicsSystem::toMeters(py) };
                    b2Body* pb = m_world.hasComponent<Collidable>(playerEntityId)
                               ? m_world.getComponent<Collidable>(playerEntityId).body : nullptr;
                    if (m_physics.rayCastClear(from2, to2, pb))
                        cov.playerVisible = true;
                }
            }
        }
        m_losRoundRobin = (m_losRoundRobin + LOS_BUDGET_PER_FRAME) % MAX_ENTITIES;
    }

    // ── 2. Takedown check ──
    if (playerEntityId != static_cast<uint32_t>(MAX_ENTITIES))
        checkTakedown(playerEntityId, input);

    // ── 3. Corpse detection ──
    checkCorpseDetection();
}

void StealthSystem::renderCones(sf::RenderTarget& rt) const {
    constexpr int SEGMENTS = 10;  // arc subdivisions — more = smoother cone

    for (uint32_t eid = 0; eid < MAX_ENTITIES; ++eid) {
        if (!m_world.isAlive(eid)) continue;
        if (!m_world.hasComponent<ConeOfVision>(eid)) continue;
        if (!m_world.hasComponent<Transform>(eid)) continue;

        const auto& cov = m_world.getComponent<ConeOfVision>(eid);
        const auto& tf  = m_world.getComponent<Transform>(eid);

        float ex = tf.x + 12.f, ey = tf.y + 24.f;

        // Facing direction from velocity (most reliable real-time indicator)
        float facing = 1.f;
        if (m_world.hasComponent<Velocity>(eid)) {
            const auto& vel = m_world.getComponent<Velocity>(eid);
            if (vel.vx < -0.5f)      facing = -1.f;
            else if (vel.vx > 0.5f)  facing = 1.f;
        }
        // Fallback: derive from AIState target
        if (m_world.hasComponent<AIState>(eid)) {
            const auto& ai = m_world.getComponent<AIState>(eid);
            if (ai.targetX < tf.x - 4.f)      facing = -1.f;
            else if (ai.targetX > tf.x + 4.f) facing = 1.f;
        }

        float baseAngle = (facing >= 0.f) ? 0.f : 3.14159265f;

        // Semi-transparent fill: cyan idle, red when player spotted
        sf::Color fillCol = cov.playerVisible
            ? sf::Color(0xFF, 0x00, 0x38, 0x50)
            : sf::Color(0x00, 0xFF, 0xEE, 0x28);

        // Triangle fan: center + SEGMENTS+1 arc vertices
        sf::VertexArray va(sf::TriangleFan, static_cast<std::size_t>(SEGMENTS + 2));
        va[0] = { sf::Vector2f(ex, ey), fillCol };

        for (int i = 0; i <= SEGMENTS; ++i) {
            float t = static_cast<float>(i) / static_cast<float>(SEGMENTS);
            float a = baseAngle - cov.halfAngle + t * 2.f * cov.halfAngle;
            float px = ex + cov.range * std::cos(a);
            float py = ey + cov.range * std::sin(a);
            va[static_cast<std::size_t>(i + 1)] = { sf::Vector2f(px, py), fillCol };
        }

        rt.draw(va);

        // Thin outline edges (direction lines only — left and right edge of cone)
        sf::Color edgeCol = cov.playerVisible
            ? sf::Color(0xFF, 0x00, 0x38, 0xA0)
            : sf::Color(0x00, 0xFF, 0xEE, 0x70);

        sf::Vertex edge[4];
        float aLeft  = baseAngle - cov.halfAngle;
        float aRight = baseAngle + cov.halfAngle;
        edge[0] = { sf::Vector2f(ex, ey), edgeCol };
        edge[1] = { sf::Vector2f(ex + cov.range * std::cos(aLeft),  ey + cov.range * std::sin(aLeft)),  edgeCol };
        edge[2] = { sf::Vector2f(ex, ey), edgeCol };
        edge[3] = { sf::Vector2f(ex + cov.range * std::cos(aRight), ey + cov.range * std::sin(aRight)), edgeCol };
        rt.draw(edge, 4, sf::Lines);
    }
}
