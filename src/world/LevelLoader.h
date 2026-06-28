#pragma once
#include <string>
#include <optional>
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"

struct PlayerSpawn {
    float x = 0.0f;
    float y = 0.0f;
};

class LevelLoader {
public:
    // Returns spawn position on success, nullopt on failure (errors logged to stderr)
    std::optional<PlayerSpawn> load(const std::string& path,
                                    World& world,
                                    PhysicsSystem& physics);
};
