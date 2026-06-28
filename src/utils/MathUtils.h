#pragma once
#include <cmath>
#include <algorithm>

struct Vec2 {
    float x = 0.0f, y = 0.0f;

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s)       const { return {x * s,   y * s  }; }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }

    float length()  const { return std::sqrt(x * x + y * y); }
    Vec2  normalized() const {
        float l = length();
        return l > 0.0f ? Vec2{x / l, y / l} : Vec2{};
    }
    float dot(const Vec2& o) const { return x * o.x + y * o.y; }
};

struct AABB {
    float x, y, w, h;
    bool overlaps(const AABB& o) const {
        return x < o.x + o.w && x + w > o.x &&
               y < o.y + o.h && y + h > o.y;
    }
    bool contains(float px, float py) const {
        return px >= x && px <= x + w && py >= y && py <= y + h;
    }
};

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline Vec2 lerp(const Vec2& a, const Vec2& b, float t) {
    return {lerp(a.x, b.x, t), lerp(a.y, b.y, t)};
}

inline float clamp(float v, float lo, float hi) {
    return std::clamp(v, lo, hi);
}

inline float lerpAngle(float a, float b, float t) {
    float diff = b - a;
    // Wrap to [-π, π]
    while (diff >  3.14159265f) diff -= 6.28318530f;
    while (diff < -3.14159265f) diff += 6.28318530f;
    return a + diff * t;
}
