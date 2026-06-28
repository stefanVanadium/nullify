#include "WeaponSystem.h"
#include "WeaponConfig.h"
#include "ecs/Components.h"
#include "core/EventBus.h"
#include "enemies/EnemyConfig.h"
#include <cmath>
#include <algorithm>

static float randSpread(float maxRad) {
    // Fast deterministic spread — not truly random but visually fine
    static uint32_t s = 1234567891u;
    s ^= s << 13; s ^= s >> 17; s ^= s << 5;
    float t = static_cast<float>(s & 0xFFFF) / 65535.f;  // 0..1
    return (t - 0.5f) * 2.0f * maxRad;
}

WeaponSystem::WeaponSystem(World& world, PhysicsSystem& physics)
    : m_world(world)
    , m_physics(physics)
{
    for (auto& b : m_bullets) b.active = false;

    // Initialize slots
    for (int i = 0; i < WEAPON_SLOTS; ++i) {
        m_slots[i].type     = static_cast<WeaponType>(i);
        m_slots[i].unlocked = false;
        m_slots[i].ammo     = 0;
    }
    // Player starts with PHANTOM-9 only
    m_slots[0].unlocked = true;
    m_slots[0].ammo     = WeaponConfig::P9_MAG_SIZE;
}

void WeaponSystem::unlockWeapon(WeaponType wt) {
    int idx = static_cast<int>(wt);
    m_slots[idx].unlocked = true;
    switch (wt) {
        case WeaponType::STATIC_SMG:   m_slots[idx].ammo = WeaponConfig::SMG_MAG_SIZE;   break;
        case WeaponType::RAILGUN:      m_slots[idx].ammo = WeaponConfig::RAIL_MAG_SIZE;  break;
        case WeaponType::VOID_SHOTGUN: m_slots[idx].ammo = WeaponConfig::VOID_MAG_SIZE;  break;
        case WeaponType::EMP_GRENADE:  m_slots[idx].ammo = WeaponConfig::EMP_MAG_SIZE;   break;
        case WeaponType::NEURAL_SPIKE: m_slots[idx].ammo = WeaponConfig::SPIKE_MAG_SIZE; break;
        default: break;
    }
}

void WeaponSystem::switchWeapon(int delta) {
    int next = m_activeSlot;
    for (int i = 0; i < WEAPON_SLOTS; ++i) {
        next = (next + delta + WEAPON_SLOTS) % WEAPON_SLOTS;
        if (m_slots[next].unlocked) { m_activeSlot = next; break; }
    }
    EventBus::emit(WeaponSwitchedEvent{ m_activeSlot });
}

