#include "World.h"
#include <cassert>

World::World() {
    m_alive.fill(false);
}

uint32_t World::createEntity() {
    assert(m_aliveCount < MAX_ENTITIES && "Entity pool exhausted");
    // Find next free slot (simple linear scan — called rarely outside hot path)
    while (m_alive[m_nextId]) {
        m_nextId = (m_nextId + 1) % MAX_ENTITIES;
    }
    uint32_t id = m_nextId;
    m_alive[id] = true;
    m_mask[id].reset();
    ++m_aliveCount;
    m_nextId = (m_nextId + 1) % MAX_ENTITIES;
    return id;
}

void World::destroyEntity(uint32_t id) {
    assert(id < MAX_ENTITIES);
    if (!m_alive[id]) return;
    m_alive[id] = false;
    m_mask[id].reset();
    --m_aliveCount;
}

bool World::isAlive(uint32_t id) const {
    return id < MAX_ENTITIES && m_alive[id];
}

uint32_t World::findPlayer() const {
    for (uint32_t i = 0; i < MAX_ENTITIES; ++i) {
        if (m_alive[i] && m_mask[i].test(static_cast<uint8_t>(ComponentType::PlayerTag)))
            return i;
    }
    return static_cast<uint32_t>(MAX_ENTITIES);
}
