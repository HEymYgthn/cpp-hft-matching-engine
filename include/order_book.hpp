#pragma once
#include "types.hpp"
#include "memory_pool.hpp"
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>

namespace hft {

struct PriceLevel {
    Price price{0};
    Quantity total_quantity{0};
    Order* head{nullptr};
    Order* tail{nullptr};

    void append(Order* order) noexcept {
        order->prev = tail;
        order->next = nullptr;
        if (tail) {
            tail->next = order;
        } else {
            head = order;
        }
        tail = order;
        total_quantity += order->remaining();
    }

    void remove(Order* order) noexcept {
        if (order->prev) {
            order->prev->next = order->next;
        } else {
            head = order->next;
        }
        if (order->next) {
            order->next->prev = order->prev;
        } else {
            tail = order->prev;
        }
        total_quantity -= order->remaining();
        order->prev = nullptr;
        order->next = nullptr;
    }

    [[nodiscard]] bool empty() const noexcept {
        return head == nullptr;
    }
};

class OrderBook {
private:
    ObjectPool<Order> order_pool_;
    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel, std::less<Price>> asks_;
    std::unordered_map<OrderId, Order*> order_lookup_;
    std::vector<Trade> trades_;

    void match_buy_order(Order* taker, Timestamp ts);
    void match_sell_order(Order* taker, Timestamp ts);

public:
    explicit OrderBook(size_t max_orders = 1'000'000)
        : order_pool_(max_orders) {
        order_lookup_.reserve(max_orders);
        trades_.reserve(100'000);
    }

    bool add_limit_order(OrderId id, Side side, Price price, Quantity qty, Timestamp ts);
    bool cancel_order(OrderId id);

    [[nodiscard]] Price best_bid() const noexcept {
        return bids_.empty() ? 0 : bids_.begin()->first;
    }

    [[nodiscard]] Price best_ask() const noexcept {
        return asks_.empty() ? std::numeric_limits<Price>::max() : asks_.begin()->first;
    }

    [[nodiscard]] const std::vector<Trade>& get_trades() const noexcept {
        return trades_;
    }
};

} // namespace hft
