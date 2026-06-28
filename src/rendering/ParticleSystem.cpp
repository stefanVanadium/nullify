#include "ParticleSystem.h"
#include <cmath>

// Zero-heap LCG for particle randomness in hot path
static uint32_t s_rng = 54321u;
static float rand01() {
    s_rng = s_rng * 1664525u + 1013904223u;
    return static_cast<float>(s_rng >> 8) / 16777216.0f;
}
static float randSigned() { return rand01() * 2.f - 1.f; }

ParticleSystem::ParticleSystem() {
    for (auto& p : m_pool) p.active = false;

    m_bulletHitHandle = EventBus::on<BulletHitEvent>([this](const BulletHitEvent& e) {
        spawnBulletImpact(e.x, e.y);
    });
    m_enemyDiedHandle = EventBus::on<EnemyDiedEvent>([this](const EnemyDiedEvent& e) {
        spawnBlood(e.x + 12.f, e.y + 12.f);
    });
}

ParticleSystem::~ParticleSystem() {
    EventBus::unsubscribe<BulletHitEvent>(m_bulletHitHandle);
    EventBus::unsubscribe<EnemyDiedEvent>(m_enemyDiedHandle);
}

Particle& ParticleSystem::nextParticle() {
    Particle& p = m_pool[m_nextSlot];
    m_nextSlot = (m_nextSlot + 1) % POOL_SIZE;
    return p;
}

void ParticleSystem::spawnBulletImpact(float x, float y) {
    for (int i = 0; i < 6; ++i) {
        Particle& p = nextParticle();
        float angle = rand01() * 6.2832f;
        float speed = 60.f + rand01() * 120.f;
        p.x       = x;
        p.y       = y;
        p.vx      = std::cos(angle) * speed;
        p.vy      = std::sin(angle) * speed;
        p.life    = 0.22f + rand01() * 0.18f;
        p.maxLife = p.life;
        p.color   = sf::Color(0x00, 0xFF, 0xEE, 0xFF);
        p.size    = 2.f + rand01() * 2.f;
        p.active  = true;
    }
}

void ParticleSystem::spawnBlood(float x, float y) {
    static const sf::Color COLORS[2] = {
        sf::Color(0x00, 0xFF, 0xEE, 0xFF),
        sf::Color(0xFF, 0x00, 0x38, 0xFF)
    };
    for (int i = 0; i < 5; ++i) {
        Particle& p = nextParticle();
        float angle = rand01() * 6.2832f;
        float speed = 40.f + rand01() * 80.f;
        p.x       = x + randSigned() * 8.f;
        p.y       = y + randSigned() * 8.f;
        p.vx      = std::cos(angle) * speed;
        p.vy      = std::sin(angle) * speed + 30.f;
        p.life    = 0.35f + rand01() * 0.25f;
        p.maxLife = p.life;
        p.color   = COLORS[i % 2];
        p.size    = 3.f + rand01() * 3.f;
        p.active  = true;
    }
}

void ParticleSystem::update(float dt) {
    for (auto& p : m_pool) {
        if (!p.active) continue;
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.life -= dt;
        if (p.life <= 0.f) { p.active = false; continue; }
        p.color.a = static_cast<uint8_t>((p.life / p.maxLife) * 255.f);
    }
}

void ParticleSystem::batchDraw(SpriteBatch& batch) const {
    for (const auto& p : m_pool) {
        if (!p.active) continue;
        batch.draw(
            {p.x - p.size * 0.5f, p.y - p.size * 0.5f},
            {p.size, p.size},
            p.color
        );
    }
}
