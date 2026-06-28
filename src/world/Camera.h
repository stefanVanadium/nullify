#pragma once
#include <SFML/Graphics.hpp>
#include "ecs/World.h"

class Camera {
public:
    explicit Camera(World& world);

    void setLevelBounds(float levelW, float levelH);
    void update(float dt);
    void apply(sf::RenderWindow& window);

    sf::Vector2f screenToWorld(sf::Vector2i screenPos, const sf::RenderWindow& window) const;
    sf::Vector2f center() const { return m_view.getCenter(); }

private:
    static constexpr float CAMERA_LERP = 5.0f;

    World&    m_world;
    sf::View  m_view;
    float     m_levelW = 0.0f;
    float     m_levelH = 0.0f;
};
