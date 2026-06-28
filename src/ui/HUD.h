#pragma once
#include <SFML/Graphics.hpp>
#include "ecs/Components.h"

// Draws HP bar, ammo counter, alert level, and hack overlay in screen space.
class HUD {
public:
    // Returns true if font loaded; HUD still renders shapes without it.
    bool init(const std::string& fontPath);

    void render(sf::RenderWindow& window,
                const Health&     playerHealth,
                const Weapon&     playerWeapon,
                int               alertLevel);

    void renderHackOverlay(sf::RenderWindow& window, float progress);

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
};
