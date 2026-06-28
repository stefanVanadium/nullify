#include "HUD.h"
#include <algorithm>
#include <cstdio>

// Color palette constants
static const sf::Color CYAN    = sf::Color(0x00, 0xFF, 0xEE, 0xFF);
static const sf::Color MAGENTA = sf::Color(0xFF, 0x00, 0x6B, 0xFF);
static const sf::Color UI_BASE = sf::Color(0x0A, 0x10, 0x20, 0xCC);
static const sf::Color TEXT_PRI= sf::Color(0xE0, 0xEE, 0xF8, 0xFF);
static const sf::Color DANGER  = sf::Color(0xFF, 0x00, 0x38, 0xFF);
static const sf::Color YELLOW  = sf::Color(0xFF, 0xE6, 0x00, 0xFF);

bool HUD::init(const std::string& fontPath) {
    m_fontLoaded = m_font.loadFromFile(fontPath);

    // HP bar background
    m_hpBarBg.setSize({204.f, 14.f});
    m_hpBarBg.setPosition(16.f, 0.f); // y set per-render to anchor to bottom
    m_hpBarBg.setFillColor(UI_BASE);
    m_hpBarBg.setOutlineColor(sf::Color(0x1A, 0x28, 0x40, 0xFF));
    m_hpBarBg.setOutlineThickness(1.f);

    // HP bar fill — width set per-render
    m_hpBarFill.setSize({200.f, 10.f});
    m_hpBarFill.setPosition(18.f, 0.f);
    m_hpBarFill.setFillColor(CYAN);

    if (m_fontLoaded) {
        m_ammoText.setFont(m_font);
        m_ammoText.setCharacterSize(16);
        m_ammoText.setFillColor(TEXT_PRI);

        m_alertText.setFont(m_font);
        m_alertText.setCharacterSize(14);
        m_alertText.setFillColor(CYAN);
    }

    return m_fontLoaded;
}

void HUD::render(sf::RenderWindow& window,
                 const Health&     hp,
                 const Weapon&     weapon,
                 int               alertLevel) {
    const sf::View& view = window.getDefaultView();
    float W = view.getSize().x;
    float H = view.getSize().y;

    // Ensure we're in screen space
    window.setView(view);

    // ── HP bar (bottom-left) ─────────────────────────────────────────────────
    float ratio = static_cast<float>(hp.current) / static_cast<float>(std::max(hp.max, 1));
    ratio = std::clamp(ratio, 0.f, 1.f);

    float barY = H - 32.f;
    m_hpBarBg.setPosition(16.f, barY);
    window.draw(m_hpBarBg);

    float fillW = 200.f * ratio;
    // Gradient: cyan → magenta at low HP (< 30%)
    sf::Color fillColor = ratio < 0.3f
        ? sf::Color(
            static_cast<sf::Uint8>(0x00 + static_cast<int>((1.f - ratio / 0.3f) * 0xFF)),
            static_cast<sf::Uint8>(0xFF * ratio / 0.3f),
            static_cast<sf::Uint8>(0x6B + static_cast<int>((1.f - ratio / 0.3f) * (0xEE - 0x6B)))
          )
        : CYAN;
    m_hpBarFill.setSize({fillW, 10.f});
    m_hpBarFill.setFillColor(fillColor);
    m_hpBarFill.setPosition(18.f, barY + 2.f);
    window.draw(m_hpBarFill);

    // ── Ammo counter (bottom-right) ──────────────────────────────────────────
    if (m_fontLoaded) {
        char ammoStr[32];
        std::snprintf(ammoStr, sizeof(ammoStr), "%d / %d", weapon.ammo, weapon.maxAmmo);
        m_ammoText.setString(ammoStr);
        sf::FloatRect bounds = m_ammoText.getLocalBounds();
        m_ammoText.setPosition(W - bounds.width - 16.f, H - 32.f);
        window.draw(m_ammoText);

        // ── Alert level (top-right) ──────────────────────────────────────────
        const char* alertStr[] = { "STEALTH", "ALERT 1", "ALERT 2", "COMBAT" };
        const sf::Color alertColors[] = { CYAN, YELLOW, MAGENTA, DANGER };
        int al = std::clamp(alertLevel, 0, 3);
        m_alertText.setString(alertStr[al]);
        m_alertText.setFillColor(alertColors[al]);
        sf::FloatRect ab = m_alertText.getLocalBounds();
        m_alertText.setPosition(W - ab.width - 16.f, 16.f);
        window.draw(m_alertText);
    }
}
