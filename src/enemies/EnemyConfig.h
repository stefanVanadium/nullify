#pragma once

namespace EnemyConfig {

    // ---- SCOUT ----
    constexpr int   SCOUT_HP              = 50;
    constexpr float SCOUT_WIDTH           = 24.0f;
    constexpr float SCOUT_HEIGHT          = 48.0f;
    constexpr float SCOUT_PATROL_SPEED    = 80.0f;
    constexpr float SCOUT_COMBAT_SPEED    = 140.0f;
    constexpr float SCOUT_DETECT_RANGE    = 400.0f;
    constexpr float SCOUT_ATTACK_RANGE    = 250.0f;
    constexpr float SCOUT_ATTACK_INTERVAL = 1.2f;
    constexpr int   SCOUT_ATTACK_DAMAGE   = 10;
    constexpr float SCOUT_BULLET_SPEED    = 300.0f;
    constexpr float SCOUT_BULLET_MAX_DIST = 600.0f;
    constexpr float SCOUT_BULLET_W        = 6.0f;
    constexpr float SCOUT_BULLET_H        = 3.0f;

    // ---- ENFORCER ----
    constexpr int   ENFORCER_HP              = 80;
    constexpr float ENFORCER_WIDTH           = 28.0f;
    constexpr float ENFORCER_HEIGHT          = 50.0f;
    constexpr float ENFORCER_PATROL_SPEED    = 75.0f;
    constexpr float ENFORCER_COMBAT_SPEED    = 130.0f;
    constexpr float ENFORCER_DETECT_RANGE    = 380.0f;
    constexpr float ENFORCER_ATTACK_RANGE    = 280.0f;
    constexpr float ENFORCER_ATTACK_INTERVAL = 1.0f;
    constexpr int   ENFORCER_ATTACK_DAMAGE   = 12;
    constexpr float ENFORCER_ARMOR_FRONT     = 0.5f;   // damage multiplier from front

    // ---- SHIELD ----
    constexpr int   SHIELD_HP              = 100;
    constexpr float SHIELD_WIDTH           = 32.0f;
    constexpr float SHIELD_HEIGHT          = 50.0f;
    constexpr float SHIELD_PATROL_SPEED    = 60.0f;
    constexpr float SHIELD_COMBAT_SPEED    = 90.0f;
    constexpr float SHIELD_DETECT_RANGE    = 350.0f;
    constexpr float SHIELD_ATTACK_RANGE    = 150.0f;   // melee-ish range
    constexpr float SHIELD_ATTACK_INTERVAL = 1.5f;
    constexpr int   SHIELD_ATTACK_DAMAGE   = 15;
    constexpr float SHIELD_FRONT_DMG_MUL   = 0.0f;    // immune from front
    constexpr float SHIELD_SIDE_DMG_MUL    = 0.5f;    // reduced from sides

    // ---- SNIPER ----
    constexpr int   SNIPER_HP              = 40;
    constexpr float SNIPER_WIDTH           = 22.0f;
    constexpr float SNIPER_HEIGHT          = 46.0f;
    constexpr float SNIPER_PATROL_SPEED    = 50.0f;
    constexpr float SNIPER_COMBAT_SPEED    = 70.0f;
    constexpr float SNIPER_DETECT_RANGE    = 700.0f;   // long vision
    constexpr float SNIPER_ATTACK_RANGE    = 650.0f;
    constexpr float SNIPER_AIM_TIME        = 1.5f;     // seconds to aim before firing
    constexpr float SNIPER_ATTACK_INTERVAL = 3.0f;     // re-aim after shot
    constexpr int   SNIPER_ATTACK_DAMAGE   = 60;
    constexpr float SNIPER_BULLET_SPEED    = 900.0f;

    // ---- HACKER ----
    constexpr int   HACKER_HP             = 50;
    constexpr float HACKER_WIDTH          = 20.0f;
    constexpr float HACKER_HEIGHT         = 44.0f;
    constexpr float HACKER_PATROL_SPEED   = 70.0f;
    constexpr float HACKER_COMBAT_SPEED   = 100.0f;
    constexpr float HACKER_DETECT_RANGE   = 350.0f;
    constexpr float HACKER_BLOCK_RANGE    = 200.0f;   // hack override block radius

    // ---- HEAVY ----
    constexpr int   HEAVY_HP              = 200;
    constexpr float HEAVY_WIDTH           = 44.0f;
    constexpr float HEAVY_HEIGHT          = 56.0f;
    constexpr float HEAVY_PATROL_SPEED    = 50.0f;   // slow
    constexpr float HEAVY_COMBAT_SPEED    = 80.0f;
    constexpr float HEAVY_DETECT_RANGE    = 350.0f;
    constexpr float HEAVY_ATTACK_RANGE    = 400.0f;
    constexpr float HEAVY_ATTACK_INTERVAL = 0.075f;  // 800rpm spray
    constexpr int   HEAVY_ATTACK_DAMAGE   = 8;
    constexpr float HEAVY_SPREAD_RAD      = 0.26f;   // ~15°

    // ---- DRONE ----
    constexpr int   DRONE_HP              = 20;
    constexpr float DRONE_WIDTH           = 32.0f;
    constexpr float DRONE_HEIGHT          = 20.0f;
    constexpr float DRONE_PATROL_SPEED    = 100.0f;
    constexpr float DRONE_COMBAT_SPEED    = 160.0f;
    constexpr float DRONE_DETECT_RANGE    = 500.0f;  // 360° detection
    constexpr float DRONE_ATTACK_RANGE    = 300.0f;
    constexpr float DRONE_ATTACK_INTERVAL = 1.4f;
    constexpr int   DRONE_ATTACK_DAMAGE   = 8;
    constexpr float DRONE_HOVER_HEIGHT    = 120.0f;  // target Y offset above spawn

    // ---- CYBORG ELITE (boss stub) ----
    constexpr int   ELITE_HP              = 350;
    constexpr float ELITE_WIDTH           = 40.0f;
    constexpr float ELITE_HEIGHT          = 60.0f;
    constexpr float ELITE_PATROL_SPEED    = 70.0f;
    constexpr float ELITE_COMBAT_SPEED    = 150.0f;
    constexpr float ELITE_DETECT_RANGE    = 500.0f;
    constexpr float ELITE_ATTACK_RANGE    = 300.0f;
    constexpr float ELITE_ATTACK_INTERVAL = 0.8f;
    constexpr int   ELITE_ATTACK_DAMAGE   = 20;

    // ---- State durations (shared) ----
    constexpr float ALERT_DURATION       = 5.0f;
    constexpr float SEARCH_DURATION      = 15.0f;
    constexpr float LOST_LOS_TIMEOUT     = 2.0f;
    constexpr float ALERT_HEAR_RANGE     = 320.0f;
    constexpr float WAYPOINT_ARRIVE_DIST = 16.0f;

    // ---- Pool sizes ----
    constexpr size_t MAX_ENEMY_BULLETS   = 256;
    constexpr int    MAX_ENEMIES         = 32;
}
