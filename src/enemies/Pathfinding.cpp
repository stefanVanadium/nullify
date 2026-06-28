#include "Pathfinding.h"
#include <array>
#include <cmath>
#include <limits>

// Minimal A* using fixed-size arrays to avoid heap allocation.
static constexpr int MAX_OPEN  = 512;
static constexpr int MAX_NODES = 1024; // must be >= NavMesh node count

struct AStarNode {
    float g, f;
    int   parent;
    bool  open, closed;
};

int Pathfinding::findPath(const NavMesh& mesh,
                          int            startNode,
                          float          goalX,
                          float          goalY,
                          AIState&       outState) {
    int count = mesh.nodeCount();
    if (count == 0 || startNode < 0 || startNode >= count) return 0;

    int goalNode = mesh.nearestNode(goalX, goalY);
    if (goalNode < 0 || goalNode >= count) return 0;
    if (goalNode == startNode) {
        outState.pathLen    = 0;
        outState.pathCursor = 0;
        return 0;
    }

    if (count > MAX_NODES) count = MAX_NODES;

    static std::array<AStarNode, MAX_NODES> nodes;
    for (int i = 0; i < count; ++i)
        nodes[static_cast<size_t>(i)] = { 1e9f, 1e9f, -1, false, false };

    static std::array<int, MAX_OPEN> openList;
    int openCount = 0;

    auto heuristic = [&](int idx) -> float {
        float dx = mesh.node(idx).worldX - goalX;
        float dy = mesh.node(idx).worldY - goalY;
        return std::sqrt(dx * dx + dy * dy);
    };

    nodes[static_cast<size_t>(startNode)].g    = 0.0f;
    nodes[static_cast<size_t>(startNode)].f    = heuristic(startNode);
    nodes[static_cast<size_t>(startNode)].open = true;
    openList[openCount++] = startNode;

    int found = -1;
    while (openCount > 0) {
        // Find node with lowest f in open list
        int  bestIdx = 0;
        float bestF  = nodes[static_cast<size_t>(openList[0])].f;
        for (int i = 1; i < openCount; ++i) {
            float f = nodes[static_cast<size_t>(openList[i])].f;
            if (f < bestF) { bestF = f; bestIdx = i; }
        }
        int current = openList[bestIdx];
        // Remove from open list (swap with last)
        openList[bestIdx] = openList[--openCount];
        nodes[static_cast<size_t>(current)].open   = false;
        nodes[static_cast<size_t>(current)].closed = true;

        if (current == goalNode) { found = current; break; }

        const NavNode& cn = mesh.node(current);
        for (int ni = 0; ni < cn.neighborCount; ++ni) {
            int nb = cn.neighborIdx[ni];
            if (nb < 0 || nb >= count) continue;
            if (nodes[static_cast<size_t>(nb)].closed) continue;

            const NavNode& nn = mesh.node(nb);
            float dx = nn.worldX - cn.worldX;
            float dy = nn.worldY - cn.worldY;
            float newG = nodes[static_cast<size_t>(current)].g + std::sqrt(dx*dx + dy*dy);

            if (newG < nodes[static_cast<size_t>(nb)].g) {
                nodes[static_cast<size_t>(nb)].g      = newG;
                nodes[static_cast<size_t>(nb)].f      = newG + heuristic(nb);
                nodes[static_cast<size_t>(nb)].parent = current;

                if (!nodes[static_cast<size_t>(nb)].open && openCount < MAX_OPEN) {
                    nodes[static_cast<size_t>(nb)].open = true;
                    openList[openCount++] = nb;
                }
            }
        }
    }

    if (found < 0) return 0;

    // Reconstruct path into AIState (reversed then corrected)
    static std::array<int, AIState::MAX_PATH> pathNodes;
    int pathLen = 0;
    int cur = found;
    while (cur != -1 && pathLen < AIState::MAX_PATH) {
        pathNodes[static_cast<size_t>(pathLen++)] = cur;
        cur = nodes[static_cast<size_t>(cur)].parent;
    }

    // Reverse into outState
    int written = 0;
    for (int i = pathLen - 1; i >= 0 && written < AIState::MAX_PATH; --i) {
        int idx = pathNodes[static_cast<size_t>(i)];
        outState.pathX[written] = mesh.node(idx).worldX;
        outState.pathY[written] = mesh.node(idx).worldY;
        ++written;
    }
    outState.pathLen    = written;
    outState.pathCursor = 0;
    return written;
}
