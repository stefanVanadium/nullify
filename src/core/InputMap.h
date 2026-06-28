#pragma once
#include <SFML/Window.hpp>
#include <array>

enum class Action : uint8_t {
    MoveLeft    = 0,
    MoveRight   = 1,
    Jump        = 2,
    Crouch      = 3,
    Dash        = 4,
    Hack        = 5,
    Fire        = 6,
    Takedown    = 7,
    WeaponNext  = 8,
    WeaponPrev  = 9,
    COUNT       = 10
};

// Single source of truth for input state. Only class that touches sf::Keyboard/Mouse.
class InputMap {
public:
    InputMap();

    // Call once per frame with each sf::Event from the event pump
    void processEvent(const sf::Event& event);
    // Call at end of each frame to advance pressed/released to held state
    void flush();

    bool isHeld(Action a)     const;
    bool isPressed(Action a)  const;
    bool isReleased(Action a) const;

    sf::Vector2i mouseScreenPos() const { return m_mousePos; }

private:
    static constexpr size_t N = static_cast<size_t>(Action::COUNT);

    std::array<bool, N> m_held{};
    std::array<bool, N> m_pressed{};
    std::array<bool, N> m_released{};
    sf::Vector2i        m_mousePos{};

    // Map SFML key to Action; returns false if key is unmapped
    bool keyToAction(sf::Keyboard::Key key, Action& out) const;
};
