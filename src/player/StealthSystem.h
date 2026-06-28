#pragma once
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "core/EventBus.h"
#include "core/InputMap.h"
#include <SFML/Graphics.hpp>

// Handles cone-of-vision detection, hearing, corpse spotting, and silent takedowns.
// LOS raycasts are distributed round-robin across enemies to stay within budget (8/frame).
class StealthSystem {
public:
    StealthSystem(World& world, PhysicsSystem& physics);
    ~StealthSystem();

    void update(float dt, uint32_t playerEntityId, const InputMap& input);
    void renderCones(sf::RenderTarget& rt) const;

private:
    World&         m_world;
    PhysicsSystem& m_physics;

    int              m_losRoundRobin = 0;  // which enemy index gets a LOS check this frame
    EventBus::Handle m_soundHandle   = 0;

    void onSoundEmitted(const SoundEmittedEvent& e);
    void checkTakedown(uint32_t playerId, const InputMap& input);
    void checkCorpseDetection();
};
