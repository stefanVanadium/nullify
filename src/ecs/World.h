#pragma once
#include "Components.h"
#include <cstdint>
#include <array>
#include <bitset>
#include <span>
#include <optional>

static constexpr size_t MAX_ENTITIES = 1024;

enum class ComponentType : uint8_t {
    Transform    = 0,
    Velocity     = 1,
    Health       = 2,
    Renderable   = 3,
    Collidable   = 4,
    PlayerTag    = 5,
    TileTag      = 6,
    EnemyTag     = 7,
    AIState      = 8,
    WaypointPath = 9,
    Weapon       = 10,
    COUNT        = 11
};

class World {
public:
    World();

    uint32_t createEntity();
    void     destroyEntity(uint32_t id);
    bool     isAlive(uint32_t id) const;

    template<typename T> void      addComponent(uint32_t id, T&& comp);
    template<typename T> T&        getComponent(uint32_t id);
    template<typename T> const T&  getComponent(uint32_t id) const;
    template<typename T> bool      hasComponent(uint32_t id) const;

    template<typename T> std::span<T>       getComponents();
    template<typename T> std::span<const T> getComponents() const;

    size_t   entityCount() const { return m_aliveCount; }
    uint32_t findPlayer()  const;

private:
    static constexpr uint8_t CT(ComponentType t) { return static_cast<uint8_t>(t); }

    std::array<bool,        MAX_ENTITIES> m_alive{};
    std::array<std::bitset<static_cast<size_t>(ComponentType::COUNT)>, MAX_ENTITIES> m_mask{};

    std::array<Transform,    MAX_ENTITIES> m_transforms{};
    std::array<Velocity,     MAX_ENTITIES> m_velocities{};
    std::array<Health,       MAX_ENTITIES> m_healths{};
    std::array<Renderable,   MAX_ENTITIES> m_renderables{};
    std::array<Collidable,   MAX_ENTITIES> m_collidables{};
    std::array<PlayerTag,    MAX_ENTITIES> m_playerTags{};
    std::array<TileTag,      MAX_ENTITIES> m_tileTags{};
    std::array<EnemyTag,     MAX_ENTITIES> m_enemyTags{};
    std::array<AIState,      MAX_ENTITIES> m_aiStates{};
    std::array<WaypointPath, MAX_ENTITIES> m_waypointPaths{};
    std::array<Weapon,       MAX_ENTITIES> m_weapons{};

    size_t   m_aliveCount = 0;
    uint32_t m_nextId     = 0;
};

// ---------- addComponent specializations ----------

template<> inline void World::addComponent<Transform>(uint32_t id, Transform&& c)     { m_transforms[id]    = std::move(c); m_mask[id].set(CT(ComponentType::Transform)); }
template<> inline void World::addComponent<Velocity>(uint32_t id, Velocity&& c)       { m_velocities[id]    = std::move(c); m_mask[id].set(CT(ComponentType::Velocity)); }
template<> inline void World::addComponent<Health>(uint32_t id, Health&& c)           { m_healths[id]       = std::move(c); m_mask[id].set(CT(ComponentType::Health)); }
template<> inline void World::addComponent<Renderable>(uint32_t id, Renderable&& c)   { m_renderables[id]   = std::move(c); m_mask[id].set(CT(ComponentType::Renderable)); }
template<> inline void World::addComponent<Collidable>(uint32_t id, Collidable&& c)   { m_collidables[id]   = std::move(c); m_mask[id].set(CT(ComponentType::Collidable)); }
template<> inline void World::addComponent<PlayerTag>(uint32_t id, PlayerTag&& c)     { m_playerTags[id]    = std::move(c); m_mask[id].set(CT(ComponentType::PlayerTag)); }
template<> inline void World::addComponent<TileTag>(uint32_t id, TileTag&& c)         { m_tileTags[id]      = std::move(c); m_mask[id].set(CT(ComponentType::TileTag)); }
template<> inline void World::addComponent<EnemyTag>(uint32_t id, EnemyTag&& c)       { m_enemyTags[id]     = std::move(c); m_mask[id].set(CT(ComponentType::EnemyTag)); }
template<> inline void World::addComponent<AIState>(uint32_t id, AIState&& c)         { m_aiStates[id]      = std::move(c); m_mask[id].set(CT(ComponentType::AIState)); }
template<> inline void World::addComponent<WaypointPath>(uint32_t id, WaypointPath&& c){ m_waypointPaths[id] = std::move(c); m_mask[id].set(CT(ComponentType::WaypointPath)); }
template<> inline void World::addComponent<Weapon>(uint32_t id, Weapon&& c)           { m_weapons[id]       = std::move(c); m_mask[id].set(CT(ComponentType::Weapon)); }

// ---------- getComponent specializations ----------

