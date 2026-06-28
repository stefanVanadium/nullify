#pragma once
#include <functional>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>
#include <algorithm>
#include <cstdint>

// Typed event bus. Callbacks stored in pre-allocated vectors — no heap in emit().
// Usage:
//   EventBus::emit(MyEvent{ ... });
//   auto handle = EventBus::on<MyEvent>([](const MyEvent& e) { ... });

// ---- Event types (define here or in feature headers) ----
struct PlayerStateChangedEvent {
    uint32_t entityId;
    int      oldState;
    int      newState;
};

struct EnemyDiedEvent {
    uint32_t entityId;
    float    x, y;
};

struct HackSuccessEvent {
    uint32_t targetId;
};

struct AlertLevelChangedEvent {
    int oldLevel;
    int newLevel;
};

// ---- Bus implementation ----

class EventBus {
public:
    using Handle = size_t;

    template<typename EventT>
    static Handle on(std::function<void(const EventT&)> callback) {
        auto& subs = getSubs<EventT>();
        Handle h = s_nextHandle++;
        subs.push_back({h, std::move(callback)});
        return h;
    }

    template<typename EventT>
    static void emit(const EventT& event) {
        // No heap allocation — iterates existing vector
        for (auto& sub : getSubs<EventT>())
            sub.callback(event);
    }

    template<typename EventT>
    static void unsubscribe(Handle h) {
        auto& subs = getSubs<EventT>();
        subs.erase(std::remove_if(subs.begin(), subs.end(),
            [h](const auto& s) { return s.handle == h; }), subs.end());
    }

    // Call on state transitions to drop stale subscriptions
    static void clearAll();

private:
    template<typename EventT>
    struct Subscription {
        Handle handle;
        std::function<void(const EventT&)> callback;
    };

    template<typename EventT>
    static std::vector<Subscription<EventT>>& getSubs() {
        static std::vector<Subscription<EventT>> subs;
        return subs;
    }

    static inline Handle s_nextHandle = 0;
};
