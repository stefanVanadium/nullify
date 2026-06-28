#include "Renderer.h"
#include "ecs/Components.h"
#include <algorithm>
#include <vector>

void Renderer::render(sf::RenderWindow& window, World& world, float alpha) {
    // Collect alive renderable entities, sort by layer
    // This vector lives on the stack via small-buffer trick; for V0.1 entity counts
    // are low enough that heap allocation here is acceptable (called ~144×/sec, < 1024 items).
    struct DrawItem { int layer; uint32_t id; };
    static std::vector<DrawItem> items; // static avoids alloc per frame
    items.clear();

    for (uint32_t i = 0; i < MAX_ENTITIES; ++i) {
        if (!world.isAlive(i)) continue;
        if (!world.hasComponent<Renderable>(i)) continue;
        if (!world.getComponent<Renderable>(i).visible) continue;
        items.push_back({world.getComponent<Renderable>(i).layer, i});
    }

    std::sort(items.begin(), items.end(),
        [](const DrawItem& a, const DrawItem& b) { return a.layer < b.layer; });

    for (const auto& item : items) {
        const auto& t = world.getComponent<Transform>(item.id);
        auto& r       = world.getComponent<Renderable>(item.id);

        // Interpolate between previous and current position
        float drawX = t.prevX + alpha * (t.x - t.prevX);
        float drawY = t.prevY + alpha * (t.y - t.prevY);

        r.shape.setPosition(drawX, drawY);
        window.draw(r.shape);
    }
}
