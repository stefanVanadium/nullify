#pragma once
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "core/EventBus.h"
#include "NavMesh.h"
#include "EnemyConfig.h"

// Drives the state machine for a single enemy entity.
// Dispatches per-EnemyType behavior in COMBAT state.
// Called by EnemyManager once per enemy per fixed step.
class AIStateMachine {
public:
    static void update(uint32_t       enemyId,
                       float          dt,
                       uint32_t       playerEntityId,
                       World&         world,
                       PhysicsSystem& physics,
                       const NavMesh& navMesh,
                       int&           aStarBudget);

private:
    // Per-type helpers called from COMBAT case
    static void combatScout(uint32_t eid, float dt, float ex, float ey,
                             float px, float py, float playerDist2,
                             b2Body* body, AIState& ai,
                             World& world, PhysicsSystem& physics,
                             const NavMesh& nav, int& budget);

    static void combatEnforcer(uint32_t eid, float dt, float ex, float ey,
                                float px, float py, float playerDist2,
                                b2Body* body, AIState& ai,
                                World& world, PhysicsSystem& physics,
                                const NavMesh& nav, int& budget);

    static void combatSniper(uint32_t eid, float dt, float ex, float ey,
                              float px, float py,
                              b2Body* body, AIState& ai);

    static void combatHeavy(uint32_t eid, float dt, float ex, float ey,
                             float px, float py,
                             b2Body* body, AIState& ai);

    static void combatDrone(uint32_t eid, float dt, float ex, float ey,
                             float px, float py, float playerDist2,
                             b2Body* body, AIState& ai);
};
