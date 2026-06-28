#include "GameState.h"

void GameStateManager::pushState(std::unique_ptr<IGameState> state) {
    m_pendingOp    = PendingOp::Push;
    m_pendingState = std::move(state);
}

void GameStateManager::popState() {
    m_pendingOp = PendingOp::Pop;
}

void GameStateManager::replaceState(std::unique_ptr<IGameState> state) {
    m_pendingOp    = PendingOp::Replace;
    m_pendingState = std::move(state);
}

void GameStateManager::update(float dt) {
    if (!m_stack.empty())
        m_stack.back()->update(dt);
    applyPending();
}

void GameStateManager::render(sf::RenderWindow& window, float alpha) {
    if (!m_stack.empty())
        m_stack.back()->render(window, alpha);
}

void GameStateManager::applyPending() {
    switch (m_pendingOp) {
        case PendingOp::Push:
            m_stack.push_back(std::move(m_pendingState));
            m_stack.back()->enter();
            break;
        case PendingOp::Pop:
            if (!m_stack.empty()) {
                m_stack.back()->exit();
                m_stack.pop_back();
            }
            break;
        case PendingOp::Replace:
            if (!m_stack.empty()) {
                m_stack.back()->exit();
                m_stack.pop_back();
            }
            m_stack.push_back(std::move(m_pendingState));
            m_stack.back()->enter();
            break;
        case PendingOp::None:
            break;
    }
    m_pendingOp = PendingOp::None;
}
