#include "NavMesh.h"
#include <cmath>
#include <limits>

void NavMesh::build(const std::vector<std::vector<int>>& collision,
                    int width, int height, int tileSize) {
    m_width    = width;
    m_height   = height;
    m_tileSize = tileSize;
    m_nodes.clear();
    m_grid.assign(static_cast<size_t>(width * height), -1);

    auto isSolid = [&](int c, int r) -> bool {
        if (r < 0 || r >= height || c < 0 || c >= width) return false;
        if (r >= static_cast<int>(collision.size())) return false;
        if (c >= static_cast<int>(collision[static_cast<size_t>(r)].size())) return false;
        return collision[static_cast<size_t>(r)][static_cast<size_t>(c)] != 0;
    };

    float ts = static_cast<float>(tileSize);

    // First pass: identify walkable nodes
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            // Walkable: not solid AND (at bottom OR has solid tile below)
            if (isSolid(c, r)) continue;
            if (!isSolid(c, r + 1) && r + 1 < height) continue; // no floor below

            NavNode n;
            n.col    = c;
            n.row    = r;
            n.worldX = c * ts + ts * 0.5f;
            n.worldY = r * ts + ts * 0.5f;
            n.neighborCount = 0;
            for (int& nb : n.neighborIdx) nb = -1;

            int idx = static_cast<int>(m_nodes.size());
            m_grid[static_cast<size_t>(r * width + c)] = idx;
            m_nodes.push_back(n);
        }
    }

    // Second pass: connect horizontal neighbors
    for (auto& n : m_nodes) {
        auto tryConnect = [&](int dc, int dr) {
            int nc = n.col + dc;
            int nr = n.row + dr;
            if (nc < 0 || nc >= width || nr < 0 || nr >= height) return;
            int nidx = m_grid[static_cast<size_t>(nr * width + nc)];
            if (nidx == -1) return;
            if (n.neighborCount < 4)
                n.neighborIdx[n.neighborCount++] = nidx;
        };
        tryConnect(-1, 0); // left
        tryConnect( 1, 0); // right
    }
}

int NavMesh::nearestNode(float worldX, float worldY) const {
    if (m_nodes.empty()) return -1;
    float bestDist = std::numeric_limits<float>::max();
    int   bestIdx  = 0;
    for (int i = 0; i < static_cast<int>(m_nodes.size()); ++i) {
        float dx = m_nodes[static_cast<size_t>(i)].worldX - worldX;
        float dy = m_nodes[static_cast<size_t>(i)].worldY - worldY;
        float d  = dx * dx + dy * dy;
        if (d < bestDist) { bestDist = d; bestIdx = i; }
    }
    return bestIdx;
}
