#include "WeaponSystem.h"
#include "WeaponConfig.h"
#include "ecs/Components.h"
#include "core/EventBus.h"
#include "enemies/EnemyConfig.h"
#include <cmath>
#include <algorithm>

WeaponSystem::WeaponSystem(World& world, PhysicsSystem& physics)
    : m_world(world)
    , m_physics(physics)
{
    for (auto& b : m_bullets) b.active = false;
}

void WeaponSystem::fire(sf::Vector2f from, sf::Vector2f target, uint32_t playerEntity) {
    if (!m_world.isAlive(playerEntity)) return;
    if (!m_world.hasComponent<Weapon>(playerEntity)) return;

    auto& w = m_world.getComponent<Weapon>(playerEntity);
    if (w.cooldown > 0.0f) return;

    float dx = target.x - from.x;
    float dy = target.y - from.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f) return;

    dx /= len;
    dy /= len;

    BulletState& b = m_bullets[m_nextSlot];
    m_nextSlot = (m_nextSlot + 1) % MAX_BULLETS;

    b.x          = from.x;
    b.y          = from.y;
    b.dirX       = dx;
    b.dirY       = dy;
    b.speed      = WeaponConfig::BULLET_SPEED;
    b.travelDist = 0.0f;
    b.maxDist    = WeaponConfig::BULLET_MAX_DIST;
    b.active     = true;

    w.cooldown  = WeaponConfig::FIRE_COOLDOWN;
    w.facingDir = (dx >= 0.0f) ? 1.0f : -1.0f;
}

void WeaponSystem::update(float dt) {
    // Decrement player weapon cooldown
    uint32_t pid = m_world.findPlayer();
    if (pid != static_cast<uint32_t>(MAX_ENTITIES) && m_world.hasComponent<Weapon>(pid)) {
        auto& w = m_world.getComponent<Weapon>(pid);
        if (w.cooldown > 0.0f) w.cooldown -= dt;
        if (w.cooldown < 0.0f) w.cooldown = 0.0f;
    }

    for (auto& b : m_bullets) {
        if (!b.active) continue;

        float step = b.speed * dt;
        float nx   = b.x + b.dirX * step;
        float ny   = b.y + b.dirY * step;

        // LOS raycast — if path is blocked, bullet stops
        b2Vec2 from = { PhysicsSystem::toMeters(b.x),  PhysicsSystem::toMeters(b.y)  };
        b2Vec2 to   = { PhysicsSystem::toMeters(nx), PhysicsSystem::toMeters(ny) };

        if (!m_physics.rayCastClear(from, to)) {
            // Check if we hit an enemy — endpoint (nx,ny) is inside a fixture
            bool     hitEnemy = false;
            uint32_t hitId    = static_cast<uint32_t>(MAX_ENTITIES);
            for (uint32_t eid = 0; eid < MAX_ENTITIES; ++eid) {
                if (!m_world.isAlive(eid) || !m_world.hasComponent<EnemyTag>(eid)) continue;
                if (!m_world.hasComponent<Transform>(eid))  continue;
                const auto& tf = m_world.getComponent<Transform>(eid);
                float cx = tf.x + EnemyConfig::SCOUT_WIDTH  * 0.5f;
                float cy = tf.y + EnemyConfig::SCOUT_HEIGHT * 0.5f;
                float ex2 = cx - nx, ey2 = cy - ny;
                if (ex2*ex2 + ey2*ey2 < EnemyConfig::SCOUT_WIDTH * EnemyConfig::SCOUT_WIDTH) {
                    if (m_world.hasComponent<Health>(eid)) {
                        auto& hp = m_world.getComponent<Health>(eid);
                        hp.current = std::max(0, hp.current - WeaponConfig::BULLET_DAMAGE);
                    }
                    hitEnemy = true;
                    hitId    = eid;
                    break;
                }
            }
            EventBus::emit(BulletHitEvent{ nx, ny, hitEnemy, hitId });
            b.active = false;
            continue;
        }

        b.x = nx;
        b.y = ny;
        b.travelDist += step;

        if (b.travelDist >= b.maxDist) {
            b.active = false;
        }
    }
}

void WeaponSystem::batchDraw(SpriteBatch& batch) const {
    static const sf::Color BULLET_COLOR = sf::Color(0x00, 0xFF, 0xEE, 0xFF);
    for (const auto& b : m_bullets) {
        if (!b.active) continue;
        batch.draw(
            {b.x - WeaponConfig::BULLET_W * 0.5f, b.y - WeaponConfig::BULLET_H * 0.5f},
            {WeaponConfig::BULLET_W, WeaponConfig::BULLET_H},
            BULLET_COLOR
        );
    }
}

int WeaponSystem::activeBullets() const {
    int count = 0;
    for (const auto& b : m_bullets)
        if (b.active) ++count;
    return count;
}
