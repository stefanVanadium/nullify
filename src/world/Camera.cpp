#include "Camera.h"
#include "ecs/Components.h"
#include <algorithm>

Camera::Camera(World& world)
    : m_world(world)
    , m_view(sf::FloatRect(0, 0, 1280, 720))
{}

void Camera::setLevelBounds(float levelW, float levelH) {
    m_levelW = levelW;
    m_levelH = levelH;
}

void Camera::update(float dt) {
    uint32_t pid = m_world.findPlayer();
    if (pid == static_cast<uint32_t>(MAX_ENTITIES)) return;
    if (!m_world.hasComponent<Transform>(pid)) return;

    const auto& t = m_world.getComponent<Transform>(pid);

    // Target: center the camera on the player
    float targetX = t.x + 12.0f; // half of player width (24px)
    float targetY = t.y + 24.0f; // half of player height (48px)

    sf::Vector2f center = m_view.getCenter();
    float halfW = m_view.getSize().x * 0.5f;
    float halfH = m_view.getSize().y * 0.5f;

    // Lerp toward target
    center.x += (targetX - center.x) * CAMERA_LERP * dt;
    center.y += (targetY - center.y) * CAMERA_LERP * dt;

    // Clamp to level bounds; guard against level smaller than viewport
    if (m_levelW > 0.0f) {
        float lo = halfW, hi = std::max(halfW, m_levelW - halfW);
        center.x = std::clamp(center.x, lo, hi);
    }
    if (m_levelH > 0.0f) {
        float lo = halfH, hi = std::max(halfH, m_levelH - halfH);
        center.y = std::clamp(center.y, lo, hi);
    }

    m_view.setCenter(center);
}

void Camera::apply(sf::RenderWindow& window) {
    window.setView(m_view);
}