void WeaponSystem::fire(sf::Vector2f from, sf::Vector2f target, uint32_t playerEntity) {
    (void)playerEntity;

    WeaponSlot& slot = m_slots[m_activeSlot];
    if (!slot.unlocked)   return;
    if (slot.cooldown > 0) return;
    if (slot.ammo <= 0)   return;

    float dx = target.x - from.x;
    float dy = target.y - from.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 0.001f) return;
    dx /= len; dy /= len;

    WeaponType wt = slot.type;
    float soundRadius = 0.f;

    switch (wt) {
    case WeaponType::PHANTOM9:
        fireBullet(from.x, from.y, dx, dy,
                   WeaponConfig::P9_BULLET_SPEED, WeaponConfig::P9_MAX_DIST);
        slot.cooldown = WeaponConfig::P9_FIRE_COOLDOWN;
        soundRadius   = WeaponConfig::P9_SOUND_RADIUS;
        break;

    case WeaponType::STATIC_SMG: {
        float spread = randSpread(WeaponConfig::SMG_SPREAD_RAD);
        float sdx = dx * std::cos(spread) - dy * std::sin(spread);
        float sdy = dx * std::sin(spread) + dy * std::cos(spread);
        fireBullet(from.x, from.y, sdx, sdy,
                   WeaponConfig::SMG_BULLET_SPEED, WeaponConfig::SMG_MAX_DIST);
        slot.cooldown = WeaponConfig::SMG_FIRE_COOLDOWN;
        soundRadius   = WeaponConfig::SMG_SOUND_RADIUS;
        break;
    }

    case WeaponType::RAILGUN:
        fireBullet(from.x, from.y, dx, dy,
                   WeaponConfig::RAIL_BULLET_SPEED, WeaponConfig::RAIL_MAX_DIST,
                   BulletType::STANDARD, 0.f, true);
        slot.cooldown = WeaponConfig::RAIL_FIRE_COOLDOWN;
        soundRadius   = WeaponConfig::RAIL_SOUND_RADIUS;
        break;

    case WeaponType::VOID_SHOTGUN:
        for (int p = 0; p < WeaponConfig::VOID_PELLETS; ++p) {
            float spread = randSpread(WeaponConfig::VOID_SPREAD_RAD);
            float pdx = dx * std::cos(spread) - dy * std::sin(spread);
            float pdy = dx * std::sin(spread) + dy * std::cos(spread);
            fireBullet(from.x, from.y, pdx, pdy,
                       WeaponConfig::VOID_BULLET_SPEED, WeaponConfig::VOID_MAX_DIST);
        }
        slot.cooldown = WeaponConfig::VOID_FIRE_COOLDOWN;
        soundRadius   = WeaponConfig::VOID_SOUND_RADIUS;
        break;

    case WeaponType::EMP_GRENADE:
        // Arc upward slightly then gravity pulls down
        fireBullet(from.x, from.y, dx, dy * 0.3f - 0.6f,
                   WeaponConfig::EMP_THROW_SPEED, 9999.f,
                   BulletType::EMP, -WeaponConfig::EMP_GRAVITY);
        slot.cooldown = WeaponConfig::EMP_FIRE_COOLDOWN;
        soundRadius   = WeaponConfig::EMP_SOUND_RADIUS;
        break;

    case WeaponType::NEURAL_SPIKE:
        fireBullet(from.x, from.y, dx, dy,
                   WeaponConfig::SPIKE_BULLET_SPEED, WeaponConfig::SPIKE_MAX_DIST,
                   BulletType::NEURAL_SPIKE);
        slot.cooldown = WeaponConfig::SPIKE_FIRE_COOLDOWN;
        soundRadius   = WeaponConfig::SPIKE_SOUND_RADIUS;
        break;

    default: break;
    }

    // ammo intentionally infinite — finite ammo to be re-enabled when pickups are polished
    // --slot.ammo;

    if (soundRadius > 0.f)
        EventBus::emit(SoundEmittedEvent{ from.x, from.y, soundRadius, true });

    // Update legacy Weapon component facingDir (used by rendering)
    uint32_t pid = m_world.findPlayer();
    if (pid != static_cast<uint32_t>(MAX_ENTITIES) && m_world.hasComponent<Weapon>(pid))
        m_world.getComponent<Weapon>(pid).facingDir = (dx >= 0.f) ? 1.f : -1.f;
}

void WeaponSystem::fireBullet(float x, float y, float dx, float dy,
                               float speed, float maxDist,
                               BulletType type, float vyGravity, bool penetrates) {
    BulletState& b = m_bullets[m_nextSlot];
    m_nextSlot = (m_nextSlot + 1) % MAX_BULLETS;

    b.x          = x;
    b.y          = y;
    b.dirX       = dx;
    b.dirY       = dy;
    b.speed      = speed;
    b.travelDist = 0.f;
    b.maxDist    = maxDist;
    b.vyGravity  = vyGravity;
    b.type       = type;
    b.penetrates = penetrates;
    b.active     = true;
}

