#pragma once
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

// ALL game component definitions. POD only — no methods, no constructors.

struct Transform {
    float x        = 0.0f;
    float y        = 0.0f;
    float rotation = 0.0f;
    // Previous-frame position for render interpolation
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

struct Renderable {
    sf::RectangleShape shape; // placeholder; will be sprite atlas in Sprint 2
    int  layer   = 0;
    bool visible = true;
};

struct Collidable {
    b2Body* body = nullptr;
    float   w    = 0.0f;
    float   h    = 0.0f;
};

// Marker components — zero size in SoA (bool presence flag in World)
struct PlayerTag {};
struct TileTag   {};
