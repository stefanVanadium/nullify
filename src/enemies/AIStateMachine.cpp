#include "AIStateMachine.h"
#include "Pathfinding.h"
#include "ecs/Components.h"
#include "core/EventBus.h"
#include <cmath>
#include <algorithm>

static float dist2(float ax, float ay, float bx, float by) {
    float dx = bx - ax, dy = by - ay;
    return dx * dx + dy * dy;
}

static void moveToward(b2Body* body, float targetX, float targetY,
                       float tx, float ty, float speedPx) {
    float dx = targetX - tx;
    float dy = targetY - ty;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len < 1.0f) { body->SetLinearVelocity({0.f, body->GetLinearVelocity().y}); return; }
    dx /= len; dy /= len;
    float speedM = PhysicsSystem::toMeters(speedPx);
    b2Vec2 v = body->GetLinearVelocity();
    body->SetLinearVelocity({dx * speedM, v.y});
}

static bool checkLOS(uint32_t enemyId, uint32_t playerId,
                     World& world, PhysicsSystem& physics) {
    if (!world.hasComponent<Transform>(enemyId) || !world.hasComponent<Transform>(playerId))
        return false;
    const auto& et = world.getComponent<Transform>(enemyId);
    const auto& pt = world.getComponent<Transform>(playerId);
    float ex = et.x + EnemyConfig::SCOUT_WIDTH  * 0.5f;
    float ey = et.y + EnemyConfig::SCOUT_HEIGHT * 0.5f;
    float px = pt.x + 12.0f;
    float py = pt.y + 24.0f;
    b2Vec2 from = { PhysicsSystem::toMeters(ex), PhysicsSystem::toMeters(ey) };
    b2Vec2 to   = { PhysicsSystem::toMeters(px), PhysicsSystem::toMeters(py) };
    // Ignore the player's own body — ray endpoint is inside it, so it would
    // always block the cast even when nothing is between enemy and player.
    b2Body* playerBody = world.hasComponent<Collidable>(playerId)
                       ? world.getComponent<Collidable>(playerId).body
                       : nullptr;
    return physics.rayCastClear(from, to, playerBody);
}

