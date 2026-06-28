#pragma once
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "core/InputMap.h"

class Player {
public:
    Player(World& world, PhysicsSystem& physics);

    // Place ZERO at the given pixel position; creates ECS entity + physics body
    void spawn(float x, float y);

    // Called each fixed step — reads input, applies forces to physics body
    void update(float dt, const InputMap& input);

    uint32_t entityId() const { return m_entityId; }

    // Called by PlayerStateMachine to notify ground contact state
    void setGrounded(bool grounded) { m_grounded = grounded; }
    bool isGrounded() const { return m_grounded; }

private:
    World&         m_world;
    PhysicsSystem& m_physics;
    uint32_t       m_entityId = UINT32_MAX;
    bool           m_grounded = false;

    float          m_coyoteTimer  = 0.0f;
    float          m_jumpBuffer   = 0.0f;
};
