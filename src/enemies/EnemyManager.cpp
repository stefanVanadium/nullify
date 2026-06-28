#include "EnemyManager.h"
#include "AIStateMachine.h"
#include "ecs/Components.h"
#include "player/WeaponConfig.h"
#include <cmath>
#include <algorithm>

EnemyManager::EnemyManager(World& world, PhysicsSystem& physics)
    : m_world(world)
    , m_physics(physics)
{
    m_entities.fill(static_cast<uint32_t>(MAX_ENTITIES));
    for (auto& b : m_enemyBullets) b.active = false;

    m_bulletHitHandle = EventBus::on<BulletHitEvent>(
        [this](const BulletHitEvent& e) { onBulletHit(e); });
    m_enemyDiedHandle = EventBus::on<EnemyDiedEvent>(
        [this](const EnemyDiedEvent& e) { onEnemyDied(e); });
    m_enemyFireHandle = EventBus::on<EnemyFireEvent>(
        [this](const EnemyFireEvent& e) { onEnemyFire(e); });
    m_empHandle = EventBus::on<EMPDetonatedEvent>(
        [this](const EMPDetonatedEvent& e) { onEMPDetonated(e); });
}

EnemyManager::~EnemyManager() {
    EventBus::unsubscribe<BulletHitEvent>(m_bulletHitHandle);
    EventBus::unsubscribe<EnemyDiedEvent>(m_enemyDiedHandle);
    EventBus::unsubscribe<EnemyFireEvent>(m_enemyFireHandle);
    EventBus::unsubscribe<EMPDetonatedEvent>(m_empHandle);
}

void EnemyManager::buildNavMesh(const std::vector<std::vector<int>>& collision,
                                int width, int height, int tileSize) {
    m_navMesh.build(collision, width, height, tileSize);
}

uint32_t EnemyManager::spawnEnemy(EnemyType type, float x, float y,
                                   const WaypointPath& waypoints) {
    if (m_count >= EnemyConfig::MAX_ENEMIES) return static_cast<uint32_t>(MAX_ENTITIES);

    // Per-type config
    struct TypeCfg { int hp; float w, h; sf::Color col; };
    auto cfg = [&]() -> TypeCfg {
        switch (type) {
            case EnemyType::ENFORCER:
                return { EnemyConfig::ENFORCER_HP, EnemyConfig::ENFORCER_WIDTH, EnemyConfig::ENFORCER_HEIGHT,
                         sf::Color(0x1A, 0x28, 0x40, 0xFF) };
            case EnemyType::SHIELD:
                return { EnemyConfig::SHIELD_HP, EnemyConfig::SHIELD_WIDTH, EnemyConfig::SHIELD_HEIGHT,
                         sf::Color(0xFF, 0xE6, 0x00, 0xFF) };
            case EnemyType::SNIPER:
                return { EnemyConfig::SNIPER_HP, EnemyConfig::SNIPER_WIDTH, EnemyConfig::SNIPER_HEIGHT,
                         sf::Color(0x60, 0x80, 0xA0, 0xFF) };
            case EnemyType::HACKER:
                return { EnemyConfig::HACKER_HP, EnemyConfig::HACKER_WIDTH, EnemyConfig::HACKER_HEIGHT,
                         sf::Color(0xAA, 0x00, 0xFF, 0xFF) };
            case EnemyType::HEAVY:
                return { EnemyConfig::HEAVY_HP, EnemyConfig::HEAVY_WIDTH, EnemyConfig::HEAVY_HEIGHT,
                         sf::Color(0xFF, 0x00, 0x38, 0xFF) };
            case EnemyType::DRONE:
                return { EnemyConfig::DRONE_HP, EnemyConfig::DRONE_WIDTH, EnemyConfig::DRONE_HEIGHT,
                         sf::Color(0x00, 0xFF, 0xEE, 0xCC) };
            case EnemyType::CYBORG_ELITE:
                return { EnemyConfig::ELITE_HP, EnemyConfig::ELITE_WIDTH, EnemyConfig::ELITE_HEIGHT,
                         sf::Color(0xAA, 0x00, 0xFF, 0xFF) };
            default: // SCOUT
                return { EnemyConfig::SCOUT_HP, EnemyConfig::SCOUT_WIDTH, EnemyConfig::SCOUT_HEIGHT,
                         sf::Color(0xFF, 0x00, 0x6B, 0xFF) };
        }
    }();

    uint32_t id = m_world.createEntity();

    Transform t{}; t.x = x; t.y = y; t.prevX = x; t.prevY = y;
    m_world.addComponent<Transform>(id, std::move(t));
    m_world.addComponent<Velocity>(id, Velocity{});
    m_world.addComponent<Health>(id, Health{ cfg.hp, cfg.hp });

    Renderable r;
    r.size  = { cfg.w, cfg.h };
    r.color = cfg.col;
    r.layer = 9;
    m_world.addComponent<Renderable>(id, std::move(r));

    b2Body* body = m_physics.createDynamicBody(x, y, cfg.w, cfg.h);
    // Drone: sensor body (no tile collision)
    if (type == EnemyType::DRONE && body) {
        for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
            f->SetSensor(true);
        body->SetGravityScale(0.f);
    }
    Collidable c{}; c.body = body; c.w = cfg.w; c.h = cfg.h;
    m_world.addComponent<Collidable>(id, std::move(c));

    m_world.addComponent<EnemyTag>(id, EnemyTag{ type });
    m_world.addComponent<AIState>(id, AIState{});
    m_world.addComponent<WaypointPath>(id, WaypointPath(waypoints));

    // Add perception components for stealth system
    ConeOfVision cov{};
    m_world.addComponent<ConeOfVision>(id, std::move(cov));
    m_world.addComponent<HearingRadius>(id, HearingRadius{});
    m_world.addComponent<StealthBody>(id, StealthBody{});
    m_world.addComponent<SilentTakedown>(id, SilentTakedown{});

    m_entities[static_cast<size_t>(m_count++)] = id;
    return id;
}

