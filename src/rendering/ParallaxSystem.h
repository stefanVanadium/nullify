#pragma once
#include <SFML/Graphics.hpp>
#include <array>

// Three placeholder colored background layers scrolling at different speeds.
// Replaced with real sprite layers in Sprint 3 once art assets exist.
class ParallaxSystem {
public:
    void init(sf::Vector2u windowSize);
    // cameraX: world x of the camera center this frame
    void render(sf::RenderTarget& target, float cameraX);

    int drawCallCount() const { return 3; }

private:
    struct Layer {
        sf::RectangleShape rect;
        float              speedFactor;
        sf::Color          color;
    };

    static constexpr int NUM_LAYERS = 3;
    std::array<Layer, NUM_LAYERS> m_layers;
    sf::Vector2u                  m_windowSize;
};
