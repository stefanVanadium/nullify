#pragma once

namespace PlayerConfig {
    constexpr float MOVE_SPEED      = 220.0f;   // pixels/sec
    constexpr float JUMP_IMPULSE    = 520.0f;   // pixels/sec initial y velocity (upward)
    constexpr float MAX_FALL_SPEED  = 800.0f;   // pixels/sec downward (positive = down in screen)
    constexpr float COYOTE_TIME     = 0.12f;    // seconds after leaving edge where jump is still allowed
    constexpr float JUMP_BUFFER     = 0.10f;    // seconds before landing that jump input is remembered
    constexpr float WIDTH           = 24.0f;    // pixels
    constexpr float HEIGHT          = 48.0f;    // pixels
}
