#include "RagdollSystem.h"
#include <cmath>
#include <SFML/Graphics.hpp>

// Ragdoll body layout (all sizes in pixels, converted to meters internally):
//   [0] torso   28×20
//   [1] head    14×14
//   [2] arm L   18×8
//   [3] arm R   18×8
//   [4] leg L   10×20
//   [5] leg R   10×20

namespace {
    struct PartDef { float ox, oy, w, h; };   // offset from enemy center, size
    constexpr PartDef PARTS[RagdollInstance::BODIES] = {
        {  0.0f, -4.0f, 28.0f, 20.0f },  // torso
        {  0.0f,-22.0f, 14.0f, 14.0f },  // head
        {-20.0f, -4.0f, 18.0f,  8.0f },  // arm L
        { 20.0f, -4.0f, 18.0f,  8.0f },  // arm R
        { -8.0f, 16.0f, 10.0f, 20.0f },  // leg L
        {  8.0f, 16.0f, 10.0f, 20.0f },  // leg R
    };

    // Joint: bodyA index, bodyB index, anchor offset from bodyA center (pixels)
    struct JointDef { int a, b; float ax, ay; float lo, hi; };
    constexpr JointDef JOINT_DEFS[RagdollInstance::JOINTS] = {
        { 0, 1,   0.0f,-10.0f,  -30.0f,  30.0f },  // torso → head
        { 0, 2, -14.0f,  0.0f,  -80.0f,  20.0f },  // torso → arm L
        { 0, 3,  14.0f,  0.0f,  -20.0f,  80.0f },  // torso → arm R
        { 0, 4,  -8.0f, 10.0f,  -10.0f, 100.0f },  // torso → leg L
        { 0, 5,   8.0f, 10.0f, -100.0f,  10.0f },  // torso → leg R
    };

    // Neon palette colors per body part
    const sf::Color PART_COLORS[RagdollInstance::BODIES] = {
        sf::Color(0x1A, 0x28, 0x40, 0xFF),  // torso — UI border
        sf::Color(0x0A, 0x10, 0x20, 0xFF),  // head  — UI base
        sf::Color(0x00, 0xFF, 0xEE, 0xFF),  // arm L — neon cyan
        sf::Color(0x00, 0xFF, 0xEE, 0xFF),  // arm R — neon cyan
        sf::Color(0x1A, 0x28, 0x40, 0xFF),  // leg L
        sf::Color(0x1A, 0x28, 0x40, 0xFF),  // leg R
    };
}

RagdollSystem::RagdollSystem(PhysicsSystem& physics)
    : m_physics(physics)
{
    m_diedHandle = EventBus::on<EnemyDiedEvent>([this](const EnemyDiedEvent& e) {
        onEnemyDied(e);
    });
}

RagdollSystem::~RagdollSystem() {
    EventBus::unsubscribe<EnemyDiedEvent>(m_diedHandle);
    for (auto& r : m_pool)
        if (r.active) freeInstance(r);
}

void RagdollSystem::onEnemyDied(const EnemyDiedEvent& e) {
    // Impulse direction: slight upward + random horizontal
    float ix = (e.x > 0.f) ? -80.f : 80.f;
    float iy = -120.f;
    spawnRagdoll(e.x, e.y, ix, iy);
}

void RagdollSystem::spawnRagdoll(float cx, float cy, float impulseX, float impulseY) {
    // Find a free slot (reuse oldest active if full)
    RagdollInstance* slot = nullptr;
    RagdollInstance* oldest = nullptr;
    float minLife = 999.f;

    for (auto& r : m_pool) {
        if (!r.active) { slot = &r; break; }
        if (r.lifetime < minLife) { minLife = r.lifetime; oldest = &r; }
    }
    if (!slot) {
        freeInstance(*oldest);
        slot = oldest;
    }

    *slot = RagdollInstance{};
    slot->active   = true;
    slot->lifetime = 4.0f;
    slot->alpha    = 1.0f;

    // Create bodies
    for (int i = 0; i < RagdollInstance::BODIES; ++i) {
        float bx = cx + PARTS[i].ox - PARTS[i].w * 0.5f;
        float by = cy + PARTS[i].oy - PARTS[i].h * 0.5f;
        b2Body* body = m_physics.createDynamicBody(bx, by, PARTS[i].w, PARTS[i].h);
        // Allow rotation for ragdoll bodies
        body->SetFixedRotation(false);
        body->SetLinearDamping(0.8f);
        body->SetAngularDamping(1.5f);
        slot->bodies[i] = body;
    }

    // Apply impulse to torso
    b2Vec2 imp{ PhysicsSystem::toMeters(impulseX), PhysicsSystem::toMeters(impulseY) };
    slot->bodies[0]->ApplyLinearImpulseToCenter(imp, true);

    // Create joints
    for (int j = 0; j < RagdollInstance::JOINTS; ++j) {
        const auto& jd = JOINT_DEFS[j];
        b2Body* bA = slot->bodies[jd.a];
        b2Vec2 anchor = bA->GetPosition();
        anchor.x += PhysicsSystem::toMeters(jd.ax);
        anchor.y += PhysicsSystem::toMeters(jd.ay);
        slot->joints[j] = m_physics.createRevoluteJoint(
            bA, slot->bodies[jd.b], anchor, jd.lo, jd.hi);
    }
}

void RagdollSystem::freeInstance(RagdollInstance& r) {
    for (int j = RagdollInstance::JOINTS - 1; j >= 0; --j)
        if (r.joints[j]) { m_physics.destroyJoint(r.joints[j]); r.joints[j] = nullptr; }
    for (int i = 0; i < RagdollInstance::BODIES; ++i)
        if (r.bodies[i]) { m_physics.destroyBody(r.bodies[i]); r.bodies[i] = nullptr; }
    r.active = false;
}

void RagdollSystem::update(float dt) {
    constexpr float FADE_START = 1.0f;  // begin fading when 1s remains

    for (auto& r : m_pool) {
        if (!r.active) continue;
        r.lifetime -= dt;
        if (r.lifetime <= 0.f) {
            freeInstance(r);
            continue;
        }
        r.alpha = (r.lifetime < FADE_START)
                  ? r.lifetime / FADE_START
                  : 1.0f;
    }
}

void RagdollSystem::render(SpriteBatch& batch) const {
    for (const auto& r : m_pool) {
        if (!r.active) continue;
        uint8_t a = static_cast<uint8_t>(r.alpha * 255.f);

        for (int i = 0; i < RagdollInstance::BODIES; ++i) {
            if (!r.bodies[i]) continue;
            const b2Vec2& pos = r.bodies[i]->GetPosition();

            float px = PhysicsSystem::toPixels(pos.x) - PARTS[i].w * 0.5f;
            float py = PhysicsSystem::toPixels(pos.y) - PARTS[i].h * 0.5f;

            sf::Color col = PART_COLORS[i];
            col.a = a;

            batch.draw({ px, py }, { PARTS[i].w, PARTS[i].h }, col);
        }
    }
}
