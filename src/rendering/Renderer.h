#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include "ecs/World.h"
#include "SpriteBatch.h"
#include "ShaderManager.h"
#include "ParallaxSystem.h"
#include "world/TileMap.h"

class Renderer {
public:
    void init(sf::Vector2u windowSize);

    // Returns draw call count for the debug title.
    int render(sf::RenderWindow& window,
               World&            world,
               float             alpha,
               float             hpRatio,         // 0..1, drives vignette intensity
               sf::Vector2f      mouseWorldPos,
               const TileMap&    tileMap,
               ParallaxSystem&   parallax,
               ShaderManager&    shaders);

private:
    SpriteBatch m_entityBatch;

    // Full-screen overlay quads (screen space, pre-built in init)
    std::array<sf::Vertex, 4> m_scanlineQuad{};
    std::array<sf::Vertex, 4> m_vignetteQuad{};

    // Crosshair circle
    sf::CircleShape m_crosshair;
};
