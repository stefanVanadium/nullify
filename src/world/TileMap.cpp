#include "TileMap.h"

static const sf::Color TILE_COLOR = sf::Color(0x1A, 0x28, 0x40, 0xFF);
static constexpr float TILE_INSET = 1.0f;

void TileMap::build(const TileMapData& data, PhysicsSystem& physics) {
    m_vertexCount = 0;
    float ts = static_cast<float>(data.tileSize);

    int rowCount = static_cast<int>(data.collision.size());
    for (int row = 0; row < rowCount; ++row) {
        const auto& rowData = data.collision[row];
        int colCount = static_cast<int>(rowData.size());

        // Build render quads and collect contiguous solid runs for edge chains
        int groupStart = -1;
        for (int col = 0; col <= colCount; ++col) {
            bool solid = (col < colCount) && (rowData[col] != 0);

            if (solid) {
                if (groupStart < 0) groupStart = col;  // start new run

                // Render quad
                if (m_vertexCount + 4 <= MAX_TILE_VERTS) {
                    float px = col * ts, py = row * ts;
                    float x0 = px + TILE_INSET, y0 = py + TILE_INSET;
                    float x1 = px + ts - TILE_INSET, y1 = py + ts - TILE_INSET;
                    m_vertices[m_vertexCount + 0] = {sf::Vector2f(x0, y0), TILE_COLOR};
                    m_vertices[m_vertexCount + 1] = {sf::Vector2f(x1, y0), TILE_COLOR};
                    m_vertices[m_vertexCount + 2] = {sf::Vector2f(x1, y1), TILE_COLOR};
                    m_vertices[m_vertexCount + 3] = {sf::Vector2f(x0, y1), TILE_COLOR};
                    m_vertexCount += 4;
                }
            } else {
                if (groupStart >= 0) {
                    // End of a solid run [groupStart, col-1] — emit one edge chain
                    float x0 = PhysicsSystem::toMeters(groupStart * ts);
                    float x1 = PhysicsSystem::toMeters(col * ts);
                    float y0 = PhysicsSystem::toMeters(row * ts);
                    float y1 = PhysicsSystem::toMeters((row + 1) * ts);
                    // Counter-clockwise loop (top-left, top-right, bottom-right, bottom-left)
                    b2Vec2 verts[4] = {
                        {x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}
                    };
                    physics.createEdgeChain(verts, 4);
                    groupStart = -1;
                }
            }
        }
    }
}
