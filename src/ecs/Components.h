#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

// ALL game component definitions. POD only — no methods, no constructors.

struct Transform {
    float x        = 0.0f;
    float y        = 0.0f;
    float rotation = 0.0f;
    float prevX    = 0.0f;
    float prevY    = 0.0f;
};

struct Velocity {
    float vx = 0.0f;
    float vy = 0.0f;
};

struct Health {
    int current = 100;
    int max     = 100;
};

// Replaced sf::RectangleShape with plain data — SpriteBatch renders from these fields
struct Renderable {
    sf::Vector2f size;
    sf::Color    color{0xFF, 0xFF, 0xFF, 0xFF};
    int          layer   = 0;
    bool         visible = true;
};

struct Collidable {
    b2Body* body = nullptr;
    float   w    = 0.0f;
    float   h    = 0.0f;
};

struct Weapon {
    int   ammo      = 30;
    int   maxAmmo   = 30;
    float cooldown  = 0.0f;    // remaining cooldown until next shot
    float facingDir = 1.0f;    // 1=right, -1=left
};

// ---- Enemy types ----

enum class EnemyType : uint8_t { SCOUT = 0 };
enum class AIStateEnum : uint8_t { PATROL = 0, ALERT = 1, COMBAT = 2, SEARCH = 3 };

struct EnemyTag {
    EnemyType type = EnemyType::SCOUT;
};

struct AIState {
    AIStateEnum current    = AIStateEnum::PATROL;
    float stateTimer       = 0.0f;
    int   waypointIdx      = 0;    // current waypoint in PATROL
    float alertX           = 0.0f; // sound source for ALERT
    float alertY           = 0.0f;
    float targetX          = 0.0f; // current movement target
    float targetY          = 0.0f;
    float lastSeenX        = 0.0f; // last known player position
    float lastSeenY        = 0.0f;
    float attackTimer      = 0.0f;
    bool  hasLOS           = false;
    bool  pathDirty        = true;
    // Cached A* path (fixed-size to avoid heap allocation)
    static constexpr int MAX_PATH = 32;
    float pathX[MAX_PATH]{};
    float pathY[MAX_PATH]{};
    int   pathLen    = 0;
    int   pathCursor = 0;
};

struct WaypointPath {
    static constexpr int MAX_WP = 8;
    float x[MAX_WP]{};
    float y[MAX_WP]{};
    int   count = 0;
};

// Marker components
struct PlayerTag {};
struct TileTag   {};
