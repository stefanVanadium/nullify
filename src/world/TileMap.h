#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "ecs/Systems/PhysicsSystem.h"

struct TileMapData {
    int width    = 0;
    int height   = 0;
    int tileSize = 32;
    std::vector<std::vector<int>> collision; // [row][col], 0=empty 1=solid
};

class TileMap {
public:
    // Build Box2D static bodies + static vertex array from tile data.
    // Does NOT create any ECS entities — tiles are purely physics + geometry.
    void build(const TileMapData& data, PhysicsSystem& physics);

    const sf::Vertex* vertices()      const { return m_vertices.data(); }
    size_t            vertexCount()   const { return m_vertexCount; }

private:
    static constexpr size_t MAX_TILE_VERTS = 8192 * 4; // 8192 tiles × 4 verts

    std::array<sf::Vertex, MAX_TILE_VERTS> m_vertices{};
    size_t                                  m_vertexCount = 0;
};
