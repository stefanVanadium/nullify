#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>
#include "core/EventBus.h"
#include "rendering/SpriteBatch.h"

struct Particle {
    float     x, y;
    float     vx, vy;
    float     life;
    float     maxLife;
    sf::Color color;
    float     size;
    bool      active = false;
};

class ParticleSystem {
public:
    static constexpr size_t POOL_SIZE = 4096;

    ParticleSystem();
    ~ParticleSystem();

    void spawnBulletImpact(float x, float y);
    void spawnBlood(float x, float y);
    void update(float dt);
    void batchDraw(SpriteBatch& batch) const;

private:
    std::array<Particle, POOL_SIZE> m_pool{};
    size_t m_nextSlot = 0;

    Particle& nextParticle();

    EventBus::Handle m_bulletHitHandle = 0;
    EventBus::Handle m_enemyDiedHandle  = 0;
};
