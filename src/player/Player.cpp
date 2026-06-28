#include "Player.h"
#include "PlayerConfig.h"
#include "PlayerStateMachine.h"
#include "WeaponConfig.h"
#include "ecs/Components.h"
#include <algorithm>
#include <cmath>

Player::Player(World& world, PhysicsSystem& physics)
    : m_world(world)
    , m_physics(physics)
{}

Player::~Player() {
    if (m_stateHandle != 0)
        EventBus::unsubscribe<PlayerStateChangedEvent>(m_stateHandle);
}

void Player::spawn(float x, float y) {
    m_entityId = m_world.createEntity();

    Transform t{};
    t.x = x; t.y = y; t.prevX = x; t.prevY = y;
    m_world.addComponent<Transform>(m_entityId, std::move(t));

    m_world.addComponent<Velocity>(m_entityId, Velocity{});
    m_world.addComponent<Health>(m_entityId, Health{100, 100});

    Renderable r;
    r.size  = {PlayerConfig::WIDTH, PlayerConfig::HEIGHT};
    r.color = sf::Color(0x00, 0xFF, 0xEE, 0xFF);
    r.layer = 10;
    m_world.addComponent<Renderable>(m_entityId, std::move(r));

    b2Body* body = m_physics.createDynamicBody(x, y, PlayerConfig::WIDTH, PlayerConfig::HEIGHT);
    Collidable c{}; c.body = body; c.w = PlayerConfig::WIDTH; c.h = PlayerConfig::HEIGHT;
    m_world.addComponent<Collidable>(m_entityId, std::move(c));

    m_world.addComponent<PlayerTag>(m_entityId, PlayerTag{});

    Weapon w; w.ammo = WeaponConfig::MAG_SIZE; w.maxAmmo = WeaponConfig::MAG_SIZE;
    m_world.addComponent<Weapon>(m_entityId, std::move(w));

    m_physics.setWorld(&m_world);

    // Subscribe after entityId is known
    uint32_t myId = m_entityId;
    m_stateHandle = EventBus::on<PlayerStateChangedEvent>(
        [this, myId](const PlayerStateChangedEvent& e) {
            if (e.entityId == myId) handleStateChanged(e.newState);
        });
}

void Player::handleStateChanged(int newState) {
    if (!m_world.hasComponent<Collidable>(m_entityId)) return;
    b2Body* body = m_world.getComponent<Collidable>(m_entityId).body;
    if (!body) return;

    auto st = static_cast<PlayerState>(newState);

    if (st == PlayerState::DASH) {
        m_isDashing = true;
        b2Vec2 vel  = body->GetLinearVelocity();
        float dashSpeedM = PhysicsSystem::toMeters(PlayerConfig::DASH_SPEED);
        body->SetLinearVelocity({m_dashDirX * dashSpeedM, vel.y * 0.1f});
    } else {
        m_isDashing = false;
    }

    if (st == PlayerState::SLIDE) {
        m_isSliding = true;
        b2Vec2 vel  = body->GetLinearVelocity();
        vel.x *= PlayerConfig::SLIDE_SPEED_MULT;
        body->SetLinearVelocity(vel);
    } else {
        m_isSliding = false;
    }
}

void Player::update(float dt, const InputMap& input) {
    if (m_entityId == UINT32_MAX) return;

    auto& col  = m_world.getComponent<Collidable>(m_entityId);
    b2Body* body = col.body;
    if (!body) return;

    b2Vec2 vel = body->GetLinearVelocity();

    // Track facing direction for dash
    if (input.isHeld(Action::MoveLeft))  { m_facingDir = -1.0f; m_dashDirX = -1.0f; m_dashDirY = 0.0f; }
    if (input.isHeld(Action::MoveRight)) { m_facingDir =  1.0f; m_dashDirX =  1.0f; m_dashDirY = 0.0f; }

    if (!m_isDashing && !m_isSliding) {
        float targetVx = 0.0f;
        if (input.isHeld(Action::MoveLeft))  targetVx = -PhysicsSystem::toMeters(PlayerConfig::MOVE_SPEED);
        if (input.isHeld(Action::MoveRight)) targetVx =  PhysicsSystem::toMeters(PlayerConfig::MOVE_SPEED);
        vel.x = targetVx;
    }

    float halfH = PhysicsSystem::toMeters(PlayerConfig::HEIGHT * 0.5f);
    m_grounded = m_physics.isBodyGrounded(body, halfH);

    if (m_grounded) m_coyoteTimer = PlayerConfig::COYOTE_TIME;
    else            m_coyoteTimer = std::max(0.0f, m_coyoteTimer - dt);

    bool jumpHeld = input.isHeld(Action::Jump);
    if (jumpHeld && !m_wasJumpHeld) m_jumpBuffer = PlayerConfig::JUMP_BUFFER;
    else                            m_jumpBuffer = std::max(0.0f, m_jumpBuffer - dt);
    m_wasJumpHeld = jumpHeld;

    if (!m_isDashing && m_jumpBuffer > 0.0f && m_coyoteTimer > 0.0f) {
        vel.y = -PhysicsSystem::toMeters(PlayerConfig::JUMP_IMPULSE);
        m_jumpBuffer = 0.0f; m_coyoteTimer = 0.0f;
    }

    float maxFall = PhysicsSystem::toMeters(PlayerConfig::MAX_FALL_SPEED);
    vel.y = std::min(vel.y, maxFall);

    body->SetLinearVelocity(vel);
}
