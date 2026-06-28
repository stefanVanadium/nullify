#include "AIStateMachine.h"
#include "Pathfinding.h"
#include "ecs/Components.h"
#include "core/EventBus.h"
#include <cmath>
#include <algorithm>

// ---- Helpers ---------------------------------------------------------------

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
    dx /= len;
    float speedM = PhysicsSystem::toMeters(speedPx);
    b2Vec2 v = body->GetLinearVelocity();
    body->SetLinearVelocity({dx * speedM, v.y});
}

static bool checkLOS(uint32_t enemyId, uint32_t playerId,
                     World& world, PhysicsSystem& physics,
                     float halfW, float halfH) {
    if (!world.hasComponent<Transform>(enemyId) || !world.hasComponent<Transform>(playerId))
        return false;
    const auto& et = world.getComponent<Transform>(enemyId);
    const auto& pt = world.getComponent<Transform>(playerId);
    float ex = et.x + halfW;
    float ey = et.y + halfH;
    float px = pt.x + 12.0f;
    float py = pt.y + 24.0f;
    b2Vec2 from = { PhysicsSystem::toMeters(ex), PhysicsSystem::toMeters(ey) };
    b2Vec2 to   = { PhysicsSystem::toMeters(px), PhysicsSystem::toMeters(py) };
    b2Body* playerBody = world.hasComponent<Collidable>(playerId)
                       ? world.getComponent<Collidable>(playerId).body : nullptr;
    return physics.rayCastClear(from, to, playerBody);
}

// Returns {halfW, halfH} from EnemyType config
static std::pair<float,float> halfExtents(EnemyType t) {
    switch (t) {
        case EnemyType::ENFORCER:     return { EnemyConfig::ENFORCER_WIDTH * 0.5f,  EnemyConfig::ENFORCER_HEIGHT * 0.5f };
        case EnemyType::SHIELD:       return { EnemyConfig::SHIELD_WIDTH * 0.5f,    EnemyConfig::SHIELD_HEIGHT * 0.5f };
        case EnemyType::SNIPER:       return { EnemyConfig::SNIPER_WIDTH * 0.5f,    EnemyConfig::SNIPER_HEIGHT * 0.5f };
        case EnemyType::HACKER:       return { EnemyConfig::HACKER_WIDTH * 0.5f,    EnemyConfig::HACKER_HEIGHT * 0.5f };
        case EnemyType::HEAVY:        return { EnemyConfig::HEAVY_WIDTH * 0.5f,     EnemyConfig::HEAVY_HEIGHT * 0.5f };
        case EnemyType::DRONE:        return { EnemyConfig::DRONE_WIDTH * 0.5f,     EnemyConfig::DRONE_HEIGHT * 0.5f };
        case EnemyType::CYBORG_ELITE: return { EnemyConfig::ELITE_WIDTH * 0.5f,     EnemyConfig::ELITE_HEIGHT * 0.5f };
        default:                       return { EnemyConfig::SCOUT_WIDTH * 0.5f,    EnemyConfig::SCOUT_HEIGHT * 0.5f };
    }
}

static float detectRange(EnemyType t) {
    switch (t) {
        case EnemyType::ENFORCER:     return EnemyConfig::ENFORCER_DETECT_RANGE;
        case EnemyType::SHIELD:       return EnemyConfig::SHIELD_DETECT_RANGE;
        case EnemyType::SNIPER:       return EnemyConfig::SNIPER_DETECT_RANGE;
        case EnemyType::HACKER:       return EnemyConfig::HACKER_DETECT_RANGE;
        case EnemyType::HEAVY:        return EnemyConfig::HEAVY_DETECT_RANGE;
        case EnemyType::DRONE:        return EnemyConfig::DRONE_DETECT_RANGE;
        case EnemyType::CYBORG_ELITE: return EnemyConfig::ELITE_DETECT_RANGE;
        default:                       return EnemyConfig::SCOUT_DETECT_RANGE;
    }
}

static float patrolSpeed(EnemyType t) {
    switch (t) {
        case EnemyType::ENFORCER:     return EnemyConfig::ENFORCER_PATROL_SPEED;
        case EnemyType::SHIELD:       return EnemyConfig::SHIELD_PATROL_SPEED;
        case EnemyType::SNIPER:       return EnemyConfig::SNIPER_PATROL_SPEED;
        case EnemyType::HACKER:       return EnemyConfig::HACKER_PATROL_SPEED;
        case EnemyType::HEAVY:        return EnemyConfig::HEAVY_PATROL_SPEED;
        case EnemyType::DRONE:        return EnemyConfig::DRONE_PATROL_SPEED;
        case EnemyType::CYBORG_ELITE: return EnemyConfig::ELITE_PATROL_SPEED;
        default:                       return EnemyConfig::SCOUT_PATROL_SPEED;
    }
}

