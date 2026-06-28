#include "ParallaxSystem.h"
#include <cmath>

void ParallaxSystem::init(sf::Vector2u windowSize) {
    m_windowSize = windowSize;

    // Layer 0: distant skyline — barely moves
    m_layers[0].speedFactor = 0.1f;
    m_layers[0].color       = sf::Color(0x07, 0x0C, 0x18, 0xFF); // near-black

    // Layer 1: mid-distance buildings
    m_layers[1].speedFactor = 0.3f;
    m_layers[1].color       = sf::Color(0x0A, 0x14, 0x22, 0xFF);

    // Layer 2: foreground silhouettes
    m_layers[2].speedFactor = 0.6f;
    m_layers[2].color       = sf::Color(0x0E, 0x1C, 0x30, 0xFF);

    for (auto& l : m_layers) {
        l.rect.setSize(sf::Vector2f(
            static_cast<float>(windowSize.x),
            static_cast<float>(windowSize.y)
        ));
        l.rect.setFillColor(l.color);
    }
}

void ParallaxSystem::render(sf::RenderTarget& target, float cameraX) {
    sf::View defaultView = target.getDefaultView();
    sf::View saved       = target.getView();
    target.setView(defaultView);

    float w = static_cast<float>(m_windowSize.x);
    for (auto& l : m_layers) {
        // Offset the layer left as camera moves right, but slower
        float offset = std::fmod(cameraX * l.speedFactor, w);
        l.rect.setPosition(-offset, 0.f);
        target.draw(l.rect);
    }

    target.setView(saved);
}
