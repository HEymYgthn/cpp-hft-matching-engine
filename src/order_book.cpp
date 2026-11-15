#include "../include/order_book.hpp"
#include <algorithm>

namespace hft {

bool OrderBook::add_limit_order(OrderId id, Side side, Price price, Quantity qty, Timestamp ts) {
    if (qty == 0 || order_lookup_.contains(id)) [[unlikely]] {
        return false;
    }

    Order* order = order_pool_.acquire();
    if (!order) [[unlikely]] {
        return false;
    }

    order->id = id;
    order->side = side;
    order->price = price;
    order->quantity = qty;
    order->filled_quantity = 0;
    order->status = OrderStatus::NEW;
    order->timestamp = ts;

    if (side == Side::BUY) {
        match_buy_order(order, ts);
        if (order->remaining() > 0) {
            bids_[price].price = price;
            bids_[price].append(order);
            order_lookup_[id] = order;
        } else {
            order_pool_.release(order);
        }
    } else {
        match_sell_order(order, ts);
        if (order->remaining() > 0) {
            asks_[price].price = price;
            asks_[price].append(order);
            order_lookup_[id] = order;
        } else {
            order_pool_.release(order);
        }
    }
    return true;
}

void OrderBook::match_buy_order(Order* taker, Timestamp ts) {
    while (taker->remaining() > 0 && !asks_.empty()) {
        auto best_ask_it = asks_.begin();
        if (best_ask_it->first > taker->price) {
            break;
        }

        PriceLevel& level = best_ask_it->second;
        while (taker->remaining() > 0 && !level.empty()) {
            Order* maker = level.head;
            Quantity match_qty = std::min(taker->remaining(), maker->remaining());

            taker->filled_quantity += match_qty;
            maker->filled_quantity += match_qty;
            level.total_quantity -= match_qty;

            trades_.push_back(Trade{
                .maker_order_id = maker->id,
                .taker_order_id = taker->id,
                .price = maker->price,
                .quantity = match_qty,
                .timestamp = ts
            });

            if (maker->remaining() == 0) {
                maker->status = OrderStatus::FILLED;
                level.remove(maker);
                order_lookup_.erase(maker->id);
                order_pool_.release(maker);
            } else {
                maker->status = OrderStatus::PARTIALLY_FILLED;
            }
        }

        if (level.empty()) {
            asks_.erase(best_ask_it);
        }
    }
    taker->status = (taker->remaining() == 0) ? OrderStatus::FILLED :
                    (taker->filled_quantity > 0 ? OrderStatus::PARTIALLY_FILLED : OrderStatus::NEW);
}

void OrderBook::match_sell_order(Order* taker, Timestamp ts) {
    while (taker->remaining() > 0 && !bids_.empty()) {
        auto best_bid_it = bids_.begin();
        if (best_bid_it->first < taker->price) {
            break;
        }

        PriceLevel& level = best_bid_it->second;
        while (taker->remaining() > 0 && !level.empty()) {
            Order* maker = level.head;
            Quantity match_qty = std::min(taker->remaining(), maker->remaining());

            taker->filled_quantity += match_qty;
            maker->filled_quantity += match_qty;
            level.total_quantity -= match_qty;

            trades_.push_back(Trade{
                .maker_order_id = maker->id,
                .taker_order_id = taker->id,
                .price = maker->price,
                .quantity = match_qty,
                .timestamp = ts
            });

            if (maker->remaining() == 0) {
                maker->status = OrderStatus::FILLED;
                level.remove(maker);
                order_lookup_.erase(maker->id);
                order_pool_.release(maker);
            } else {
                maker->status = OrderStatus::PARTIALLY_FILLED;
            }
        }

        if (level.empty()) {
            bids_.erase(best_bid_it);
        }
    }
    taker->status = (taker->remaining() == 0) ? OrderStatus::FILLED :
                    (taker->filled_quantity > 0 ? OrderStatus::PARTIALLY_FILLED : OrderStatus::NEW);
}

bool OrderBook::cancel_order(OrderId id) {
    auto it = order_lookup_.find(id);
    if (it == order_lookup_.end()) [[unlikely]] {
        return false;
    }

    Order* order = it->second;
    if (order->side == Side::BUY) {
        auto level_it = bids_.find(order->price);
        if (level_it != bids_.end()) {
            level_it->second.remove(order);
            if (level_it->second.empty()) {
                bids_.erase(level_it);
            }
        }
    } else {
        auto level_it = asks_.find(order->price);
        if (level_it != asks_.end()) {
            level_it->second.remove(order);
            if (level_it->second.empty()) {
                asks_.erase(level_it);
            }
        }
    }

    order_lookup_.erase(it);
    order_pool_.release(order);
    return true;
}

} // namespace hft
