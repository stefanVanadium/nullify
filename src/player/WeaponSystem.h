#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <cstdint>
#include "ecs/World.h"
#include "ecs/Systems/PhysicsSystem.h"
#include "rendering/SpriteBatch.h"
#include "WeaponConfig.h"

enum class BulletType : uint8_t {
    STANDARD,
    EMP,
    NEURAL_SPIKE
};

struct BulletState {
    float      x, y;
    float      dirX, dirY;
    float      speed;
    float      travelDist;
    float      maxDist;
    float      vyGravity;   // vertical velocity for parabolic (EMP), px/s
    BulletType type;
    bool       penetrates;  // railgun through-wall mode
    bool       active;
};

struct WeaponSlot {
    WeaponType type      = WeaponType::PHANTOM9;
    int        ammo      = 0;
    float      cooldown  = 0.0f;
    bool       unlocked  = false;
};

class WeaponSystem {
public:
    static constexpr size_t MAX_BULLETS    = 1024;
    static constexpr int    WEAPON_SLOTS   = static_cast<int>(WeaponType::COUNT);

    WeaponSystem(World& world, PhysicsSystem& physics);

    // Fire active weapon from worldPos toward targetPos
    void fire(sf::Vector2f from, sf::Vector2f target, uint32_t playerEntity);

    // Advance bullets, raycast collision, emit events
    void update(float dt);

    // Batch all active bullet quads into SpriteBatch
    void batchDraw(SpriteBatch& batch) const;

    // Cycle weapon slot (delta = +1 or -1)
    void switchWeapon(int delta);

    // Unlock a weapon (called by pickup system)
    void unlockWeapon(WeaponType wt);

    WeaponType  activeWeapon()    const { return m_slots[m_activeSlot].type; }
    int         activeSlot()      const { return m_activeSlot; }
    int         ammo(int slot)    const { return m_slots[slot].ammo; }
    bool        isUnlocked(int s) const { return m_slots[s].unlocked; }
    int         activeBullets()   const;

private:
    World&         m_world;
    PhysicsSystem& m_physics;

    std::array<BulletState, MAX_BULLETS> m_bullets{};
    size_t m_nextSlot = 0;

    std::array<WeaponSlot, WEAPON_SLOTS> m_slots{};
    int m_activeSlot = 0;

    void fireBullet(float x, float y, float dx, float dy,
                    float speed, float maxDist,
                    BulletType type = BulletType::STANDARD,
                    float vyGravity = 0.0f,
                    bool  penetrates = false);

    // Returns damage dealt if hit, 0 otherwise. Also applies knockback for VOID.
    int  tryHitEnemy(float nx, float ny, WeaponType wt);

    sf::Color bulletColor(BulletType t, WeaponType wt) const;
};
