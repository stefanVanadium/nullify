#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include "ecs/World.h"
#include "SpriteBatch.h"
#include "ShaderManager.h"
#include "ParallaxSystem.h"
#include "ParticleSystem.h"
#include "RagdollSystem.h"
#include "world/TileMap.h"
#include "player/WeaponSystem.h"

class EnemyManager;  // forward-declared to avoid include chain

struct RenderEffects {
    float caIntensity   = 0.f;  // chromatic aberration 0-1
    float hackIntensity = 0.f;  // glitch 0-1
    float gameTime      = 0.f;  // elapsed seconds for glitch animation
};

class Renderer {
public:
    void init(sf::Vector2u windowSize);

    // Returns draw call count for the debug title.
    int render(sf::RenderWindow&     window,
               World&                world,
               float                 alpha,
               float                 hpRatio,
               sf::Vector2f          mouseWorldPos,
               const TileMap&        tileMap,
               ParallaxSystem&       parallax,
               ShaderManager&        shaders,
               const WeaponSystem&   weapon,
               const EnemyManager&   enemies,
               ParticleSystem&       particles,
               const RagdollSystem&  ragdolls,
               const RenderEffects&  effects);

private:
    sf::RenderTexture m_sceneRT;
    sf::Sprite        m_sceneSprite;
    SpriteBatch       m_entityBatch;
    SpriteBatch       m_particleBatch;

    // Full-screen overlay quads (screen space, pre-built in init)
    std::array<sf::Vertex, 4> m_scanlineQuad{};
    std::array<sf::Vertex, 4> m_vignetteQuad{};

    sf::CircleShape m_crosshair;
    float           m_sceneW = 0.f;
    float           m_sceneH = 0.f;
};
