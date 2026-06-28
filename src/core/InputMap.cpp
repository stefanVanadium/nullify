#include "InputMap.h"

InputMap::InputMap() {
    m_held.fill(false);
    m_pressed.fill(false);
    m_released.fill(false);
}

bool InputMap::keyToAction(sf::Keyboard::Key key, Action& out) const {
    switch (key) {
        case sf::Keyboard::A:     case sf::Keyboard::Left:  out = Action::MoveLeft;  return true;
        case sf::Keyboard::D:     case sf::Keyboard::Right: out = Action::MoveRight; return true;
        case sf::Keyboard::W:     case sf::Keyboard::Space: out = Action::Jump;       return true;
        case sf::Keyboard::S:     case sf::Keyboard::Down:  out = Action::Crouch;    return true;
        case sf::Keyboard::LShift:                          out = Action::Dash;      return true;
        case sf::Keyboard::E:                               out = Action::Hack;      return true;
        default: return false;
    }
}

void InputMap::processEvent(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        Action a;
        if (keyToAction(event.key.code, a)) {
            size_t i = static_cast<size_t>(a);
            if (!m_held[i]) {   // avoid repeat events
                m_pressed[i] = true;
                m_held[i]    = true;
            }
        }
    } else if (event.type == sf::Event::KeyReleased) {
        Action a;
        if (keyToAction(event.key.code, a)) {
            size_t i = static_cast<size_t>(a);
            m_held[i]     = false;
            m_released[i] = true;
        }
    } else if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left)
            m_pressed[static_cast<size_t>(Action::Fire)] = true,
            m_held[static_cast<size_t>(Action::Fire)]    = true;
    } else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left)
            m_held[static_cast<size_t>(Action::Fire)]     = false,
            m_released[static_cast<size_t>(Action::Fire)] = true;
    }
}

void InputMap::flush() {
    m_pressed.fill(false);
    m_released.fill(false);
}

bool InputMap::isHeld(Action a)     const { return m_held[static_cast<size_t>(a)]; }
bool InputMap::isPressed(Action a)  const { return m_pressed[static_cast<size_t>(a)]; }
bool InputMap::isReleased(Action a) const { return m_released[static_cast<size_t>(a)]; }
