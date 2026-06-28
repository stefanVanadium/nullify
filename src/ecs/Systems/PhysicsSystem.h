#pragma once
#include <box2d/box2d.h>
#include "ecs/World.h"

// Sole owner of b2World. No other class accesses it.
class PhysicsSystem {
public:
    PhysicsSystem();
    ~PhysicsSystem();

    // Called at fixed 60 Hz
    void update(float fixedDt);

    // Sync b2Body positions → Transform components
    void syncToWorld(World& world);

    // Body factory helpers — called by LevelLoader and Player, never from hot path
    b2Body* createStaticBody(float x, float y, float w, float h);
    b2Body* createDynamicBody(float x, float y, float w, float h);

    // Edge-chain body for tilemap rows (more efficient than per-tile boxes)
    b2Body* createEdgeChain(const b2Vec2* verts, int32 count);

    bool isBodyGrounded(b2Body* body, float halfHeightMeters) const;

    // Returns true if the line between from and to hits no fixtures
    // ignore: if non-null, that body's fixtures are skipped (use for LOS target body)
    bool rayCastClear(b2Vec2 from, b2Vec2 to, b2Body* ignore = nullptr) const;

    // Safely removes a body from b2World — call before destroyEntity on physics entities
    void destroyBody(b2Body* body);

    void setWorld(World* world) { m_ecsWorld = world; }

private:
    b2World  m_b2World;
    World*   m_ecsWorld = nullptr;

    static constexpr int32 VELOCITY_ITERATIONS = 8;
    static constexpr int32 POSITION_ITERATIONS = 3;

    // Box2D uses meters; game uses pixels @ 32px per meter
    static constexpr float PPM = 32.0f;

public:
    static float toMeters(float px) { return px / PPM; }
    static float toPixels(float m)  { return m * PPM;  }
};
