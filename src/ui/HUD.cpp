#include "HUD.h"
#include <algorithm>
#include <cstdio>
#include <fstream>

static const sf::Color CYAN    = sf::Color(0x00, 0xFF, 0xEE, 0xFF);
static const sf::Color MAGENTA = sf::Color(0xFF, 0x00, 0x6B, 0xFF);
static const sf::Color UI_BASE = sf::Color(0x0A, 0x10, 0x20, 0xCC);
static const sf::Color UI_BDR  = sf::Color(0x1A, 0x28, 0x40, 0xFF);
static const sf::Color TEXT_PRI= sf::Color(0xE0, 0xEE, 0xF8, 0xFF);
static const sf::Color TEXT_DIM= sf::Color(0x60, 0x80, 0xA0, 0xFF);
static const sf::Color DANGER  = sf::Color(0xFF, 0x00, 0x38, 0xFF);
static const sf::Color YELLOW  = sf::Color(0xFF, 0xE6, 0x00, 0xFF);
static const sf::Color VIOLET  = sf::Color(0xAA, 0x00, 0xFF, 0xFF);

static const char* WEAPON_ABBR[WeaponSystem::WEAPON_SLOTS] = {
    "P-9", "SMG", "RLG", "SHG", "EMP", "NSP"
};

bool HUD::init(const std::string& fontPath) {
    if (std::ifstream(fontPath).is_open())
        m_fontLoaded = m_font.loadFromFile(fontPath);
    else
        m_fontLoaded = false;

    m_hpBarBg.setSize({204.f, 14.f});
    m_hpBarBg.setFillColor(UI_BASE);
    m_hpBarBg.setOutlineColor(UI_BDR);
    m_hpBarBg.setOutlineThickness(1.f);

    m_hpBarFill.setSize({200.f, 10.f});
    m_hpBarFill.setFillColor(CYAN);

    if (m_fontLoaded) {
        m_ammoText.setFont(m_font);
        m_ammoText.setCharacterSize(16);
        m_ammoText.setFillColor(TEXT_PRI);

        m_alertText.setFont(m_font);
        m_alertText.setCharacterSize(14);
        m_alertText.setFillColor(CYAN);

        m_hackText.setFont(m_font);
        m_hackText.setCharacterSize(20);
        m_hackText.setFillColor(VIOLET);
        m_hackText.setString("[ HACKING... ]");
    }

    m_hackBarBg.setSize({204.f, 14.f});
    m_hackBarBg.setFillColor(UI_BASE);
    m_hackBarBg.setOutlineColor(VIOLET);
    m_hackBarBg.setOutlineThickness(1.f);

    m_hackBarFill.setFillColor(VIOLET);

    return m_fontLoaded;
}

void HUD::renderWeaponSlots(sf::RenderWindow& window, const WeaponSystem& weapons, float /*W*/, float H) {
    // 6 slots, 44×44px each, 4px gap, anchored bottom-left above HP bar
    constexpr float SLOT_W = 44.f;
    constexpr float SLOT_H = 44.f;
    constexpr float GAP    = 4.f;
    const float startX = 16.f;
    const float startY = H - 32.f - SLOT_H - 8.f;  // above HP bar with 8px margin

    sf::RectangleShape slotBg({SLOT_W, SLOT_H});
    sf::RectangleShape slotFill;
    sf::Text slotNum, slotName;

    if (m_fontLoaded) {
        slotNum.setFont(m_font);
        slotNum.setCharacterSize(10);
        slotName.setFont(m_font);
        slotName.setCharacterSize(12);
    }

    int active = weapons.activeSlot();

    for (int i = 0; i < WeaponSystem::WEAPON_SLOTS; ++i) {
        float sx = startX + i * (SLOT_W + GAP);
        float sy = startY;
        bool unlocked = weapons.isUnlocked(i);
        bool isActive = (i == active);

        // Background
        slotBg.setPosition(sx, sy);
        slotBg.setFillColor(isActive
            ? sf::Color(0x00, 0x28, 0x28, 0xCC)
            : UI_BASE);
        slotBg.setOutlineThickness(isActive ? 2.f : 1.f);
        slotBg.setOutlineColor(isActive
            ? CYAN
            : (unlocked ? UI_BDR : sf::Color(0x1A, 0x28, 0x40, 0x60)));
        window.draw(slotBg);

        if (!unlocked) continue;

        if (!m_fontLoaded) continue;

        // Slot number (top-left, dim)
        char numBuf[4];
        std::snprintf(numBuf, sizeof(numBuf), "%d", i + 1);
        slotNum.setString(numBuf);
        slotNum.setFillColor(isActive ? CYAN : TEXT_DIM);
        slotNum.setPosition(sx + 4.f, sy + 3.f);
        window.draw(slotNum);

        // Weapon abbreviation (centered, active = cyan, else dim)
        slotName.setString(WEAPON_ABBR[i]);
        slotName.setFillColor(isActive ? CYAN : TEXT_DIM);
        sf::FloatRect nb = slotName.getLocalBounds();
        slotName.setPosition(sx + (SLOT_W - nb.width) * 0.5f, sy + (SLOT_H - nb.height) * 0.5f + 2.f);
        window.draw(slotName);

        // Ammo count (bottom-right, small)
        char ammoBuf[8];
        std::snprintf(ammoBuf, sizeof(ammoBuf), "%d", weapons.ammo(i));
        slotNum.setString(ammoBuf);
        slotNum.setFillColor(isActive ? TEXT_PRI : TEXT_DIM);
        sf::FloatRect ab = slotNum.getLocalBounds();
        slotNum.setPosition(sx + SLOT_W - ab.width - 4.f, sy + SLOT_H - 14.f);
        window.draw(slotNum);
    }
}