uint32_t EnemyManager::spawnScout(float x, float y, const WaypointPath& waypoints) {
    return spawnEnemy(EnemyType::SCOUT, x, y, waypoints);
}

void EnemyManager::update(float dt) {
    uint32_t playerEntityId = m_world.findPlayer();
    int aStarBudget = 4;

    for (int i = 0; i < m_count; ) {
        uint32_t eid = m_entities[static_cast<size_t>(i)];
        if (!m_world.isAlive(eid)) {
            m_entities[static_cast<size_t>(i)] = m_entities[static_cast<size_t>(--m_count)];
            continue;
        }
        if (m_world.hasComponent<Health>(eid)) {
            const auto& hp = m_world.getComponent<Health>(eid);
            if (hp.current <= 0) {
                const auto& tf = m_world.getComponent<Transform>(eid);
                EventBus::emit(EnemyDiedEvent{ eid, tf.x, tf.y });
                // Destroy physics body before entity — otherwise body becomes a phantom wall
                if (m_world.hasComponent<Collidable>(eid)) {
                    b2Body* body = m_world.getComponent<Collidable>(eid).body;
                    m_physics.destroyBody(body);
                }
                m_world.destroyEntity(eid);
                m_entities[static_cast<size_t>(i)] = m_entities[static_cast<size_t>(--m_count)];
                continue;
            }
        }
        AIStateMachine::update(eid, dt, playerEntityId, m_world, m_physics, m_navMesh, aStarBudget);
        ++i;
    }

    updateEnemyBullets(dt);

    // Global alert level
    int alertCount = 0;
    for (int i = 0; i < m_count; ++i) {
        uint32_t eid = m_entities[static_cast<size_t>(i)];
        if (!m_world.hasComponent<AIState>(eid)) continue;
        const auto& ai = m_world.getComponent<AIState>(eid);
        if (ai.current == AIStateEnum::COMBAT || ai.current == AIStateEnum::ALERT)
            ++alertCount;
    }
    if      (alertCount == 0) m_alertLevel = 0;
    else if (alertCount == 1) m_alertLevel = 1;
    else if (alertCount == 2) m_alertLevel = 2;
    else                      m_alertLevel = 3;
}

