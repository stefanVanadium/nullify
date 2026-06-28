#include "HackMinigame.h"
#include "Tier1Sequence.h"
#include "Tier2Circuit.h"
#include "Tier3ICE.h"
#include <new>
#include <cstddef>
#include <ctime>

static_assert(sizeof(Tier1Sequence) <= HACK_MINIGAME_STORAGE, "Tier1Sequence exceeds storage budget");
static_assert(sizeof(Tier2Circuit)  <= HACK_MINIGAME_STORAGE, "Tier2Circuit exceeds storage budget");
static_assert(sizeof(Tier3ICE)      <= HACK_MINIGAME_STORAGE, "Tier3ICE exceeds storage budget");

static int nextSeed() {
    // Simple counter — ensures different sequence each call within a session
    static int s = static_cast<int>(std::time(nullptr));
    s = (s * 1103515245 + 12345) & 0x7FFFFFFF;
    return s;
}

IHackMinigame* createHackMinigame(int tier, void* storage, size_t storageSize) {
    if (storageSize < HACK_MINIGAME_STORAGE) return nullptr;
    int seed = nextSeed();
    switch (tier) {
        case 1: {
            auto* mg = new (storage) Tier1Sequence();
            mg->init(seed);
            return mg;
        }
        case 2: {
            auto* mg = new (storage) Tier2Circuit();
            mg->init(seed);
            return mg;
        }
        case 3: {
            auto* mg = new (storage) Tier3ICE();
            mg->init(seed);
            return mg;
        }
        default:
            return nullptr;
    }
}