// ---- Per-type COMBAT helpers -----------------------------------------------

void AIStateMachine::combatScout(uint32_t, float dt, float ex, float ey,
                                  float px, float py, float playerDist2,
                                  b2Body* body, AIState& ai,
                                  World& world, PhysicsSystem& physics,
                                  const NavMesh& nav, int& budget) {
    if (playerDist2 > EnemyConfig::SCOUT_ATTACK_RANGE * EnemyConfig::SCOUT_ATTACK_RANGE) {
        if (ai.hasLOS) {
            moveToward(body, px, py, ex, ey, EnemyConfig::SCOUT_COMBAT_SPEED);
        } else {
            if (ai.pathDirty && budget > 0) {
                --budget;
                int sn = nav.nearestNode(ex, ey);
                Pathfinding::findPath(nav, sn, ai.lastSeenX, ai.lastSeenY, ai);
                ai.pathDirty = false;
            }
            if (ai.pathCursor < ai.pathLen) {
                float wx = ai.pathX[ai.pathCursor], wy = ai.pathY[ai.pathCursor];
                if (dist2(ex, ey, wx, wy) < EnemyConfig::WAYPOINT_ARRIVE_DIST * EnemyConfig::WAYPOINT_ARRIVE_DIST)
                    ++ai.pathCursor;
                else
                    moveToward(body, wx, wy, ex, ey, EnemyConfig::SCOUT_COMBAT_SPEED);
            }
        }
    } else {
        body->SetLinearVelocity({0.f, body->GetLinearVelocity().y});
        if (ai.hasLOS && ai.attackTimer <= 0.f) {
            ai.attackTimer = EnemyConfig::SCOUT_ATTACK_INTERVAL;
            float dx = px - ex, dy2 = py - ey;
            float len = std::sqrt(dx*dx + dy2*dy2);
            if (len > 0.1f) { dx /= len; dy2 /= len; }
            EventBus::emit(EnemyFireEvent{ 0u, ex, ey, dx, dy2 });
        }
    }
    (void)world; (void)physics; (void)dt;
}

void AIStateMachine::combatEnforcer(uint32_t, float dt, float ex, float ey,
                                     float px, float py, float playerDist2,
                                     b2Body* body, AIState& ai,
                                     World&, PhysicsSystem&,
                                     const NavMesh& nav, int& budget) {
    // Seek cover first, then shoot
    if (!ai.atCover) {
        // Simple: move perpendicular to player direction as "cover"
        if (ai.coverX == 0.f && ai.coverY == 0.f) {
            // Pick a position offset from current toward player but 80px to the side
            float dx = px - ex, dy2 = py - ey;
            float len = std::sqrt(dx*dx + dy2*dy2);
            if (len > 0.1f) { dx /= len; dy2 /= len; }
            ai.coverX = ex - dy2 * 80.f;
            ai.coverY = ey + dx * 80.f;
        }
        float d2 = dist2(ex, ey, ai.coverX, ai.coverY);
        if (d2 > EnemyConfig::WAYPOINT_ARRIVE_DIST * EnemyConfig::WAYPOINT_ARRIVE_DIST)
            moveToward(body, ai.coverX, ai.coverY, ex, ey, EnemyConfig::ENFORCER_COMBAT_SPEED);
        else
            ai.atCover = true;
    } else {
        body->SetLinearVelocity({0.f, body->GetLinearVelocity().y});
        if (ai.hasLOS && ai.attackTimer <= 0.f) {
            ai.attackTimer = EnemyConfig::ENFORCER_ATTACK_INTERVAL;
            float dx = px - ex, dy2 = py - ey;
            float len = std::sqrt(dx*dx + dy2*dy2);
            if (len > 0.1f) { dx /= len; dy2 /= len; }
            EventBus::emit(EnemyFireEvent{ 0u, ex, ey, dx, dy2 });
            // Reset cover occasionally to flank
            if (static_cast<int>(ai.stateTimer * 10) % 50 == 0)
                ai.atCover = false, ai.coverX = 0.f, ai.coverY = 0.f;
        }
        // If out of range, give up cover and chase
        if (playerDist2 > EnemyConfig::ENFORCER_ATTACK_RANGE * EnemyConfig::ENFORCER_ATTACK_RANGE * 4.f)
            ai.atCover = false, ai.coverX = 0.f, ai.coverY = 0.f;
    }
    (void)dt; (void)nav; (void)budget;
}

