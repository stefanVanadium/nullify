#include "EventBus.h"

// clearAll() resets all static subscription vectors via explicit template instantiation
void EventBus::clearAll() {
    getSubs<PlayerStateChangedEvent>().clear();
    getSubs<EnemyDiedEvent>().clear();
    getSubs<HackSuccessEvent>().clear();
    getSubs<AlertLevelChangedEvent>().clear();
}
