#include "LevelLoader.h"
#include "TileMap.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

std::optional<PlayerSpawn> LevelLoader::load(const std::string& path,
                                              World& world,
                                              PhysicsSystem& physics) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[LevelLoader] Cannot open: " << path << "\n";
        return std::nullopt;
    }

    json j;
    try {
        file >> j;
    } catch (const json::exception& e) {
        std::cerr << "[LevelLoader] JSON parse error: " << e.what() << "\n";
        return std::nullopt;
    }

    const auto& meta = j["meta"];
    TileMapData data;
    data.width    = meta.value("width",    60);
    data.height   = meta.value("height",   20);
    data.tileSize = meta.value("tileSize", 32);

    // Parse collision layer
    if (j.contains("layers") && j["layers"].contains("collision")) {
        const auto& rows = j["layers"]["collision"];
        for (const auto& row : rows) {
            std::vector<int> rowData;
            for (int tile : row)
                rowData.push_back(tile);
            data.collision.push_back(std::move(rowData));
        }
    } else {
        std::cerr << "[LevelLoader] Missing layers.collision in " << path << "\n";
        return std::nullopt;
    }

    TileMap tileMap;
    tileMap.build(data, world, physics);
    physics.setWorld(&world);

    // Parse player spawn
    PlayerSpawn spawn;
    if (j.contains("spawns") && j["spawns"].contains("player")) {
        const auto& sp = j["spawns"]["player"];
        int tileX = sp.value("x", 3);
        int tileY = sp.value("y", 15);
        spawn.x = static_cast<float>(tileX * data.tileSize);
        spawn.y = static_cast<float>(tileY * data.tileSize);
    }

    return spawn;
}
