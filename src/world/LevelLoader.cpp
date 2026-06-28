#include "LevelLoader.h"
#include "TileMap.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

std::optional<LevelData> LevelLoader::load(const std::string& path,
                                            World&             world,
                                            PhysicsSystem&     physics,
                                            TileMap&           tileMap) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[LevelLoader] Cannot open: " << path << "\n";
        return std::nullopt;
    }

    json j;
    try { file >> j; }
    catch (const json::exception& e) {
        std::cerr << "[LevelLoader] JSON parse error: " << e.what() << "\n";
        return std::nullopt;
    }

    LevelData data;
    const auto& meta = j["meta"];
    data.width    = meta.value("width",    60);
    data.height   = meta.value("height",   20);
    data.tileSize = meta.value("tileSize", 32);

    // ── Collision layer ──────────────────────────────────────────────────────
    if (!j.contains("layers") || !j["layers"].contains("collision")) {
        std::cerr << "[LevelLoader] Missing layers.collision in " << path << "\n";
        return std::nullopt;
    }
    for (const auto& row : j["layers"]["collision"]) {
        std::vector<int> rowData;
        for (int tile : row) rowData.push_back(tile);
        data.collision.push_back(std::move(rowData));
    }

    // Build tile geometry into caller-owned tileMap
    TileMapData tmd;
    tmd.width     = data.width;
    tmd.height    = data.height;
    tmd.tileSize  = data.tileSize;
    tmd.collision = data.collision;

    tileMap.build(tmd, physics);
    physics.setWorld(&world);

    // ── Player spawn ─────────────────────────────────────────────────────────
    if (j.contains("spawns") && j["spawns"].contains("player")) {
        const auto& sp = j["spawns"]["player"];
        int tx = sp.value("x", 3);
        int ty = sp.value("y", 17);
        data.playerSpawn.x = static_cast<float>(tx * data.tileSize);
        data.playerSpawn.y = static_cast<float>(ty * data.tileSize);
    }

    // ── Enemy spawns ─────────────────────────────────────────────────────────
    if (j.contains("spawns") && j["spawns"].contains("enemies")) {
        for (const auto& e : j["spawns"]["enemies"]) {
            EnemySpawnData es;
            es.x = static_cast<float>(e.value("x", 0) * data.tileSize);
            es.y = static_cast<float>(e.value("y", 0) * data.tileSize);
            // type: only SCOUT for now
            es.type = EnemyType::SCOUT;

            if (e.contains("waypoints")) {
                int wIdx = 0;
                for (const auto& wp : e["waypoints"]) {
                    if (wIdx >= WaypointPath::MAX_WP) break;
                    es.waypoints.x[wIdx] = static_cast<float>(wp.value("x", 0) * data.tileSize);
                    es.waypoints.y[wIdx] = static_cast<float>(wp.value("y", 0) * data.tileSize);
                    ++wIdx;
                }
                es.waypoints.count = wIdx;
            }
            data.enemies.push_back(std::move(es));
        }
    }

    // ── Cover objects ─────────────────────────────────────────────────────────
    if (j.contains("coverObjects")) {
        for (const auto& co : j["coverObjects"]) {
            CoverObject obj;
            obj.x = static_cast<float>(co.value("x", 0) * data.tileSize);
            obj.y = static_cast<float>(co.value("y", 0) * data.tileSize);
            obj.w = static_cast<float>(co.value("w", 1) * data.tileSize);
            obj.h = static_cast<float>(co.value("h", 1) * data.tileSize);
            physics.createStaticBody(obj.x, obj.y, obj.w, obj.h);

            // ECS entity for rendering — static, no Collidable needed
            uint32_t eid = world.createEntity();
            Transform t{}; t.x = obj.x; t.y = obj.y; t.prevX = obj.x; t.prevY = obj.y;
            world.addComponent<Transform>(eid, std::move(t));
            Renderable r{};
            r.size  = {obj.w, obj.h};
            r.color = sf::Color(0x1A, 0x28, 0x40, 0xFF);
            r.layer = 5;
            world.addComponent<Renderable>(eid, std::move(r));

            data.coverObjects.push_back(obj);
        }
    }

    return data;
}
