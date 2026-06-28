#pragma once

namespace WeaponConfig {
    // PHANTOM-9 pistol
    constexpr float BULLET_SPEED      = 800.0f;  // pixels/sec
    constexpr int   BULLET_DAMAGE     = 25;
    constexpr float BULLET_MAX_DIST   = 600.0f;  // pixels
    constexpr int   BULLET_PENETRATION = 0;

    constexpr float FIRE_COOLDOWN     = 0.12f;   // seconds between shots
    constexpr int   MAG_SIZE          = 30;

    // Bullet visual (colored quad)
    constexpr float BULLET_W          = 6.0f;
    constexpr float BULLET_H          = 3.0f;
}
