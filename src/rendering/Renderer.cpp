#include "Renderer.h"
#include "ecs/Components.h"
#include <algorithm>
#include <cmath>

void Renderer::init(sf::Vector2u windowSize) {
    float w = static_cast<float>(windowSize.x);
    float h = static_cast<float>(windowSize.y);

    // Scanlines overlay: no texture coords needed (gl_FragCoord used in shader)
    m_scanlineQuad[0] = {sf::Vector2f(0, 0), sf::Color::White};
    m_scanlineQuad[1] = {sf::Vector2f(w, 0), sf::Color::White};
    m_scanlineQuad[2] = {sf::Vector2f(w, h), sf::Color::White};
    m_scanlineQuad[3] = {sf::Vector2f(0, h), sf::Color::White};

    // Vignette overlay: UV coords (0,0)→(1,1) as texture coords
    m_vignetteQuad[0] = {sf::Vector2f(0, 0), sf::Color::White, sf::Vector2f(0.f, 0.f)};
    m_vignetteQuad[1] = {sf::Vector2f(w, 0), sf::Color::White, sf::Vector2f(1.f, 0.f)};
    m_vignetteQuad[2] = {sf::Vector2f(w, h), sf::Color::White, sf::Vector2f(1.f, 1.f)};
    m_vignetteQuad[3] = {sf::Vector2f(0, h), sf::Color::White, sf::Vector2f(0.f, 1.f)};

    m_crosshair.setRadius(4.f);
    m_crosshair.setOrigin(4.f, 4.f);
    m_crosshair.setFillColor(sf::Color(0x00, 0xFF, 0xEE, 0xCC));
    m_crosshair.setOutlineColor(sf::Color(0x00, 0xFF, 0xEE, 0xFF));
    m_crosshair.setOutlineThickness(1.f);
}

int Renderer::render(sf::RenderWindow& window,
                     World&            world,
                     float             alpha,
                     float             hpRatio,
                     sf::Vector2f      mouseWorldPos,
                     const TileMap&    tileMap,
                     ParallaxSystem&   parallax,
                     ShaderManager&    shaders,
                     const WeaponSystem& weapon) {
    int drawCalls = 0;

    // ── World space (camera view already applied by caller) ──────────────────

    // 1. Parallax background layers (switch to default view inside)
    parallax.render(window, window.getView().getCenter().x);
    drawCalls += 3;

    // 2. Static tile geometry — 1 draw call
    if (tileMap.vertexCount() > 0) {
        window.draw(tileMap.vertices(), tileMap.vertexCount(), sf::Quads);
        ++drawCalls;
    }

    // 3. Dynamic entities via SpriteBatch
    struct DrawItem { int layer; uint32_t id; };
    static std::array<DrawItem, MAX_ENTITIES> items;
    int itemCount = 0;

    for (uint32_t i = 0; i < MAX_ENTITIES; ++i) {
        if (!world.isAlive(i)) continue;
        if (!world.hasComponent<Renderable>(i)) continue;
        const auto& r = world.getComponent<Renderable>(i);
        if (!r.visible) continue;
        items[static_cast<size_t>(itemCount++)] = {r.layer, i};
    }

    std::sort(items.data(), items.data() + itemCount,
        [](const DrawItem& a, const DrawItem& b) { return a.layer < b.layer; });

    m_entityBatch.begin();
    for (int k = 0; k < itemCount; ++k) {
        uint32_t id  = items[static_cast<size_t>(k)].id;
        const auto& t = world.getComponent<Transform>(id);
        const auto& r = world.getComponent<Renderable>(id);

        float drawX = t.prevX + alpha * (t.x - t.prevX);
        float drawY = t.prevY + alpha * (t.y - t.prevY);
        m_entityBatch.draw({drawX, drawY}, r.size, r.color);
    }
    weapon.batchDraw(m_entityBatch);   // bullets in same batch = 0 extra draw calls
    drawCalls += m_entityBatch.end(window);

    // 4. Crosshair (world space)
    m_crosshair.setPosition(mouseWorldPos);
    window.draw(m_crosshair);
    ++drawCalls;

    // ── Screen space overlays ────────────────────────────────────────────────
    sf::View screenView = window.getDefaultView();
    window.setView(screenView);

    // 5. Scanlines shader overlay
    if (sf::Shader* sl = shaders.get(ShaderType::Scanlines)) {
        sf::RenderStates rs;
        rs.shader    = sl;
        rs.blendMode = sf::BlendAlpha;
        window.draw(m_scanlineQuad.data(), 4, sf::Quads, rs);
        ++drawCalls;
    }

    // 6. Vignette shader overlay — intensity increases at low HP
    if (sf::Shader* vg = shaders.get(ShaderType::Vignette)) {
        float intensity = 0.7f + (1.0f - hpRatio) * 0.9f; // 0.7 full HP → 1.6 at 0 HP
        vg->setUniform("intensity", intensity);
        sf::RenderStates rs;
        rs.shader    = vg;
        rs.blendMode = sf::BlendAlpha;
        window.draw(m_vignetteQuad.data(), 4, sf::Quads, rs);
        ++drawCalls;
    }

    return drawCalls;
}
