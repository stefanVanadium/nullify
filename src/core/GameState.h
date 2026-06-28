#pragma once
#include <memory>
#include <vector>
#include <SFML/Graphics.hpp>

class InputMap;
class World;

// Base state interface
class IGameState {
public:
    virtual ~IGameState() = default;
    virtual void enter()                         = 0;
    virtual void exit()                          = 0;
    virtual void update(float dt)                = 0;
    virtual void render(sf::RenderWindow& window, float alpha) = 0;
};

// State machine — push/pop/replace stack
class GameStateManager {
public:
    void pushState(std::unique_ptr<IGameState> state);
    void popState();
    void replaceState(std::unique_ptr<IGameState> state);

    void update(float dt);
    void render(sf::RenderWindow& window, float alpha);

    bool         empty() const { return m_stack.empty(); }
    IGameState*  top()   const { return m_stack.empty() ? nullptr : m_stack.back().get(); }

private:
    std::vector<std::unique_ptr<IGameState>> m_stack;

    // Pending operations applied after update to avoid mid-frame mutation
    enum class PendingOp { None, Push, Pop, Replace };
    PendingOp                       m_pendingOp    = PendingOp::None;
    std::unique_ptr<IGameState>     m_pendingState;

    void applyPending();
};
