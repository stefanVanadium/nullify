#pragma once
#include <SFML/Graphics.hpp>

// Abstract base for all hacking minigame tiers.
// Returned by HackMinigame::create(tier). Owned by HackingUI.
class IHackMinigame {
public:
    enum class Result { RUNNING, SUCCESS, FAILURE };

    virtual ~IHackMinigame() = default;

    // Called every game frame while in HACK state (dt is real time).
    virtual Result update(float dt) = 0;

    // Draw the minigame UI in screen space (window already in default view).
    virtual void render(sf::RenderWindow& window, const sf::Font& font) const = 0;

    float timeLeft() const { return m_timeLeft; }

protected:
    float m_timeLeft = 0.f;
};

// Factory — allocates into caller-supplied storage to stay off-heap.
// Returns nullptr if tier is out of range.
IHackMinigame* createHackMinigame(int tier, void* storage, size_t storageSize);

// Storage sizes (keep in sync with Tier implementation sizeof).
constexpr size_t HACK_MINIGAME_STORAGE = 512;
