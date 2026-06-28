#include "LevelLoader.h"
#include "TileMap.h"
#include "ecs/Components.h"
#include "player/WeaponConfig.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <string_view>

using json = nlohmann::json;

static EnemyType parseEnemyType(std::string_view s) {
    if (s == "ENFORCER")     return EnemyType::ENFORCER;
    if (s == "SHIELD")       return EnemyType::SHIELD;
    if (s == "SNIPER")       return EnemyType::SNIPER;
    if (s == "HACKER")       return EnemyType::HACKER;
    if (s == "HEAVY")        return EnemyType::HEAVY;
    if (s == "DRONE")        return EnemyType::DRONE;
    if (s == "CYBORG_ELITE") return EnemyType::CYBORG_ELITE;
    return EnemyType::SCOUT; // default
}

static int parseWeaponType(std::string_view s) {
    if (s == "STATIC_SMG")   return static_cast<int>(WeaponType::STATIC_SMG);
    if (s == "RAILGUN")      return static_cast<int>(WeaponType::RAILGUN);
    if (s == "VOID_SHOTGUN") return static_cast<int>(WeaponType::VOID_SHOTGUN);
    if (s == "EMP_GRENADE")  return static_cast<int>(WeaponType::EMP_GRENADE);
    if (s == "NEURAL_SPIKE") return static_cast<int>(WeaponType::NEURAL_SPIKE);
    return static_cast<int>(WeaponType::PHANTOM9);
}

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
            es.type = parseEnemyType(e.value("type", "SCOUT"));

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
            r.color = sf::Color(0x0E, 0x2A, 0x44, 0xFF);  // visible mid-blue, distinct from bg
            r.layer = 5;
            world.addComponent<Renderable>(eid, std::move(r));

            data.coverObjects.push_back(obj);
        }
    }

    // ── Hackable terminals ───────────────────────────────────────────────────
    if (j.contains("hackables")) {
        for (const auto& h : j["hackables"]) {
            float hx = static_cast<float>(h.value("x", 0) * data.tileSize);
            float hy = static_cast<float>(h.value("y", 0) * data.tileSize);

            uint32_t eid = world.createEntity();
            Transform t{}; t.x = hx; t.y = hy; t.prevX = hx; t.prevY = hy;
            world.addComponent<Transform>(eid, std::move(t));
            Renderable r{};
            r.size  = {16.f, 24.f};
            r.color = sf::Color(0xAA, 0x00, 0xFF, 0xFF);  // hack violet
            r.layer = 8;
            world.addComponent<Renderable>(eid, std::move(r));
            int tier = h.value("tier", 1);
            world.addComponent<HackableTag>(eid, HackableTag{ false, tier });

            data.hackables.push_back({ hx, hy, tier });
        }
    }

    // ── Weapon pickups ───────────────────────────────────────────────────────
    if (j.contains("spawns") && j["spawns"].contains("items")) {
        for (const auto& item : j["spawns"]["items"]) {
            if (item.value("itemType", "") != "weapon") continue;
            float wx = static_cast<float>(item.value("x", 0) * data.tileSize);
            float wy = static_cast<float>(item.value("y", 0) * data.tileSize);
            int   wt = parseWeaponType(item.value("weaponType", "PHANTOM9"));

            uint32_t eid = world.createEntity();
            Transform t{}; t.x = wx; t.y = wy; t.prevX = wx; t.prevY = wy;
            world.addComponent<Transform>(eid, std::move(t));
            Renderable r{};
            r.size  = {12.f, 8.f};
            r.color = sf::Color(0xFF, 0xE6, 0x00, 0xFF);  // neon yellow — pickup
            r.layer = 7;
            world.addComponent<Renderable>(eid, std::move(r));
            world.addComponent<PickupTag>(eid, PickupTag{ wt });

            data.weaponPickups.push_back({ wx, wy, wt });
        }
    }

    return data;
}