void AIStateMachine::combatSniper(uint32_t eid, float dt, float ex, float ey,
                                   float px, float py,
                                   b2Body* body, AIState& ai) {
    body->SetLinearVelocity({0.f, body->GetLinearVelocity().y});
    if (ai.hasLOS) {
        ai.aimTimer  += dt;
        ai.laserLock  = true;
        ai.laserEndX  = px;
        ai.laserEndY  = py;
        if (ai.aimTimer >= EnemyConfig::SNIPER_AIM_TIME && ai.attackTimer <= 0.f) {
            ai.attackTimer = EnemyConfig::SNIPER_ATTACK_INTERVAL;
            ai.aimTimer    = 0.f;
            float dx = px - ex, dy2 = py - ey;
            float len = std::sqrt(dx*dx + dy2*dy2);
            if (len > 0.1f) { dx /= len; dy2 /= len; }
            EventBus::emit(EnemyFireEvent{ eid, ex, ey, dx, dy2 });
        }
    } else {
        ai.aimTimer  = 0.f;
        ai.laserLock = false;
    }
    (void)eid;
}

void AIStateMachine::combatHeavy(uint32_t, float dt, float ex, float ey,
                                  float px, float py,
                                  b2Body* body, AIState& ai) {
    // Slow advance, continuous spray
    float d2 = dist2(ex, ey, px, py);
    if (d2 > EnemyConfig::HEAVY_ATTACK_RANGE * EnemyConfig::HEAVY_ATTACK_RANGE)
        moveToward(body, px, py, ex, ey, EnemyConfig::HEAVY_COMBAT_SPEED);
    else
        body->SetLinearVelocity({0.f, body->GetLinearVelocity().y});

    if (ai.hasLOS && ai.attackTimer <= 0.f) {
        ai.attackTimer = EnemyConfig::HEAVY_ATTACK_INTERVAL;
        float dx = px - ex, dy2 = py - ey;
        float len = std::sqrt(dx*dx + dy2*dy2);
        if (len > 0.1f) { dx /= len; dy2 /= len; }
        // Spray: slight random spread
        float jitter = (static_cast<float>(rand() % 100) / 100.f - 0.5f) * EnemyConfig::HEAVY_SPREAD_RAD;
        float sdx = dx * std::cos(jitter) - dy2 * std::sin(jitter);
        float sdy = dx * std::sin(jitter) + dy2 * std::cos(jitter);
        EventBus::emit(EnemyFireEvent{ 0u, ex, ey, sdx, sdy });
    }
    (void)dt;
}

void AIStateMachine::combatDrone(uint32_t, float dt, float ex, float ey,
                                  float px, float py, float playerDist2,
                                  b2Body* body, AIState& ai) {
    // Drone moves in world space (no navmesh), hovers above player
    float targetX = px;
    float targetY = py - EnemyConfig::DRONE_HOVER_HEIGHT;

    float dx = targetX - ex, dy2 = targetY - ey;
    float len = std::sqrt(dx*dx + dy2*dy2);
    if (len > 8.0f) {
        dx /= len; dy2 /= len;
        float spd = PhysicsSystem::toMeters(EnemyConfig::DRONE_COMBAT_SPEED);
        body->SetLinearVelocity({ dx * spd, dy2 * spd });
    } else {
        body->SetLinearVelocity({0.f, 0.f});
    }

    if (playerDist2 < EnemyConfig::DRONE_ATTACK_RANGE * EnemyConfig::DRONE_ATTACK_RANGE
        && ai.attackTimer <= 0.f) {
        ai.attackTimer = EnemyConfig::DRONE_ATTACK_INTERVAL;
        float fdx = px - ex, fdy = py - ey;
        float flen = std::sqrt(fdx*fdx + fdy*fdy);
        if (flen > 0.1f) { fdx /= flen; fdy /= flen; }
        EventBus::emit(EnemyFireEvent{ 0u, ex, ey, fdx, fdy });
    }
    (void)dt;
}

// ---- Main update -----------------------------------------------------------

