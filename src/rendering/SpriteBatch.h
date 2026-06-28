#pragma once
#include <SFML/Graphics.hpp>
#include <array>
#include <cstddef>

// Accumulates colored or textured quads into a single VertexArray, then flushes
// in one draw call. All storage is pre-allocated — zero heap in the game loop.
class SpriteBatch {
public:
    static constexpr size_t MAX_QUADS = 8192;

    SpriteBatch() = default;

    void begin();
    void draw(sf::Vector2f pos, sf::Vector2f size, sf::Color color);
    void draw(sf::IntRect texRect, sf::Vector2f pos, sf::Vector2f size,
              sf::Color tint = sf::Color::White);

    // Flushes to target. Returns 1 if anything drawn, 0 otherwise.
    int end(sf::RenderTarget& target, const sf::Texture* texture = nullptr);

    size_t quadCount() const { return m_quadCount; }

private:
    std::array<sf::Vertex, MAX_QUADS * 4> m_vertices{};
    size_t m_quadCount = 0;
};
