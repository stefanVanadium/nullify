#pragma once
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "core/InputMap.h"
#include "core/EventBus.h"

enum class PlayerState : int {
    IDLE   = 0,
    RUN    = 1,
    JUMP   = 2,
    FALL   = 3,
    CROUCH = 4,
    DASH   = 5,
    SLIDE  = 6,
};

class PlayerStateMachine {
public:
    PlayerStateMachine(World& world, InputMap& input, PhysicsSystem& physics);

    void update(float dt);

    PlayerState currentState() const { return m_state; }
    bool        isDashing()    const { return m_state == PlayerState::DASH; }
    bool        isSliding()    const { return m_state == PlayerState::SLIDE; }

private:
    void transition(PlayerState next);
    bool checkGrounded() const;

    World&         m_world;
    InputMap&      m_input;
    PhysicsSystem& m_physics;
    PlayerState    m_state        = PlayerState::IDLE;
    float          m_dashTimer    = 0.0f;
    float          m_dashCooldown = 0.0f;
    float          m_slideTimer   = 0.0f;
};
