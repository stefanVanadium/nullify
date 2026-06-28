#include "Renderer.h"
#include "enemies/EnemyManager.h"
#include "ecs/Components.h"
#include <algorithm>
#include <cmath>

void Renderer::init(sf::Vector2u windowSize) {
    m_sceneW = static_cast<float>(windowSize.x);
    m_sceneH = static_cast<float>(windowSize.y);

    m_sceneRT.create(windowSize.x, windowSize.y);
    m_sceneSprite.setTexture(m_sceneRT.getTexture());

    // Scanlines: no UVs needed (gl_FragCoord used in shader)
    m_scanlineQuad[0] = {sf::Vector2f(0,       0      ), sf::Color::White};
    m_scanlineQuad[1] = {sf::Vector2f(m_sceneW,0      ), sf::Color::White};
    m_scanlineQuad[2] = {sf::Vector2f(m_sceneW,m_sceneH), sf::Color::White};
    m_scanlineQuad[3] = {sf::Vector2f(0,       m_sceneH), sf::Color::White};

    // Vignette: UV (0,0)→(1,1)
    m_vignetteQuad[0] = {sf::Vector2f(0,       0      ), sf::Color::White, sf::Vector2f(0.f,1.f)};
    m_vignetteQuad[1] = {sf::Vector2f(m_sceneW,0      ), sf::Color::White, sf::Vector2f(1.f,1.f)};
    m_vignetteQuad[2] = {sf::Vector2f(m_sceneW,m_sceneH), sf::Color::White, sf::Vector2f(1.f,0.f)};
    m_vignetteQuad[3] = {sf::Vector2f(0,       m_sceneH), sf::Color::White, sf::Vector2f(0.f,0.f)};

    m_crosshair.setRadius(4.f);
    m_crosshair.setOrigin(4.f, 4.f);
    m_crosshair.setFillColor(sf::Color(0x00, 0xFF, 0xEE, 0xCC));
    m_crosshair.setOutlineColor(sf::Color(0x00, 0xFF, 0xEE, 0xFF));
    m_crosshair.setOutlineThickness(1.f);
}

int Renderer::render(sf::RenderWindow&    window,
                     World&               world,
                     float                alpha,
                     float                hpRatio,
                     sf::Vector2f         mouseWorldPos,
                     const TileMap&       tileMap,
                     ParallaxSystem&      parallax,
                     ShaderManager&       shaders,
                     const WeaponSystem&  weapon,
                     const EnemyManager&  enemies,
                     ParticleSystem&      particles,
                     const RagdollSystem& ragdolls,
                     const RenderEffects& effects) {
    int drawCalls = 0;

    // ── Phase 1: Render scene into m_sceneRT ────────────────────────────────
    sf::View cameraView = window.getView();  // camera view set by caller

    m_sceneRT.clear(sf::Color(0x05, 0x08, 0x10));
    m_sceneRT.setView(cameraView);

    // 1. Parallax background layers (internally switches to RT default view)
    parallax.render(m_sceneRT, cameraView.getCenter().x);
    m_sceneRT.setView(cameraView);
    drawCalls += 3;

    // 2. Static tile geometry
    if (tileMap.vertexCount() > 0) {
        m_sceneRT.draw(tileMap.vertices(), tileMap.vertexCount(), sf::Quads);
        ++drawCalls;
    }

    // 3. Dynamic entities (sorted by layer) + player bullets + enemy bullets + particles
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
    weapon.batchDraw(m_entityBatch);
    enemies.batchDrawBullets(m_entityBatch);
    ragdolls.render(m_entityBatch);
    particles.batchDraw(m_entityBatch);
    drawCalls += m_entityBatch.end(m_sceneRT);

    // 4. Crosshair (world space)
    m_crosshair.setPosition(mouseWorldPos);
    m_sceneRT.draw(m_crosshair);
    ++drawCalls;

    m_sceneRT.display();

    // ── Phase 2: Draw scene to window with post-process shaders ─────────────
    window.setView(window.getDefaultView());

    // 5. NeonGlow + chromatic aberration on scene texture
    // Texture handle is stable; content updated by m_sceneRT.display() above
    sf::Sprite& sp = m_sceneSprite;

    if (sf::Shader* ng = shaders.get(ShaderType::NeonGlow)) {
        ng->setUniform("texture",    sf::Shader::CurrentTexture);
        ng->setUniform("texelSize",  sf::Glsl::Vec2(1.f / m_sceneW, 1.f / m_sceneH));
        ng->setUniform("caIntensity", effects.caIntensity);
        window.draw(sp, ng);
    } else {
        window.draw(sp);  // fallback: no bloom
    }
    ++drawCalls;

    // 6. Scanlines overlay
    if (sf::Shader* sl = shaders.get(ShaderType::Scanlines)) {
        sf::RenderStates rs;
        rs.shader    = sl;
        rs.blendMode = sf::BlendAlpha;
        window.draw(m_scanlineQuad.data(), 4, sf::Quads, rs);
        ++drawCalls;
    }

    // 7. Vignette overlay — intensity increases at low HP
    if (sf::Shader* vg = shaders.get(ShaderType::Vignette)) {
        float intensity = 0.35f + (1.0f - hpRatio) * 0.45f;
        vg->setUniform("intensity", intensity);
        sf::RenderStates rs;
        rs.shader    = vg;
        rs.blendMode = sf::BlendAlpha;
        window.draw(m_vignetteQuad.data(), 4, sf::Quads, rs);
        ++drawCalls;
    }

    // 8. Glitch overlay — only during hack mode (separate pass on scene texture)
    if (effects.hackIntensity > 0.001f) {
        if (sf::Shader* gl = shaders.get(ShaderType::Glitch)) {
            gl->setUniform("texture",   sf::Shader::CurrentTexture);
            gl->setUniform("intensity", effects.hackIntensity);
            gl->setUniform("time",      effects.gameTime);
            window.draw(sp, gl);
            ++drawCalls;
        }
    }

    return drawCalls;
}
