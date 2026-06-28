#include "SpriteBatch.h"

void SpriteBatch::begin() {
    m_quadCount = 0;
}

void SpriteBatch::draw(sf::Vector2f pos, sf::Vector2f size, sf::Color color) {
    if (m_quadCount >= MAX_QUADS) return;
    size_t b = m_quadCount * 4;
    m_vertices[b + 0] = {sf::Vector2f(pos.x,          pos.y         ), color, {0.f, 0.f}};
    m_vertices[b + 1] = {sf::Vector2f(pos.x + size.x, pos.y         ), color, {0.f, 0.f}};
    m_vertices[b + 2] = {sf::Vector2f(pos.x + size.x, pos.y + size.y), color, {0.f, 0.f}};
    m_vertices[b + 3] = {sf::Vector2f(pos.x,          pos.y + size.y), color, {0.f, 0.f}};
    ++m_quadCount;
}

void SpriteBatch::draw(sf::IntRect texRect, sf::Vector2f pos, sf::Vector2f size, sf::Color tint) {
    if (m_quadCount >= MAX_QUADS) return;
    size_t b  = m_quadCount * 4;
    float  tx = static_cast<float>(texRect.left);
    float  ty = static_cast<float>(texRect.top);
    float  tw = static_cast<float>(texRect.width);
    float  th = static_cast<float>(texRect.height);
    m_vertices[b + 0] = {sf::Vector2f(pos.x,          pos.y         ), tint, {tx,      ty     }};
    m_vertices[b + 1] = {sf::Vector2f(pos.x + size.x, pos.y         ), tint, {tx + tw, ty     }};
    m_vertices[b + 2] = {sf::Vector2f(pos.x + size.x, pos.y + size.y), tint, {tx + tw, ty + th}};
    m_vertices[b + 3] = {sf::Vector2f(pos.x,          pos.y + size.y), tint, {tx,      ty + th}};
    ++m_quadCount;
}

int SpriteBatch::end(sf::RenderTarget& target, const sf::Texture* texture) {
    if (m_quadCount == 0) return 0;
    sf::RenderStates states;
    states.texture = texture;
    target.draw(m_vertices.data(), m_quadCount * 4, sf::Quads, states);
    return 1;
}