template<> inline Transform&    World::getComponent<Transform>(uint32_t id)    { return m_transforms[id]; }
template<> inline Velocity&     World::getComponent<Velocity>(uint32_t id)     { return m_velocities[id]; }
template<> inline Health&       World::getComponent<Health>(uint32_t id)       { return m_healths[id]; }
template<> inline Renderable&   World::getComponent<Renderable>(uint32_t id)   { return m_renderables[id]; }
template<> inline Collidable&   World::getComponent<Collidable>(uint32_t id)   { return m_collidables[id]; }
template<> inline PlayerTag&    World::getComponent<PlayerTag>(uint32_t id)    { return m_playerTags[id]; }
template<> inline TileTag&      World::getComponent<TileTag>(uint32_t id)      { return m_tileTags[id]; }
template<> inline EnemyTag&     World::getComponent<EnemyTag>(uint32_t id)     { return m_enemyTags[id]; }
template<> inline AIState&      World::getComponent<AIState>(uint32_t id)      { return m_aiStates[id]; }
template<> inline WaypointPath& World::getComponent<WaypointPath>(uint32_t id) { return m_waypointPaths[id]; }
template<> inline Weapon&       World::getComponent<Weapon>(uint32_t id)       { return m_weapons[id]; }

template<> inline const Transform&    World::getComponent<Transform>(uint32_t id)    const { return m_transforms[id]; }
template<> inline const Velocity&     World::getComponent<Velocity>(uint32_t id)     const { return m_velocities[id]; }
template<> inline const Health&       World::getComponent<Health>(uint32_t id)       const { return m_healths[id]; }
template<> inline const Renderable&   World::getComponent<Renderable>(uint32_t id)   const { return m_renderables[id]; }
template<> inline const Collidable&   World::getComponent<Collidable>(uint32_t id)   const { return m_collidables[id]; }
template<> inline const EnemyTag&     World::getComponent<EnemyTag>(uint32_t id)     const { return m_enemyTags[id]; }
template<> inline const AIState&      World::getComponent<AIState>(uint32_t id)      const { return m_aiStates[id]; }
template<> inline const WaypointPath& World::getComponent<WaypointPath>(uint32_t id) const { return m_waypointPaths[id]; }
template<> inline const Weapon&       World::getComponent<Weapon>(uint32_t id)       const { return m_weapons[id]; }

// ---------- hasComponent specializations ----------

template<> inline bool World::hasComponent<Transform>(uint32_t id)    const { return m_mask[id].test(CT(ComponentType::Transform)); }
template<> inline bool World::hasComponent<Velocity>(uint32_t id)     const { return m_mask[id].test(CT(ComponentType::Velocity)); }
template<> inline bool World::hasComponent<Health>(uint32_t id)       const { return m_mask[id].test(CT(ComponentType::Health)); }
template<> inline bool World::hasComponent<Renderable>(uint32_t id)   const { return m_mask[id].test(CT(ComponentType::Renderable)); }
template<> inline bool World::hasComponent<Collidable>(uint32_t id)   const { return m_mask[id].test(CT(ComponentType::Collidable)); }
template<> inline bool World::hasComponent<PlayerTag>(uint32_t id)    const { return m_mask[id].test(CT(ComponentType::PlayerTag)); }
template<> inline bool World::hasComponent<TileTag>(uint32_t id)      const { return m_mask[id].test(CT(ComponentType::TileTag)); }
template<> inline bool World::hasComponent<EnemyTag>(uint32_t id)     const { return m_mask[id].test(CT(ComponentType::EnemyTag)); }
template<> inline bool World::hasComponent<AIState>(uint32_t id)      const { return m_mask[id].test(CT(ComponentType::AIState)); }
template<> inline bool World::hasComponent<WaypointPath>(uint32_t id) const { return m_mask[id].test(CT(ComponentType::WaypointPath)); }
template<> inline bool World::hasComponent<Weapon>(uint32_t id)       const { return m_mask[id].test(CT(ComponentType::Weapon)); }

// ---------- getComponents specializations ----------

template<> inline std::span<Transform>    World::getComponents<Transform>()    { return {m_transforms.data(),    MAX_ENTITIES}; }
template<> inline std::span<Velocity>     World::getComponents<Velocity>()     { return {m_velocities.data(),    MAX_ENTITIES}; }
template<> inline std::span<Health>       World::getComponents<Health>()       { return {m_healths.data(),       MAX_ENTITIES}; }
template<> inline std::span<Renderable>   World::getComponents<Renderable>()   { return {m_renderables.data(),   MAX_ENTITIES}; }
template<> inline std::span<Collidable>   World::getComponents<Collidable>()   { return {m_collidables.data(),   MAX_ENTITIES}; }

template<> inline std::span<const Transform>    World::getComponents<Transform>()    const { return {m_transforms.data(),    MAX_ENTITIES}; }
template<> inline std::span<const Velocity>     World::getComponents<Velocity>()     const { return {m_velocities.data(),    MAX_ENTITIES}; }
template<> inline std::span<const Health>        World::getComponents<Health>()       const { return {m_healths.data(),       MAX_ENTITIES}; }
template<> inline std::span<const Renderable>   World::getComponents<Renderable>()   const { return {m_renderables.data(),   MAX_ENTITIES}; }
template<> inline std::span<const Collidable>   World::getComponents<Collidable>()   const { return {m_collidables.data(),   MAX_ENTITIES}; }
