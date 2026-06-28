#pragma once
#include <cstddef>
#include <cstdint>
#include <array>

// Fixed-size pool allocator. Zero heap allocation. O(1) alloc/free.
// On exhaustion, reuses the oldest allocated slot (circular overwrite).
template<typename T, size_t N>
class PoolAllocator {
public:
    PoolAllocator() {
        for (size_t i = 0; i < N; ++i)
            m_free[i] = &m_slots[i];
        m_freeCount = N;
        m_oldestIdx = 0;
    }

    T* allocate() {
        if (m_freeCount > 0) {
            return m_free[--m_freeCount];
        }
        // Exhaustion: reclaim oldest slot
        T* slot = &m_slots[m_oldestIdx];
        m_oldestIdx = (m_oldestIdx + 1) % N;
        return slot;
    }

    void free(T* ptr) {
        if (!ptr) return;
        if (m_freeCount < N)
            m_free[m_freeCount++] = ptr;
    }

    constexpr size_t capacity() const { return N; }

private:
    std::array<T, N>    m_slots;
    std::array<T*, N>   m_free;
    size_t              m_freeCount = 0;
    size_t              m_oldestIdx = 0;
};