void AIStateMachine::update(uint32_t       enemyId,
                            float          dt,
                            uint32_t       playerEntityId,
                            World&         world,
                            PhysicsSystem& physics,
                            const NavMesh& navMesh,
                            int&           aStarBudget) {
    if (!world.isAlive(enemyId)) return;
    if (!world.hasComponent<AIState>(enemyId))      return;
    if (!world.hasComponent<Transform>(enemyId))    return;
    if (!world.hasComponent<Collidable>(enemyId))   return;

    auto& ai  = world.getComponent<AIState>(enemyId);
    auto& tf  = world.getComponent<Transform>(enemyId);
    b2Body* body = world.getComponent<Collidable>(enemyId).body;
    if (!body) return;

    float ex = tf.x + EnemyConfig::SCOUT_WIDTH  * 0.5f;
    float ey = tf.y + EnemyConfig::SCOUT_HEIGHT * 0.5f;

    bool hasPlayer = (playerEntityId != static_cast<uint32_t>(MAX_ENTITIES))
                  && world.isAlive(playerEntityId);

    float px = 0.f, py = 0.f;
    float playerDist2 = 1e9f;
    if (hasPlayer && world.hasComponent<Transform>(playerEntityId)) {
        const auto& pt = world.getComponent<Transform>(playerEntityId);
        px = pt.x + 12.0f;
        py = pt.y + 24.0f;
        playerDist2 = dist2(ex, ey, px, py);
    }

    ai.stateTimer += dt;
    if (ai.attackTimer > 0.f) ai.attackTimer -= dt;

    switch (ai.current) {
    // ── PATROL ──────────────────────────────────────────────────────────────
    case AIStateEnum::PATROL: {
        // Check for player LOS
        if (hasPlayer && playerDist2 < EnemyConfig::SCOUT_DETECT_RANGE * EnemyConfig::SCOUT_DETECT_RANGE) {
            ai.hasLOS = checkLOS(enemyId, playerEntityId, world, physics);
            if (ai.hasLOS) {
                ai.lastSeenX  = px; ai.lastSeenY  = py;
                ai.stateTimer = 0.f;
                ai.current    = AIStateEnum::COMBAT;
                EventBus::emit(EnemyAlertedEvent{ enemyId, px, py });
                break;
            }
        }
        // Walk patrol waypoints
        if (!world.hasComponent<WaypointPath>(enemyId)) break;
        const auto& wp = world.getComponent<WaypointPath>(enemyId);
        if (wp.count == 0) break;
        int  widx   = ai.waypointIdx % wp.count;
        float wpx   = wp.x[widx];
        float wpy   = wp.y[widx];
        float d2    = dist2(ex, ey, wpx, wpy);
        if (d2 < EnemyConfig::WAYPOINT_ARRIVE_DIST * EnemyConfig::WAYPOINT_ARRIVE_DIST) {
            ai.waypointIdx = (ai.waypointIdx + 1) % wp.count;
        } else {
            moveToward(body, wpx, wpy, ex, ey, EnemyConfig::SCOUT_PATROL_SPEED);
        }
        break;
    }

    // ── ALERT ───────────────────────────────────────────────────────────────
    case AIStateEnum::ALERT: {
        if (hasPlayer && playerDist2 < EnemyConfig::SCOUT_DETECT_RANGE * EnemyConfig::SCOUT_DETECT_RANGE) {
            ai.hasLOS = checkLOS(enemyId, playerEntityId, world, physics);
            if (ai.hasLOS) {
                ai.lastSeenX  = px; ai.lastSeenY  = py;
                ai.stateTimer = 0.f;
                ai.current    = AIStateEnum::COMBAT;
                break;
            }
        }
        float d2 = dist2(ex, ey, ai.alertX, ai.alertY);
        if (d2 > EnemyConfig::WAYPOINT_ARRIVE_DIST * EnemyConfig::WAYPOINT_ARRIVE_DIST)
            moveToward(body, ai.alertX, ai.alertY, ex, ey, EnemyConfig::SCOUT_PATROL_SPEED);
        else
            body->SetLinearVelocity({0.f, body->GetLinearVelocity().y});
        if (ai.stateTimer >= EnemyConfig::ALERT_DURATION) {
            ai.stateTimer = 0.f;
            ai.current    = AIStateEnum::PATROL;
        }
        break;
    }

    // ── COMBAT ──────────────────────────────────────────────────────────────
    case AIStateEnum::COMBAT: {
        if (!hasPlayer) { ai.stateTimer = 0.f; ai.current = AIStateEnum::SEARCH; break; }

        ai.hasLOS = checkLOS(enemyId, playerEntityId, world, physics);
        if (ai.hasLOS) {
            ai.lastSeenX  = px; ai.lastSeenY  = py;
            ai.stateTimer = 0.f;
        }

        if (!ai.hasLOS && ai.stateTimer >= EnemyConfig::LOST_LOS_TIMEOUT) {
            ai.stateTimer = 0.f;
            ai.pathDirty  = true;
            ai.current    = AIStateEnum::SEARCH;
            break;
        }

        if (playerDist2 > EnemyConfig::SCOUT_ATTACK_RANGE * EnemyConfig::SCOUT_ATTACK_RANGE) {
            // Move toward player
            if (ai.hasLOS) {
                moveToward(body, px, py, ex, ey, EnemyConfig::SCOUT_COMBAT_SPEED);
            } else {
                // Follow A* path toward last known position
                if (ai.pathDirty && aStarBudget > 0) {
                    --aStarBudget;
                    int startNode = navMesh.nearestNode(ex, ey);
                    Pathfinding::findPath(navMesh, startNode, ai.lastSeenX, ai.lastSeenY, ai);
                    ai.pathDirty = false;
                }
                if (ai.pathCursor < ai.pathLen) {
                    float wx = ai.pathX[ai.pathCursor];
                    float wy = ai.pathY[ai.pathCursor];
                    float d  = dist2(ex, ey, wx, wy);
                    if (d < EnemyConfig::WAYPOINT_ARRIVE_DIST * EnemyConfig::WAYPOINT_ARRIVE_DIST)
                        ++ai.pathCursor;
                    else
                        moveToward(body, wx, wy, ex, ey, EnemyConfig::SCOUT_COMBAT_SPEED);
                }
            }
        } else {
            body->SetLinearVelocity({0.f, body->GetLinearVelocity().y});
            // Fire projectile bullet toward player
            if (ai.hasLOS && ai.attackTimer <= 0.f) {
                ai.attackTimer = EnemyConfig::SCOUT_ATTACK_INTERVAL;
                float dx = px - ex, dy = py - ey;
                float len = std::sqrt(dx*dx + dy*dy);
                if (len > 0.1f) { dx /= len; dy /= len; }
                EventBus::emit(EnemyFireEvent{ enemyId, ex, ey, dx, dy });
            }
        }
        break;
    }

    // ── SEARCH ──────────────────────────────────────────────────────────────
    case AIStateEnum::SEARCH: {
        if (hasPlayer && playerDist2 < EnemyConfig::SCOUT_DETECT_RANGE * EnemyConfig::SCOUT_DETECT_RANGE) {
            ai.hasLOS = checkLOS(enemyId, playerEntityId, world, physics);
            if (ai.hasLOS) {
                ai.lastSeenX  = px; ai.lastSeenY = py;
                ai.stateTimer = 0.f; ai.pathDirty = true;
                ai.current    = AIStateEnum::COMBAT;
                break;
            }
        }
        // A* to last known player position
        if (ai.pathDirty && aStarBudget > 0) {
            --aStarBudget;
            int startNode = navMesh.nearestNode(ex, ey);
            Pathfinding::findPath(navMesh, startNode, ai.lastSeenX, ai.lastSeenY, ai);
            ai.pathDirty = false;
        }
        if (ai.pathCursor < ai.pathLen) {
            float wx = ai.pathX[ai.pathCursor];
            float wy = ai.pathY[ai.pathCursor];
            float d  = dist2(ex, ey, wx, wy);
            if (d < EnemyConfig::WAYPOINT_ARRIVE_DIST * EnemyConfig::WAYPOINT_ARRIVE_DIST)
                ++ai.pathCursor;
            else
                moveToward(body, wx, wy, ex, ey, EnemyConfig::SCOUT_PATROL_SPEED);
        } else {
            body->SetLinearVelocity({0.f, body->GetLinearVelocity().y});
        }
        if (ai.stateTimer >= EnemyConfig::SEARCH_DURATION) {
            ai.stateTimer = 0.f; ai.waypointIdx = 0;
            ai.current    = AIStateEnum::PATROL;
        }
        break;
    }
    }
}
