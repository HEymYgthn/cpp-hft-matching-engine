#pragma once
#include <cstdint>
#include <limits>

namespace hft {

using OrderId = uint64_t;
using Price = uint64_t;
using Quantity = uint32_t;
using Timestamp = uint64_t;

enum class Side : uint8_t {
    BUY = 0,
    SELL = 1
};

enum class OrderStatus : uint8_t {
    NEW,
    PARTIALLY_FILLED,
    FILLED,
    CANCELED,
    REJECTED
};

struct alignas(64) Order {
    OrderId id{0};
    Price price{0};
    Quantity quantity{0};
    Quantity filled_quantity{0};
    Side side{Side::BUY};
    OrderStatus status{OrderStatus::NEW};
    Timestamp timestamp{0};

    Order* prev{nullptr};
    Order* next{nullptr};

    [[nodiscard]] Quantity remaining() const noexcept {
        return quantity - filled_quantity;
    }
};

struct Trade {
    OrderId maker_order_id;
    OrderId taker_order_id;
    Price price;
    Quantity quantity;
    Timestamp timestamp;
};

} // namespace hft
