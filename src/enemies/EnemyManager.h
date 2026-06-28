#pragma once
#include <array>
#include <cstdint>
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "core/EventBus.h"
#include "NavMesh.h"
#include "EnemyConfig.h"
#include "rendering/SpriteBatch.h"

struct EnemyBullet {
    float x, y;
    float dirX, dirY;
    float speed;
    float travelDist;
    float maxDist;
    bool  active = false;
};

// Manages spawning, updating, and despawning of all enemy entities.
class EnemyManager {
public:
    EnemyManager(World& world, PhysicsSystem& physics);
    ~EnemyManager();

    void buildNavMesh(const std::vector<std::vector<int>>& collision,
                      int width, int height, int tileSize);

    uint32_t spawnScout(float x, float y, const WaypointPath& waypoints);

    void update(float dt);

    void batchDrawBullets(SpriteBatch& batch) const;

    int alertLevel() const { return m_alertLevel; }

private:
    World&         m_world;
    PhysicsSystem& m_physics;
    NavMesh        m_navMesh;

    std::array<uint32_t, EnemyConfig::MAX_ENEMIES> m_entities{};
    int   m_count      = 0;
    int   m_alertLevel = 0;

    std::array<EnemyBullet, EnemyConfig::MAX_ENEMY_BULLETS> m_enemyBullets{};
    size_t m_nextBulletSlot = 0;

    EventBus::Handle m_bulletHitHandle = 0;
    EventBus::Handle m_enemyDiedHandle = 0;
    EventBus::Handle m_enemyFireHandle = 0;

    void onBulletHit(const BulletHitEvent& e);
    void onEnemyDied(const EnemyDiedEvent& e);
    void onEnemyFire(const EnemyFireEvent& e);
    void updateEnemyBullets(float dt);
};
