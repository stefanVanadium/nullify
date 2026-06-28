#pragma once
#include <vector>
#include <array>
#include "world/TileMap.h"

// Simple grid-based navmesh for 2D side-scroller.
// A tile is a navmesh node if it is walkable (not solid) AND has a solid tile below it.
// Connections: horizontal adjacency on the same walkable row.
struct NavNode {
    int   col, row;
    float worldX, worldY; // center of the tile in pixels
    int   neighborIdx[4]; // indices into NavMesh::m_nodes; -1 = no neighbor
    int   neighborCount = 0;
};

class NavMesh {
public:
    // Build from a collision layer. tileSize in pixels.
    void build(const std::vector<std::vector<int>>& collision,
               int width, int height, int tileSize);

    // Find node nearest to world position. Returns -1 if navmesh is empty.
    int nearestNode(float worldX, float worldY) const;

    const NavNode& node(int idx) const { return m_nodes[static_cast<size_t>(idx)]; }
    int            nodeCount()   const { return static_cast<int>(m_nodes.size()); }

private:
    std::vector<NavNode> m_nodes;
    // Grid lookup: m_grid[row * m_width + col] = node index or -1
    std::vector<int>     m_grid;
    int                  m_width  = 0;
    int                  m_height = 0;
    int                  m_tileSize = 32;
};
