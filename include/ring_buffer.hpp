#pragma once
#include <atomic>
#include <cstddef>
#include <new>
#include <vector>

namespace hft {

#ifdef __cpp_lib_hardware_interference_size
    constexpr size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
    constexpr size_t CACHE_LINE_SIZE = 64;
#endif

template <typename T, size_t Capacity>
class LockFreeSPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");

private:
    static constexpr size_t MASK = Capacity - 1;
    std::vector<T> buffer_;

    alignas(CACHE_LINE_SIZE) std::atomic<size_t> head_{0};
    alignas(CACHE_LINE_SIZE) std::atomic<size_t> tail_{0};

public:
    LockFreeSPSCQueue() : buffer_(Capacity) {}

    bool push(const T& item) noexcept {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        const size_t next_head = (current_head + 1) & MASK;

        if (next_head == tail_.load(std::memory_order_acquire)) [[unlikely]] {
            return false;
        }

        buffer_[current_head] = item;
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    bool pop(T& item) noexcept {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);

        if (current_tail == head_.load(std::memory_order_acquire)) [[unlikely]] {
            return false;
        }

        item = buffer_[current_tail];
        tail_.store((current_tail + 1) & MASK, std::memory_order_release);
        return true;
    }
};

} // namespace hft