void HUD::render(sf::RenderWindow&   window,
                 const Health&       hp,
                 const Weapon&       weapon,
                 const WeaponSystem& weapons,
                 int                 alertLevel) {
    const sf::View& view = window.getDefaultView();
    float W = view.getSize().x;
    float H = view.getSize().y;
    window.setView(view);

    // ── HP bar (bottom-left, below weapon slots) ─────────────────────────────
    float ratio = static_cast<float>(hp.current) / static_cast<float>(std::max(hp.max, 1));
    ratio = std::clamp(ratio, 0.f, 1.f);

    float barY = H - 32.f;
    m_hpBarBg.setPosition(16.f, barY);
    window.draw(m_hpBarBg);

    // Gradient: cyan → magenta at low HP (< 30%)
    sf::Color fillColor = ratio < 0.3f
        ? sf::Color(
            static_cast<sf::Uint8>(0x00 + static_cast<int>((1.f - ratio / 0.3f) * 0xFF)),
            static_cast<sf::Uint8>(0xFF * ratio / 0.3f),
            static_cast<sf::Uint8>(0x6B + static_cast<int>((1.f - ratio / 0.3f) * (0xEE - 0x6B)))
          )
        : CYAN;
    m_hpBarFill.setSize({200.f * ratio, 10.f});
    m_hpBarFill.setFillColor(fillColor);
    m_hpBarFill.setPosition(18.f, barY + 2.f);
    window.draw(m_hpBarFill);

    // ── Weapon slots ─────────────────────────────────────────────────────────
    renderWeaponSlots(window, weapons, W, H);

    // ── Ammo counter for active weapon (bottom-right) ────────────────────────
    if (m_fontLoaded) {
        char ammoStr[32];
        std::snprintf(ammoStr, sizeof(ammoStr), "%d / %d", weapon.ammo, weapon.maxAmmo);
        m_ammoText.setString(ammoStr);
        sf::FloatRect bounds = m_ammoText.getLocalBounds();
        m_ammoText.setPosition(W - bounds.width - 16.f, H - 32.f);
        window.draw(m_ammoText);

        // ── Alert level (top-right) ──────────────────────────────────────────
        const char* alertStr[]    = { "STEALTH", "ALERT 1", "ALERT 2", "COMBAT" };
        const sf::Color alertColors[] = { CYAN, YELLOW, MAGENTA, DANGER };
        int al = std::clamp(alertLevel, 0, 3);
        m_alertText.setString(alertStr[al]);
        m_alertText.setFillColor(alertColors[al]);
        sf::FloatRect ab = m_alertText.getLocalBounds();
        m_alertText.setPosition(W - ab.width - 16.f, 16.f);
        window.draw(m_alertText);
    }
}

void HUD::renderHackOverlay(sf::RenderWindow& window, float progress) {
    const sf::View& view = window.getDefaultView();
    float W = view.getSize().x;
    float H = view.getSize().y;
    window.setView(view);

    float cx = W * 0.5f;
    float cy = H * 0.42f;

    sf::RectangleShape panel({240.f, 60.f});
    panel.setFillColor(sf::Color(0x0A, 0x10, 0x20, 0xDD));
    panel.setOutlineColor(VIOLET);
    panel.setOutlineThickness(2.f);
    panel.setPosition(cx - 120.f, cy - 10.f);
    window.draw(panel);

    if (m_fontLoaded) {
        sf::FloatRect tb = m_hackText.getLocalBounds();
        m_hackText.setPosition(cx - tb.width * 0.5f, cy - 4.f);
        window.draw(m_hackText);
    }

    float barY = cy + 30.f;
    m_hackBarBg.setSize({204.f, 10.f});
    m_hackBarBg.setPosition(cx - 102.f, barY);
    window.draw(m_hackBarBg);

    float fillW = std::max(0.f, 200.f * std::clamp(progress, 0.f, 1.f));
    if (fillW > 0.f) {
        m_hackBarFill.setSize({fillW, 6.f});
        m_hackBarFill.setPosition(cx - 100.f, barY + 2.f);
        window.draw(m_hackBarFill);
    }
}
