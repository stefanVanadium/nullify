#pragma once
#include "NavMesh.h"
#include "ecs/Components.h"  // for AIState::MAX_PATH

// Standard A* on a NavMesh. Returns path length (0 = no path).
// Path is written into AIState.pathX/pathY/pathLen.
// Budget: max 4 calls per frame total (enforced by EnemyManager).
class Pathfinding {
public:
    // Finds path from startNode to nearest node to (goalX, goalY).
    // Writes result into outState. Returns number of nodes in path (0 = fail).
    static int findPath(const NavMesh& mesh,
                        int            startNode,
                        float          goalX,
                        float          goalY,
                        AIState&       outState);
};
