#pragma once
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "core/EventBus.h"
#include "NavMesh.h"
#include "EnemyConfig.h"

// Drives PATROL → ALERT → COMBAT → SEARCH state machine for a single enemy entity.
// Called by EnemyManager once per enemy per fixed step.
class AIStateMachine {
public:
    // Update one enemy entity. playerEntity = UINT32_MAX if no player.
    static void update(uint32_t       enemyId,
                       float          dt,
                       uint32_t       playerEntityId,
                       World&         world,
                       PhysicsSystem& physics,
                       const NavMesh& navMesh,
                       int&           aStarBudget); // decrement when A* runs
};
