#pragma once

namespace PlayerConfig {
    constexpr float MOVE_SPEED        = 220.0f;   // pixels/sec
    constexpr float JUMP_IMPULSE      = 520.0f;   // pixels/sec upward
    constexpr float MAX_FALL_SPEED    = 800.0f;   // pixels/sec downward
    constexpr float COYOTE_TIME       = 0.12f;    // seconds
    constexpr float JUMP_BUFFER       = 0.10f;    // seconds
    constexpr float WIDTH             = 24.0f;    // pixels
    constexpr float HEIGHT            = 48.0f;    // pixels

    // Dash
    constexpr float DASH_SPEED        = 600.0f;   // pixels/sec during dash
    constexpr float DASH_DURATION     = 0.18f;    // seconds
    constexpr float DASH_COOLDOWN     = 0.80f;    // seconds before next dash
    constexpr float DASH_IFRAME       = 0.12f;    // seconds of invincibility

    // Slide
    constexpr float SLIDE_SPEED_MULT  = 1.6f;     // velocity multiplier on slide start
    constexpr float SLIDE_DURATION    = 0.35f;    // seconds
}
