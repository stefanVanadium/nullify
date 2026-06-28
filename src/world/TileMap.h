#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"

struct TileMapData {
    int width    = 0;
    int height   = 0;
    int tileSize = 32;
    std::vector<std::vector<int>> collision; // [row][col], 0=empty 1=solid
};

class TileMap {
public:
    // Build ECS entities and Box2D bodies from tile data
    void build(const TileMapData& data, World& world, PhysicsSystem& physics);

private:
    // Creates one Box2D static box per tile (simple; edge chain is Sprint 2 optimization)
    void buildCollision(const TileMapData& data, World& world, PhysicsSystem& physics);
};
