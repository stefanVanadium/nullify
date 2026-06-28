#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "rendering/SpriteBatch.h"

struct BulletState {
    float x, y;
    float dirX, dirY;
    float speed;
    float travelDist;
    float maxDist;
    bool  active;
};

class WeaponSystem {
public:
    static constexpr size_t MAX_BULLETS = 1024;

    WeaponSystem(World& world, PhysicsSystem& physics);

    // Fire a bullet from worldPos toward targetPos
    void fire(sf::Vector2f from, sf::Vector2f target, uint32_t playerEntity);

    // Advance bullets, emit BulletHitEvent on deactivation
    void update(float dt);

    // Batch all active bullet quads into an existing SpriteBatch
    void batchDraw(SpriteBatch& batch) const;

    int activeBullets() const;

private:
    World&         m_world;
    PhysicsSystem& m_physics;

    std::array<BulletState, MAX_BULLETS> m_bullets{};
    size_t m_nextSlot = 0; // circular overwrite on exhaustion
};