void EnemyManager::updateEnemyBullets(float dt) {
    uint32_t playerId = m_world.findPlayer();
    float px = 0.f, py = 0.f;
    if (playerId != static_cast<uint32_t>(MAX_ENTITIES) && m_world.hasComponent<Transform>(playerId)) {
        const auto& pt = m_world.getComponent<Transform>(playerId);
        px = pt.x + 12.f; py = pt.y + 24.f;
    }

    for (auto& b : m_enemyBullets) {
        if (!b.active) continue;

        float step = b.speed * dt;
        float nx = b.x + b.dirX * step;
        float ny = b.y + b.dirY * step;

        b2Vec2 from = { PhysicsSystem::toMeters(b.x), PhysicsSystem::toMeters(b.y) };
        b2Vec2 to   = { PhysicsSystem::toMeters(nx),  PhysicsSystem::toMeters(ny)  };

        if (!m_physics.rayCastClear(from, to)) {
            // Check if we hit the player
            if (playerId != static_cast<uint32_t>(MAX_ENTITIES)) {
                float dx = nx - px, dy = ny - py;
                if (dx*dx + dy*dy < 36.f * 36.f) {  // ~player half-width
                    if (m_world.hasComponent<Health>(playerId)) {
                        auto& hp = m_world.getComponent<Health>(playerId);
                        hp.current = std::max(0, hp.current - EnemyConfig::SCOUT_ATTACK_DAMAGE);
                        EventBus::emit(PlayerDamagedEvent{ playerId, EnemyConfig::SCOUT_ATTACK_DAMAGE });
                    }
                }
            }
            b.active = false;
            continue;
        }

        b.x = nx;
        b.y = ny;
        b.travelDist += step;
        if (b.travelDist >= b.maxDist) b.active = false;
    }
}

void EnemyManager::batchDrawBullets(SpriteBatch& batch) const {
    static const sf::Color BULLET_COLOR = sf::Color(0xFF, 0x00, 0x38, 0xFF);
    for (const auto& b : m_enemyBullets) {
        if (!b.active) continue;
        batch.draw(
            {b.x - EnemyConfig::SCOUT_BULLET_W * 0.5f, b.y - EnemyConfig::SCOUT_BULLET_H * 0.5f},
            {EnemyConfig::SCOUT_BULLET_W, EnemyConfig::SCOUT_BULLET_H},
            BULLET_COLOR
        );
    }
}

void EnemyManager::onBulletHit(const BulletHitEvent& e) {
    for (int i = 0; i < m_count; ++i) {
        uint32_t eid = m_entities[static_cast<size_t>(i)];
        if (!m_world.isAlive(eid) || !m_world.hasComponent<Transform>(eid)) continue;
        const auto& tf = m_world.getComponent<Transform>(eid);
        float dx = tf.x - e.x, dy = tf.y - e.y;
        float d2 = dx*dx + dy*dy;
        float hr = EnemyConfig::ALERT_HEAR_RANGE;
        if (d2 <= hr * hr) {
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

void EnemyManager::onEnemyDied(const EnemyDiedEvent&) {}

void EnemyManager::onEMPDetonated(const EMPDetonatedEvent& e) {
    for (int i = 0; i < m_count; ++i) {
        uint32_t eid = m_entities[static_cast<size_t>(i)];
        if (!m_world.isAlive(eid) || !m_world.hasComponent<Transform>(eid)) continue;
        const auto& tf = m_world.getComponent<Transform>(eid);
        float dx = tf.x - e.x, dy = tf.y - e.y;
        if (dx*dx + dy*dy > e.radius * e.radius) continue;

        // Only affects electronic types
        EnemyType et = m_world.hasComponent<EnemyTag>(eid)
                     ? m_world.getComponent<EnemyTag>(eid).type
                     : EnemyType::SCOUT;
        if (et == EnemyType::DRONE || et == EnemyType::CYBORG_ELITE) {
            if (m_world.hasComponent<AIState>(eid)) {
                auto& ai = m_world.getComponent<AIState>(eid);
                ai.empDisabled = true;
                ai.empTimer    = WeaponConfig::EMP_DISABLE_DUR;
            }
        }
    }
}

void EnemyManager::onEnemyFire(const EnemyFireEvent& e) {
    EnemyBullet& b = m_enemyBullets[m_nextBulletSlot];
    m_nextBulletSlot = (m_nextBulletSlot + 1) % EnemyConfig::MAX_ENEMY_BULLETS;
    b.x          = e.fromX;
    b.y          = e.fromY;
    b.dirX       = e.dirX;
    b.dirY       = e.dirY;
    b.speed      = EnemyConfig::SCOUT_BULLET_SPEED;
    b.travelDist = 0.f;
    b.maxDist    = EnemyConfig::SCOUT_BULLET_MAX_DIST;
    b.active     = true;
}
