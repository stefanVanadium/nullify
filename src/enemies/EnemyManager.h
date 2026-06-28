#pragma once
#include <array>
#include <cstdint>
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "core/EventBus.h"
#include "NavMesh.h"
#include "EnemyConfig.h"

// Manages spawning, updating, and despawning of all enemy entities.
class EnemyManager {
public:
    EnemyManager(World& world, PhysicsSystem& physics);
    ~EnemyManager();

    // Build navmesh from level collision data
    void buildNavMesh(const std::vector<std::vector<int>>& collision,
                      int width, int height, int tileSize);

    // Spawn a SCOUT at world position with preset patrol waypoints
    uint32_t spawnScout(float x, float y, const WaypointPath& waypoints);

    // Called at fixed 60 Hz — updates all active enemies
    void update(float dt);

    int alertLevel() const { return m_alertLevel; }

private:
    World&         m_world;
    PhysicsSystem& m_physics;
    NavMesh        m_navMesh;

    std::array<uint32_t, EnemyConfig::MAX_ENEMIES> m_entities{};
    int   m_count      = 0;
    int   m_alertLevel = 0;

    EventBus::Handle m_bulletHitHandle = 0;
    EventBus::Handle m_enemyDiedHandle = 0;

    void onBulletHit(const BulletHitEvent& e);
    void onEnemyDied(const EnemyDiedEvent& e);
};
