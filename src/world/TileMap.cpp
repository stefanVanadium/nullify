#include "TileMap.h"

static const sf::Color TILE_COLOR = sf::Color(0x1A, 0x28, 0x40, 0xFF);
static constexpr float TILE_INSET = 1.0f; // 1px gap creates pseudo-outline

void TileMap::build(const TileMapData& data, PhysicsSystem& physics) {
    m_vertexCount = 0;
    float ts = static_cast<float>(data.tileSize);

    int rowCount = static_cast<int>(data.collision.size());
    for (int row = 0; row < rowCount; ++row) {
        for (int col = 0; col < data.width; ++col) {
            if (col >= static_cast<int>(data.collision[row].size())) break;
            if (data.collision[row][col] == 0) continue;

            float px = col * ts;
            float py = row * ts;

            // Static Box2D body for collision — pointer discarded; body lives in b2World
            physics.createStaticBody(px, py, ts, ts);

            // Add quad to vertex array (slightly inset for pseudo-outline effect)
            if (m_vertexCount + 4 > MAX_TILE_VERTS) continue;

            float x0 = px + TILE_INSET;
            float y0 = py + TILE_INSET;
            float x1 = px + ts - TILE_INSET;
            float y1 = py + ts - TILE_INSET;

            m_vertices[m_vertexCount + 0] = {sf::Vector2f(x0, y0), TILE_COLOR};
            m_vertices[m_vertexCount + 1] = {sf::Vector2f(x1, y0), TILE_COLOR};
            m_vertices[m_vertexCount + 2] = {sf::Vector2f(x1, y1), TILE_COLOR};
            m_vertices[m_vertexCount + 3] = {sf::Vector2f(x0, y1), TILE_COLOR};
            m_vertexCount += 4;
        }
    }
}
