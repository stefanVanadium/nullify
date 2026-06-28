#pragma once
#include <array>
#include <cstdint>
#include <box2d/box2d.h>
#include "ecs/Systems/PhysicsSystem.h"
#include "core/EventBus.h"
#include "rendering/SpriteBatch.h"

struct RagdollInstance {
    static constexpr int BODIES = 6;
    static constexpr int JOINTS = 5;

    b2Body*  bodies[BODIES]{};
    b2Joint* joints[JOINTS]{};
    float    lifetime = 4.0f;
    float    alpha    = 1.0f;
    bool     active   = false;
};

// Owns a fixed pool of ragdoll instances. Subscribes to EnemyDiedEvent.
// Creates Box2D bodies + revolute joints per death, fades them over 4 seconds.
// Pool size matches MAX_ENEMIES so exhaustion is impossible in practice.
class RagdollSystem {
public:
    static constexpr int MAX_RAGDOLLS = 32;

    explicit RagdollSystem(PhysicsSystem& physics);
    ~RagdollSystem();

    void update(float dt);
    void render(SpriteBatch& batch) const;

private:
    PhysicsSystem& m_physics;

    std::array<RagdollInstance, MAX_RAGDOLLS> m_pool{};
    EventBus::Handle m_diedHandle = 0;

    void onEnemyDied(const EnemyDiedEvent& e);
    void spawnRagdoll(float x, float y, float impulseX, float impulseY);
    void freeInstance(RagdollInstance& r);
};
