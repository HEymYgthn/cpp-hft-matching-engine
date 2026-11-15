#pragma once
#include <vector>
#include <cstddef>
#include <stdexcept>

namespace hft {

template <typename T>
class ObjectPool {
private:
    std::vector<T> pool_;
    std::vector<T*> free_list_;

public:
    explicit ObjectPool(size_t capacity) : pool_(capacity) {
        free_list_.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            free_list_.push_back(&pool_[capacity - 1 - i]);
        }
    }

    [[nodiscard]] T* acquire() noexcept {
        if (free_list_.empty()) [[unlikely]] {
            return nullptr;
        }
        T* obj = free_list_.back();
        free_list_.pop_back();
        return obj;
    }

    void release(T* obj) noexcept {
        if (obj) [[likely]] {
            *obj = T{};
            free_list_.push_back(obj);
        }
    }

    [[nodiscard]] size_t available() const noexcept {
        return free_list_.size();
    }
};

} // namespace hft
