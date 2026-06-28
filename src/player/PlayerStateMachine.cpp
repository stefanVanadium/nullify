#include "PlayerStateMachine.h"
#include "PlayerConfig.h"
#include "ecs/Components.h"

PlayerStateMachine::PlayerStateMachine(World& world, InputMap& input, PhysicsSystem& physics)
    : m_world(world)
    , m_input(input)
    , m_physics(physics)
{}

bool PlayerStateMachine::checkGrounded() const {
    uint32_t pid = m_world.findPlayer();
    if (pid == static_cast<uint32_t>(MAX_ENTITIES)) return false;
    if (!m_world.hasComponent<Collidable>(pid)) return false;
    b2Body* body = m_world.getComponent<Collidable>(pid).body;
    float halfH  = PhysicsSystem::toMeters(PlayerConfig::HEIGHT * 0.5f);
    return m_physics.isBodyGrounded(body, halfH);
}

void PlayerStateMachine::transition(PlayerState next) {
    if (m_state == next) return;
    uint32_t pid = m_world.findPlayer();
    EventBus::emit(PlayerStateChangedEvent{
        .entityId = pid,
        .oldState = static_cast<int>(m_state),
        .newState = static_cast<int>(next)
    });
    m_state = next;
}

void PlayerStateMachine::update(float dt) {
    bool grounded = checkGrounded();

    uint32_t pid = m_world.findPlayer();
    if (pid == static_cast<uint32_t>(MAX_ENTITIES)) return;

    bool movingH  = m_input.isHeld(Action::MoveLeft) || m_input.isHeld(Action::MoveRight);
    bool crouching = m_input.isHeld(Action::Crouch);
    bool dashPressed = m_input.isPressed(Action::Dash);

    b2Vec2 vel{0, 0};
    if (m_world.hasComponent<Collidable>(pid)) {
        if (b2Body* body = m_world.getComponent<Collidable>(pid).body)
            vel = body->GetLinearVelocity();
    }
    bool movingUp   = vel.y >  0.5f;
    bool movingDown = vel.y < -0.5f;

    // Cooldown timers always tick
    if (m_dashCooldown > 0.0f) m_dashCooldown -= dt;
    if (m_dashTimer    > 0.0f) m_dashTimer    -= dt;
    if (m_slideTimer   > 0.0f) m_slideTimer   -= dt;

    switch (m_state) {
        case PlayerState::IDLE:
            if (!grounded)                                  transition(movingUp ? PlayerState::JUMP : PlayerState::FALL);
            else if (dashPressed && m_dashCooldown <= 0.0f) { m_dashTimer = PlayerConfig::DASH_DURATION; m_dashCooldown = PlayerConfig::DASH_COOLDOWN; transition(PlayerState::DASH); }
            else if (crouching)                              transition(PlayerState::CROUCH);
            else if (movingH)                                transition(PlayerState::RUN);
            break;

        case PlayerState::RUN:
            if (!grounded)                                  transition(movingUp ? PlayerState::JUMP : PlayerState::FALL);
            else if (dashPressed && m_dashCooldown <= 0.0f) { m_dashTimer = PlayerConfig::DASH_DURATION; m_dashCooldown = PlayerConfig::DASH_COOLDOWN; transition(PlayerState::DASH); }
            else if (crouching)                             { m_slideTimer = PlayerConfig::SLIDE_DURATION; transition(PlayerState::SLIDE); }
            else if (!movingH)                               transition(PlayerState::IDLE);
            break;

        case PlayerState::JUMP:
            if (grounded)    transition(PlayerState::IDLE);
            else if (movingDown) transition(PlayerState::FALL);
            break;

        case PlayerState::FALL:
            if (grounded)    transition(movingH ? PlayerState::RUN : PlayerState::IDLE);
            else if (movingUp) transition(PlayerState::JUMP);
            break;

        case PlayerState::CROUCH:
            if (!grounded)   transition(PlayerState::FALL);
            else if (!crouching) transition(movingH ? PlayerState::RUN : PlayerState::IDLE);
            break;

        case PlayerState::DASH:
            if (m_dashTimer <= 0.0f) transition(grounded ? (movingH ? PlayerState::RUN : PlayerState::IDLE) : PlayerState::FALL);
            break;

        case PlayerState::SLIDE:
            if (m_slideTimer <= 0.0f) transition(!crouching ? (movingH ? PlayerState::RUN : PlayerState::IDLE) : PlayerState::CROUCH);
            break;
    }
}