void AIStateMachine::update(uint32_t       enemyId,
                            float          dt,
                            uint32_t       playerEntityId,
                            World&         world,
                            PhysicsSystem& physics,
                            const NavMesh& navMesh,
                            int&           aStarBudget) {
    if (!world.isAlive(enemyId)) return;
    if (!world.hasComponent<AIState>(enemyId))    return;
    if (!world.hasComponent<Transform>(enemyId))  return;
    if (!world.hasComponent<Collidable>(enemyId)) return;

    auto& ai   = world.getComponent<AIState>(enemyId);
    auto& tf   = world.getComponent<Transform>(enemyId);
    b2Body* body = world.getComponent<Collidable>(enemyId).body;
    if (!body) return;

    EnemyType etype = world.hasComponent<EnemyTag>(enemyId)
                    ? world.getComponent<EnemyTag>(enemyId).type
                    : EnemyType::SCOUT;

    // EMP stun — skip all AI
    if (ai.empDisabled) {
        ai.empTimer -= dt;
        if (ai.empTimer <= 0.f) ai.empDisabled = false;
        body->SetLinearVelocity({0.f, 0.f});
        return;
    }

    auto [halfW, halfH] = halfExtents(etype);
    float ex = tf.x + halfW;
    float ey = tf.y + halfH;

    bool hasPlayer = (playerEntityId != static_cast<uint32_t>(MAX_ENTITIES))
                  && world.isAlive(playerEntityId);

    float px = 0.f, py = 0.f, playerDist2 = 1e9f;
    if (hasPlayer && world.hasComponent<Transform>(playerEntityId)) {
        const auto& pt = world.getComponent<Transform>(playerEntityId);
        px = pt.x + 12.0f;
        py = pt.y + 24.0f;
        playerDist2 = dist2(ex, ey, px, py);
    }

    ai.stateTimer += dt;
    if (ai.attackTimer > 0.f) ai.attackTimer -= dt;

    float dRange  = detectRange(etype);
    float pSpeed  = patrolSpeed(etype);

    // HACKER: emit block if player is in hack mode nearby
    if (etype == EnemyType::HACKER && ai.current == AIStateEnum::COMBAT) {
        float br = EnemyConfig::HACKER_BLOCK_RANGE;
        if (playerDist2 < br * br) {
            ai.hackBlocking = true;
            EventBus::emit(HackBlockedEvent{ enemyId });
        } else {
            ai.hackBlocking = false;
        }
    }

    switch (ai.current) {
    // ── PATROL ──────────────────────────────────────────────────────────────
    case AIStateEnum::PATROL: {
        if (hasPlayer && playerDist2 < dRange * dRange) {
            // Drone has 360° detection (no cone), others need LOS
            bool spotted = (etype == EnemyType::DRONE)
                         ? true
                         : checkLOS(enemyId, playerEntityId, world, physics, halfW, halfH);
            if (spotted) {
                ai.lastSeenX  = px; ai.lastSeenY = py;
                ai.stateTimer = 0.f;
                ai.current    = AIStateEnum::COMBAT;
                ai.hasLOS     = true;
                EventBus::emit(EnemyAlertedEvent{ enemyId, px, py });
                break;
            }
        }
        if (!world.hasComponent<WaypointPath>(enemyId)) break;
        const auto& wp = world.getComponent<WaypointPath>(enemyId);
        if (wp.count == 0) break;
        int  widx = ai.waypointIdx % wp.count;
        float wpx = wp.x[widx], wpy = wp.y[widx];
        if (dist2(ex, ey, wpx, wpy) < EnemyConfig::WAYPOINT_ARRIVE_DIST * EnemyConfig::WAYPOINT_ARRIVE_DIST)
            ai.waypointIdx = (ai.waypointIdx + 1) % wp.count;
        else
            moveToward(body, wpx, wpy, ex, ey, pSpeed);
        break;
    }

    // ── ALERT ───────────────────────────────────────────────────────────────
    case AIStateEnum::ALERT: {
        if (hasPlayer && playerDist2 < dRange * dRange) {
            bool spotted = (etype == EnemyType::DRONE)
                         ? true
                         : checkLOS(enemyId, playerEntityId, world, physics, halfW, halfH);
            if (spotted) {
                ai.lastSeenX  = px; ai.lastSeenY = py;
                ai.stateTimer = 0.f;
                ai.current    = AIStateEnum::COMBAT;
                ai.hasLOS     = true;
                break;
            }
        }
        float d2 = dist2(ex, ey, ai.alertX, ai.alertY);
        if (d2 > EnemyConfig::WAYPOINT_ARRIVE_DIST * EnemyConfig::WAYPOINT_ARRIVE_DIST)
            moveToward(body, ai.alertX, ai.alertY, ex, ey, pSpeed);
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

        // SHIELD/HACKER don't shoot; DRONE doesn't need LOS
        if (etype != EnemyType::DRONE)
            ai.hasLOS = checkLOS(enemyId, playerEntityId, world, physics, halfW, halfH);
        else
            ai.hasLOS = true;

        if (ai.hasLOS) { ai.lastSeenX = px; ai.lastSeenY = py; ai.stateTimer = 0.f; }

        if (!ai.hasLOS && ai.stateTimer >= EnemyConfig::LOST_LOS_TIMEOUT) {
            ai.stateTimer = 0.f; ai.pathDirty = true;
            ai.current = AIStateEnum::SEARCH;
            break;
        }

        // Dispatch per type
        switch (etype) {
        case EnemyType::SCOUT:
            combatScout(enemyId, dt, ex, ey, px, py, playerDist2,
                        body, ai, world, physics, navMesh, aStarBudget);
            break;
        case EnemyType::ENFORCER:
        case EnemyType::CYBORG_ELITE:   // stub — ENFORCER behavior
            combatEnforcer(enemyId, dt, ex, ey, px, py, playerDist2,
                           body, ai, world, physics, navMesh, aStarBudget);
            break;
        case EnemyType::SNIPER:
            combatSniper(enemyId, dt, ex, ey, px, py, body, ai);
            break;
        case EnemyType::HACKER:
            // HACKER just stays back, no shooting
            if (playerDist2 > 200.f * 200.f)
                moveToward(body, ai.lastSeenX, ai.lastSeenY, ex, ey,
                           EnemyConfig::HACKER_COMBAT_SPEED * 0.5f);
            else
                body->SetLinearVelocity({0.f, body->GetLinearVelocity().y});
            break;
        case EnemyType::SHIELD:
            // Advance toward player, shield blocks frontal damage (handled in damage system)
            moveToward(body, px, py, ex, ey, EnemyConfig::SHIELD_COMBAT_SPEED);
            if (playerDist2 < EnemyConfig::SHIELD_ATTACK_RANGE * EnemyConfig::SHIELD_ATTACK_RANGE
                && ai.attackTimer <= 0.f) {
                ai.attackTimer = EnemyConfig::SHIELD_ATTACK_INTERVAL;
                EventBus::emit(PlayerDamagedEvent{
                    playerEntityId, EnemyConfig::SHIELD_ATTACK_DAMAGE });
            }
            break;
        case EnemyType::HEAVY:
            combatHeavy(enemyId, dt, ex, ey, px, py, body, ai);
            break;
        case EnemyType::DRONE:
            combatDrone(enemyId, dt, ex, ey, px, py, playerDist2, body, ai);
            break;
        default: break;
        }
        break;
    }

    // ── SEARCH ──────────────────────────────────────────────────────────────
    case AIStateEnum::SEARCH: {
        if (hasPlayer && playerDist2 < dRange * dRange) {
            bool spotted = (etype == EnemyType::DRONE)
                         ? true
                         : checkLOS(enemyId, playerEntityId, world, physics, halfW, halfH);
            if (spotted) {
                ai.lastSeenX = px; ai.lastSeenY = py;
                ai.stateTimer = 0.f; ai.pathDirty = true;
                ai.current = AIStateEnum::COMBAT;
                break;
            }
        }
        if (etype != EnemyType::DRONE) {
            if (ai.pathDirty && aStarBudget > 0) {
                --aStarBudget;
                int sn = navMesh.nearestNode(ex, ey);
                Pathfinding::findPath(navMesh, sn, ai.lastSeenX, ai.lastSeenY, ai);
                ai.pathDirty = false;
            }
            if (ai.pathCursor < ai.pathLen) {
                float wx = ai.pathX[ai.pathCursor], wy = ai.pathY[ai.pathCursor];
                if (dist2(ex, ey, wx, wy) < EnemyConfig::WAYPOINT_ARRIVE_DIST * EnemyConfig::WAYPOINT_ARRIVE_DIST)
                    ++ai.pathCursor;
                else
                    moveToward(body, wx, wy, ex, ey, pSpeed);
            } else {
                body->SetLinearVelocity({0.f, body->GetLinearVelocity().y});
            }
        }
        if (ai.stateTimer >= EnemyConfig::SEARCH_DURATION) {
            ai.stateTimer = 0.f; ai.waypointIdx = 0;
            ai.current = AIStateEnum::PATROL;
        }
        break;
    }
    }
}