int WeaponSystem::tryHitEnemy(float nx, float ny, WeaponType wt) {
    for (uint32_t eid = 0; eid < MAX_ENTITIES; ++eid) {
        if (!m_world.isAlive(eid) || !m_world.hasComponent<EnemyTag>(eid)) continue;
        if (!m_world.hasComponent<Transform>(eid)) continue;
        const auto& tf = m_world.getComponent<Transform>(eid);
        float cx = tf.x + EnemyConfig::SCOUT_WIDTH * 0.5f;
        float cy = tf.y + EnemyConfig::SCOUT_HEIGHT * 0.5f;
        float ex = cx - nx, ey = cy - ny;
        if (ex*ex + ey*ey > EnemyConfig::SCOUT_WIDTH * EnemyConfig::SCOUT_WIDTH) continue;

        int dmg = 0;
        switch (wt) {
            case WeaponType::PHANTOM9:     dmg = WeaponConfig::P9_DAMAGE;    break;
            case WeaponType::STATIC_SMG:   dmg = WeaponConfig::SMG_DAMAGE;   break;
            case WeaponType::RAILGUN:      dmg = WeaponConfig::RAIL_DAMAGE;  break;
            case WeaponType::VOID_SHOTGUN: dmg = WeaponConfig::VOID_DAMAGE;  break;
            case WeaponType::EMP_GRENADE:  dmg = 0;                           break;
            case WeaponType::NEURAL_SPIKE: dmg = WeaponConfig::SPIKE_DAMAGE; break;
            default: break;
        }
        if (m_world.hasComponent<Health>(eid)) {
            auto& hp = m_world.getComponent<Health>(eid);
            hp.current = std::max(0, hp.current - dmg);
        }
        // VOID SHOTGUN knockback
        if (wt == WeaponType::VOID_SHOTGUN && m_world.hasComponent<Collidable>(eid)) {
            b2Body* body = m_world.getComponent<Collidable>(eid).body;
            if (body) {
                float kx = cx - nx, ky = cy - ny;
                float klen = std::sqrt(kx*kx + ky*ky);
                if (klen > 0.01f) { kx /= klen; ky /= klen; }
                float kb = PhysicsSystem::toMeters(WeaponConfig::VOID_KNOCKBACK);
                body->SetLinearVelocity({ body->GetLinearVelocity().x + kx * kb,
                                          body->GetLinearVelocity().y + ky * kb });
            }
        }
        return dmg;
    }
    return 0;
}

void WeaponSystem::update(float dt) {
    // Cooldown tick for active slot
    WeaponSlot& active = m_slots[m_activeSlot];
    if (active.cooldown > 0.f) active.cooldown = std::max(0.f, active.cooldown - dt);

    WeaponType wt = active.type;

    for (auto& b : m_bullets) {
        if (!b.active) continue;

        float step = b.speed * dt;

        // Apply gravity for EMP arc
        if (b.type == BulletType::EMP) {
            b.dirY     += WeaponConfig::EMP_GRAVITY * dt / b.speed;
            // Normalize so speed stays constant on horizontal, not on vector
        }

        float nx = b.x + b.dirX * step;
        float ny = b.y + b.dirY * step;

        b2Vec2 from2 = { PhysicsSystem::toMeters(b.x), PhysicsSystem::toMeters(b.y) };
        b2Vec2 to2   = { PhysicsSystem::toMeters(nx),  PhysicsSystem::toMeters(ny)  };

        bool blocked = !m_physics.rayCastClear(from2, to2);

        if (blocked && !b.penetrates) {
            // EMP detonates on impact
            if (b.type == BulletType::EMP) {
                EventBus::emit(EMPDetonatedEvent{ nx, ny, WeaponConfig::EMP_RADIUS });
                EventBus::emit(BulletHitEvent{ nx, ny, false, static_cast<uint32_t>(MAX_ENTITIES) });
                b.active = false;
                continue;
            }
            bool hitEnemy = (tryHitEnemy(nx, ny, wt) > 0);
            // Neural Spike: emit hack event on hit
            if (b.type == BulletType::NEURAL_SPIKE) {
                for (uint32_t eid = 0; eid < MAX_ENTITIES; ++eid) {
                    if (!m_world.isAlive(eid) || !m_world.hasComponent<EnemyTag>(eid)) continue;
                    if (!m_world.hasComponent<Transform>(eid)) continue;
                    const auto& tf = m_world.getComponent<Transform>(eid);
                    float cx = tf.x + EnemyConfig::SCOUT_WIDTH * 0.5f;
                    float cy = tf.y + EnemyConfig::SCOUT_HEIGHT * 0.5f;
                    float ex = cx - nx, ey = cy - ny;
                    if (ex*ex + ey*ey < EnemyConfig::SCOUT_WIDTH * EnemyConfig::SCOUT_WIDTH) {
                        EventBus::emit(EnemyHackedEvent{ eid, WeaponConfig::SPIKE_CONTROL_DUR });
                        break;
                    }
                }
            }
            uint32_t hitId = static_cast<uint32_t>(MAX_ENTITIES);
            EventBus::emit(BulletHitEvent{ nx, ny, hitEnemy, hitId });
            b.active = false;
            continue;
        }

        if (blocked && b.penetrates) {
            // Railgun: hit enemy but keep going
            tryHitEnemy(nx, ny, wt);
        }

        b.x = nx;
        b.y = ny;
        b.travelDist += step;

        if (b.travelDist >= b.maxDist) {
            if (b.type == BulletType::EMP)
                EventBus::emit(EMPDetonatedEvent{ b.x, b.y, WeaponConfig::EMP_RADIUS });
            b.active = false;
        }
    }
}

