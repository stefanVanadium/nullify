#include "Player.h"
#include "PlayerConfig.h"
#include "ecs/Components.h"
#include <algorithm>
#include <cmath>

Player::Player(World& world, PhysicsSystem& physics)
    : m_world(world)
    , m_physics(physics)
{}

void Player::spawn(float x, float y) {
    m_entityId = m_world.createEntity();

    Transform t{};
    t.x = x; t.y = y;
    t.prevX = x; t.prevY = y;
    m_world.addComponent<Transform>(m_entityId, std::move(t));

    m_world.addComponent<Velocity>(m_entityId, Velocity{});

    m_world.addComponent<Health>(m_entityId, Health{100, 100});

    Renderable r;
    r.shape.setSize({PlayerConfig::WIDTH, PlayerConfig::HEIGHT});
    r.shape.setFillColor(sf::Color(0x00, 0xFF, 0xEE)); // Neon cyan
    r.layer = 10;
    r.visible = true;
    m_world.addComponent<Renderable>(m_entityId, std::move(r));

    b2Body* body = m_physics.createDynamicBody(x, y,
        PlayerConfig::WIDTH, PlayerConfig::HEIGHT);

    Collidable c{};
    c.body = body;
    c.w    = PlayerConfig::WIDTH;
    c.h    = PlayerConfig::HEIGHT;
    m_world.addComponent<Collidable>(m_entityId, std::move(c));

    m_world.addComponent<PlayerTag>(m_entityId, PlayerTag{});

    m_physics.setWorld(&m_world);
}

void Player::update(float dt, const InputMap& input) {
    if (m_entityId == UINT32_MAX) return;

    auto& col = m_world.getComponent<Collidable>(m_entityId);
    b2Body* body = col.body;
    if (!body) return;

    b2Vec2 vel = body->GetLinearVelocity();

    // Horizontal movement
    float targetVx = 0.0f;
    if (input.isHeld(Action::MoveLeft))  targetVx = -PhysicsSystem::toMeters(PlayerConfig::MOVE_SPEED);
    if (input.isHeld(Action::MoveRight)) targetVx =  PhysicsSystem::toMeters(PlayerConfig::MOVE_SPEED);
    vel.x = targetVx;

    // Coyote time countdown
    if (m_grounded) {
        m_coyoteTimer = PlayerConfig::COYOTE_TIME;
    } else {
        m_coyoteTimer = std::max(0.0f, m_coyoteTimer - dt);
    }

    // Jump buffer countdown
    if (input.isPressed(Action::Jump)) {
        m_jumpBuffer = PlayerConfig::JUMP_BUFFER;
    } else {
        m_jumpBuffer = std::max(0.0f, m_jumpBuffer - dt);
    }

    // Apply jump if buffer + coyote allow
    if (m_jumpBuffer > 0.0f && m_coyoteTimer > 0.0f) {
        vel.y = PhysicsSystem::toMeters(PlayerConfig::JUMP_IMPULSE);
        m_jumpBuffer  = 0.0f;
        m_coyoteTimer = 0.0f;
    }

    // Clamp fall speed
    float maxFall = -PhysicsSystem::toMeters(PlayerConfig::MAX_FALL_SPEED);
    vel.y = std::max(vel.y, maxFall);

    body->SetLinearVelocity(vel);
}
