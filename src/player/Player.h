#pragma once
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "core/InputMap.h"
#include "core/EventBus.h"

class Player {
public:
    Player(World& world, PhysicsSystem& physics);
    ~Player();

    void spawn(float x, float y);
    void update(float dt, const InputMap& input);

    uint32_t entityId()  const { return m_entityId; }
    bool     isGrounded() const { return m_grounded; }

private:
    void handleStateChanged(int newState);

    World&         m_world;
    PhysicsSystem& m_physics;
    uint32_t       m_entityId    = UINT32_MAX;
    bool           m_grounded    = false;

    float          m_coyoteTimer  = 0.0f;
    float          m_jumpBuffer   = 0.0f;
    bool           m_wasJumpHeld  = false;

    bool           m_isDashing    = false;
    float          m_dashDirX     = 1.0f;
    float          m_dashDirY     = 0.0f;
    bool           m_isSliding    = false;
    float          m_facingDir    = 1.0f;  // 1=right, -1=left

    EventBus::Handle m_stateHandle = 0;
};