sf::Color WeaponSystem::bulletColor(BulletType t, WeaponType wt) const {
    if (t == BulletType::EMP)         return sf::Color(0xFF, 0xE6, 0x00, 0xFF); // yellow
    if (t == BulletType::NEURAL_SPIKE) return sf::Color(0xAA, 0x00, 0xFF, 0xFF); // violet
    switch (wt) {
        case WeaponType::RAILGUN:      return sf::Color(0x00, 0xFF, 0xEE, 0xFF); // bright cyan
        case WeaponType::VOID_SHOTGUN: return sf::Color(0xAA, 0x00, 0xFF, 0xCC); // violet pellet
        default:                        return sf::Color(0x00, 0xFF, 0xEE, 0xFF); // cyan
    }
}

void WeaponSystem::batchDraw(SpriteBatch& batch) const {
    WeaponType wt = m_slots[m_activeSlot].type;
    for (const auto& b : m_bullets) {
        if (!b.active) continue;
        float bw, bh;
        switch (b.type) {
            case BulletType::EMP:         bw = WeaponConfig::EMP_BULLET_W;   bh = WeaponConfig::EMP_BULLET_H;   break;
            case BulletType::NEURAL_SPIKE: bw = WeaponConfig::SPIKE_BULLET_W; bh = WeaponConfig::SPIKE_BULLET_H; break;
            default:
                switch (wt) {
                    case WeaponType::RAILGUN:      bw = WeaponConfig::RAIL_BULLET_W;  bh = WeaponConfig::RAIL_BULLET_H;  break;
                    case WeaponType::VOID_SHOTGUN: bw = WeaponConfig::VOID_BULLET_W;  bh = WeaponConfig::VOID_BULLET_H;  break;
                    case WeaponType::STATIC_SMG:   bw = WeaponConfig::SMG_BULLET_W;   bh = WeaponConfig::SMG_BULLET_H;   break;
                    default:                        bw = WeaponConfig::P9_BULLET_W;    bh = WeaponConfig::P9_BULLET_H;    break;
                }
        }
        batch.draw({ b.x - bw * 0.5f, b.y - bh * 0.5f }, { bw, bh }, bulletColor(b.type, wt));
    }
}

int WeaponSystem::activeBullets() const {
    int count = 0;
    for (const auto& b : m_bullets)
        if (b.active) ++count;
    return count;
}
