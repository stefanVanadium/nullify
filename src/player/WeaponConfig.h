#pragma once
#include <cstdint>

enum class WeaponType : uint8_t {
    PHANTOM9     = 0,
    STATIC_SMG   = 1,
    RAILGUN      = 2,
    VOID_SHOTGUN = 3,
    EMP_GRENADE  = 4,
    NEURAL_SPIKE = 5,
    COUNT        = 6
};

namespace WeaponConfig {

    // ---- PHANTOM-9 (silenced pistol, burst/single toggle) ----
    constexpr float P9_BULLET_SPEED   = 800.0f;
    constexpr int   P9_DAMAGE         = 25;
    constexpr float P9_MAX_DIST       = 600.0f;
    constexpr float P9_FIRE_COOLDOWN  = 0.12f;
    constexpr int   P9_MAG_SIZE       = 30;
    constexpr float P9_SOUND_RADIUS   = 0.0f;    // silenced
    constexpr float P9_BULLET_W       = 6.0f;
    constexpr float P9_BULLET_H       = 3.0f;

    // ---- STATIC SMG (electroplasmatic, high ROF, spread) ----
    constexpr float SMG_BULLET_SPEED  = 700.0f;
    constexpr int   SMG_DAMAGE        = 12;
    constexpr float SMG_MAX_DIST      = 500.0f;
    constexpr float SMG_FIRE_COOLDOWN = 0.10f;   // 600rpm
    constexpr int   SMG_MAG_SIZE      = 45;
    constexpr float SMG_SPREAD_RAD    = 0.14f;   // ~8°
    constexpr float SMG_SOUND_RADIUS  = 400.0f;
    constexpr float SMG_BULLET_W      = 5.0f;
    constexpr float SMG_BULLET_H      = 3.0f;

    // ---- RAILGUN MK2 (penetrates walls when hack charge active) ----
    constexpr float RAIL_BULLET_SPEED  = 1200.0f;
    constexpr int   RAIL_DAMAGE        = 80;
    constexpr float RAIL_MAX_DIST      = 900.0f;
    constexpr float RAIL_FIRE_COOLDOWN = 2.0f;
    constexpr int   RAIL_MAG_SIZE      = 8;
    constexpr float RAIL_SOUND_RADIUS  = 500.0f;
    constexpr float RAIL_BULLET_W      = 12.0f;
    constexpr float RAIL_BULLET_H      = 3.0f;

    // ---- VOID SHOTGUN (8 pellets, knockback, short range) ----
    constexpr float VOID_BULLET_SPEED  = 600.0f;
    constexpr int   VOID_DAMAGE        = 18;     // per pellet
    constexpr float VOID_MAX_DIST      = 250.0f;
    constexpr float VOID_FIRE_COOLDOWN = 0.7f;
    constexpr int   VOID_MAG_SIZE      = 16;
    constexpr int   VOID_PELLETS       = 8;
    constexpr float VOID_SPREAD_RAD    = 0.52f;  // ~30°
    constexpr float VOID_KNOCKBACK     = 400.0f; // px/s applied to hit enemies
    constexpr float VOID_SOUND_RADIUS  = 450.0f;
    constexpr float VOID_BULLET_W      = 5.0f;
    constexpr float VOID_BULLET_H      = 5.0f;

    // ---- EMP GRENADE (arc trajectory, AoE disable electronics) ----
    constexpr float EMP_THROW_SPEED    = 400.0f;
    constexpr float EMP_GRAVITY        = 800.0f;  // px/s² downward
    constexpr float EMP_RADIUS         = 180.0f;
    constexpr float EMP_DISABLE_DUR    = 5.0f;
    constexpr float EMP_FIRE_COOLDOWN  = 1.2f;
    constexpr int   EMP_MAG_SIZE       = 4;
    constexpr float EMP_SOUND_RADIUS   = 350.0f;
    constexpr float EMP_BULLET_W       = 10.0f;
    constexpr float EMP_BULLET_H       = 10.0f;

    // ---- NEURAL SPIKE (slow projectile, hacks enemy on hit) ----
    constexpr float SPIKE_BULLET_SPEED  = 250.0f;
    constexpr int   SPIKE_DAMAGE        = 5;
    constexpr float SPIKE_MAX_DIST      = 600.0f;
    constexpr float SPIKE_FIRE_COOLDOWN = 1.5f;
    constexpr int   SPIKE_MAG_SIZE      = 6;
    constexpr float SPIKE_CONTROL_DUR   = 3.0f;
    constexpr float SPIKE_SOUND_RADIUS  = 100.0f;
    constexpr float SPIKE_BULLET_W      = 6.0f;
    constexpr float SPIKE_BULLET_H      = 6.0f;

    // Legacy aliases — keeps existing code compiling
    constexpr float BULLET_SPEED    = P9_BULLET_SPEED;
    constexpr int   BULLET_DAMAGE   = P9_DAMAGE;
    constexpr float BULLET_MAX_DIST = P9_MAX_DIST;
    constexpr float FIRE_COOLDOWN   = P9_FIRE_COOLDOWN;
    constexpr int   MAG_SIZE        = P9_MAG_SIZE;
    constexpr float BULLET_W        = P9_BULLET_W;
    constexpr float BULLET_H        = P9_BULLET_H;
}
