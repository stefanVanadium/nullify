#pragma once

namespace EnemyConfig {
    // SCOUT stats
    constexpr int   SCOUT_HP             = 50;
    constexpr float SCOUT_WIDTH          = 24.0f;  // pixels
    constexpr float SCOUT_HEIGHT         = 48.0f;  // pixels
    constexpr float SCOUT_PATROL_SPEED   = 80.0f;  // pixels/sec
    constexpr float SCOUT_COMBAT_SPEED   = 140.0f; // pixels/sec
    constexpr float SCOUT_DETECT_RANGE   = 400.0f; // pixels — vision distance
    constexpr float SCOUT_ATTACK_RANGE   = 250.0f; // pixels — can shoot
    constexpr float SCOUT_ATTACK_INTERVAL = 1.2f;  // seconds between shots
    constexpr int   SCOUT_ATTACK_DAMAGE  = 10;

    // State durations
    constexpr float ALERT_DURATION       = 5.0f;   // seconds in ALERT before back to PATROL
    constexpr float SEARCH_DURATION      = 15.0f;  // seconds in SEARCH before back to PATROL
    constexpr float LOST_LOS_TIMEOUT     = 2.0f;   // seconds without LOS → SEARCH

    // Alert hear range (sound events)
    constexpr float ALERT_HEAR_RANGE     = 320.0f; // pixels

    // Waypoint arrival threshold
    constexpr float WAYPOINT_ARRIVE_DIST = 16.0f;  // pixels

    // Max enemies in the world at once
    constexpr int   MAX_ENEMIES          = 32;
}
