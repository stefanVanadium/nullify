#pragma once
#include <string>
#include <vector>
#include <optional>
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "ecs/Components.h"
#include "world/TileMap.h"

struct PlayerSpawn {
    float x = 0.f, y = 0.f;
};

struct EnemySpawnData {
    float        x = 0.f, y = 0.f;
    EnemyType    type = EnemyType::SCOUT;
    WaypointPath waypoints;
};

struct CoverObject {
    float x = 0.f, y = 0.f, w = 0.f, h = 0.f;
};

struct HackableSpawnData {
    float x = 0.f, y = 0.f;
    int   tier = 1;
};

struct WeaponPickupData {
    float      x = 0.f, y = 0.f;
    int        weaponTypeInt = 0;
};

struct LevelData {
    PlayerSpawn                    playerSpawn;
    std::vector<EnemySpawnData>    enemies;
    std::vector<CoverObject>       coverObjects;
    std::vector<HackableSpawnData> hackables;
    std::vector<WeaponPickupData>  weaponPickups;
    // Collision data retained for NavMesh building
    std::vector<std::vector<int>>  collision;
    int width    = 0;
    int height   = 0;
    int tileSize = 32;
};

class LevelLoader {
public:
    // Builds TileMap physics bodies + vertex array into `tileMap`, returns rich level data.
    std::optional<LevelData> load(const std::string& path,
                                  World&             world,
                                  PhysicsSystem&     physics,
                                  TileMap&           tileMap);
};
