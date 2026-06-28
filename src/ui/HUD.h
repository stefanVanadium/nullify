#pragma once
#include <SFML/Graphics.hpp>
#include "ecs/Components.h"
#include "player/WeaponSystem.h"

// Draws HP bar, ammo counter, weapon slots, alert level, hack overlay.
class HUD {
public:
    bool init(const std::string& fontPath);

    void render(sf::RenderWindow&   window,
                const Health&       playerHealth,
                const Weapon&       playerWeapon,
                const WeaponSystem& weapons,
                int                 alertLevel);

    void renderHackOverlay(sf::RenderWindow& window, float progress);

    const sf::Font* font() const { return m_fontLoaded ? &m_font : nullptr; }

private:
    sf::Font            m_font;
    bool                m_fontLoaded = false;
    sf::RectangleShape  m_hpBarBg;
    sf::RectangleShape  m_hpBarFill;
    sf::Text            m_ammoText;
    sf::Text            m_alertText;
    sf::Text            m_hackText;
    sf::RectangleShape  m_hackBarBg;
    sf::RectangleShape  m_hackBarFill;

    void renderWeaponSlots(sf::RenderWindow& window, const WeaponSystem& weapons, float W, float H);
};
