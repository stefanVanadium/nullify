#include "PhysicsSystem.h"
#include <cassert>

PhysicsSystem::PhysicsSystem()
    // y-down convention: positive y = downward (matches screen space), gravity pulls +y
    : m_b2World(b2Vec2(0.0f, 20.0f))
{
}

PhysicsSystem::~PhysicsSystem() = default;

void PhysicsSystem::update(float fixedDt) {
    m_b2World.Step(fixedDt, VELOCITY_ITERATIONS, POSITION_ITERATIONS);
    if (m_ecsWorld)
        syncToWorld(*m_ecsWorld);
}

void PhysicsSystem::syncToWorld(World& world) {
    auto collidables = world.getComponents<Collidable>();
    auto transforms  = world.getComponents<Transform>();

    for (size_t i = 0; i < MAX_ENTITIES; ++i) {
        if (!world.isAlive(static_cast<uint32_t>(i))) continue;
        if (!world.hasComponent<Collidable>(static_cast<uint32_t>(i))) continue;

        b2Body* body = collidables[i].body;
        if (!body || body->GetType() == b2_staticBody) continue;

        const b2Vec2& pos = body->GetPosition();

        // Save previous position for interpolation before overwriting
        transforms[i].prevX = transforms[i].x;
        transforms[i].prevY = transforms[i].y;

        // Box2D y-down matches screen space directly — no sign flip needed
        float halfW = collidables[i].w * 0.5f;
        float halfH = collidables[i].h * 0.5f;
        transforms[i].x = toPixels(pos.x) - halfW;
        transforms[i].y = toPixels(pos.y) - halfH;
    }
}

b2Body* PhysicsSystem::createStaticBody(float x, float y, float w, float h) {
    b2BodyDef def;
    def.type = b2_staticBody;
    // pixel (x,y) is top-left; Box2D center in meters, same y-down convention
    def.position.Set(toMeters(x + w * 0.5f), toMeters(y + h * 0.5f));

    b2Body* body = m_b2World.CreateBody(&def);

    b2PolygonShape shape;
    shape.SetAsBox(toMeters(w * 0.5f), toMeters(h * 0.5f));

    b2FixtureDef fixture;
    fixture.shape    = &shape;
    fixture.friction = 0.3f;
    body->CreateFixture(&fixture);
    return body;
}

b2Body* PhysicsSystem::createDynamicBody(float x, float y, float w, float h) {
    b2BodyDef def;
    def.type            = b2_dynamicBody;
    def.position.Set(toMeters(x + w * 0.5f), toMeters(y + h * 0.5f));
    def.fixedRotation   = true; // player/enemies don't tumble
    def.linearDamping   = 0.0f;

    b2Body* body = m_b2World.CreateBody(&def);

    b2PolygonShape shape;
    shape.SetAsBox(toMeters(w * 0.5f), toMeters(h * 0.5f));

    b2FixtureDef fixture;
    fixture.shape    = &shape;
    fixture.density  = 1.0f;
    fixture.friction = 0.2f;
    body->CreateFixture(&fixture);
    return body;
}

// Simple RayCast callback that records whether we hit anything
struct GroundCastCallback : public b2RayCastCallback {
    bool hit = false;
    float ReportFixture(b2Fixture*, const b2Vec2&, const b2Vec2&, float) override {
        hit = true;
        return 0.0f; // stop at first hit
    }
};

bool PhysicsSystem::isBodyGrounded(b2Body* body, float halfHeightMeters) const {
    if (!body) return false;
    b2Vec2 center = body->GetPosition();
    b2Vec2 start  = center;
    // Cast downward (positive y in y-down convention), just past the feet
    b2Vec2 end    = { center.x, center.y + halfHeightMeters + 0.05f };

    GroundCastCallback cb;
    m_b2World.RayCast(&cb, start, end);
    return cb.hit;
}

b2Body* PhysicsSystem::createEdgeChain(const b2Vec2* verts, int32 count) {
    b2BodyDef def;
    def.type = b2_staticBody;
    b2Body* body = m_b2World.CreateBody(&def);

    b2ChainShape chain;
    chain.CreateLoop(verts, count);

    b2FixtureDef fixture;
    fixture.shape    = &chain;
    fixture.friction = 0.3f;
    body->CreateFixture(&fixture);
    return body;
}
