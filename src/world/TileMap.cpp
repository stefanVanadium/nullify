#include "TileMap.h"
#include "ecs/Components.h"

void TileMap::build(const TileMapData& data, World& world, PhysicsSystem& physics) {
    buildCollision(data, world, physics);
}

void TileMap::buildCollision(const TileMapData& data, World& world, PhysicsSystem& physics) {
    float ts = static_cast<float>(data.tileSize);

    for (int row = 0; row < data.height; ++row) {
        if (row >= static_cast<int>(data.collision.size())) break;
        for (int col = 0; col < data.width; ++col) {
            if (col >= static_cast<int>(data.collision[row].size())) break;
            if (data.collision[row][col] == 0) continue;

            float px = col * ts;
            float py = row * ts;

            // Static physics body
            b2Body* body = physics.createStaticBody(px, py, ts, ts);

            // ECS entity for rendering the tile
            uint32_t id = world.createEntity();

            Transform t{};
            t.x = px; t.y = py;
            t.prevX = px; t.prevY = py;
            world.addComponent<Transform>(id, std::move(t));

            Renderable r;
            r.shape.setSize({ts, ts});
            r.shape.setFillColor(sf::Color(0x1A, 0x28, 0x40)); // UI border color
            r.shape.setOutlineColor(sf::Color(0x0A, 0x10, 0x20));
            r.shape.setOutlineThickness(1.0f);
            r.layer   = 0;
            r.visible = true;
            world.addComponent<Renderable>(id, std::move(r));

            Collidable c{};
            c.body = body;
            c.w    = ts;
            c.h    = ts;
            world.addComponent<Collidable>(id, std::move(c));

            world.addComponent<TileTag>(id, TileTag{});
        }
    }
}
