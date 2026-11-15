#include "../include/order_book.hpp"
#include "../include/ring_buffer.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace hft;

struct IncomingCommand {
    enum Type { ADD, CANCEL } cmd_type;
    OrderId id;
    Side side;
    Price price;
    Quantity qty;
};

int main() {
    constexpr size_t NUM_ORDERS = 500'000;
    OrderBook book(NUM_ORDERS + 1000);
    LockFreeSPSCQueue<IncomingCommand, 1048576> queue;

    std::mt19937_64 rng(42);
    std::uniform_int_distribution<Price> price_dist(9950, 10050);
    std::uniform_int_distribution<Quantity> qty_dist(1, 100);
    std::uniform_int_distribution<int> side_dist(0, 1);

    std::vector<IncomingCommand> commands;
    commands.reserve(NUM_ORDERS);

    for (size_t i = 1; i <= NUM_ORDERS; ++i) {
        commands.push_back(IncomingCommand{
            .cmd_type = IncomingCommand::ADD,
            .id = i,
            .side = (side_dist(rng) == 0) ? Side::BUY : Side::SELL,
            .price = price_dist(rng),
            .qty = qty_dist(rng)
        });
    }

    std::vector<uint64_t> latencies_ns;
    latencies_ns.reserve(NUM_ORDERS);

    std::cout << "[HFT Engine] Running benchmark with " << NUM_ORDERS << " orders...\n";

    auto total_start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < NUM_ORDERS; ++i) {
        queue.push(commands[i]);

        IncomingCommand cmd;
        auto t0 = std::chrono::high_resolution_clock::now();
        if (queue.pop(cmd)) {
            if (cmd.cmd_type == IncomingCommand::ADD) {
                book.add_limit_order(cmd.id, cmd.side, cmd.price, cmd.qty, i);
            } else {
                book.cancel_order(cmd.id);
            }
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        latencies_ns.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    }

    auto total_end = std::chrono::high_resolution_clock::now();
    double total_sec = std::chrono::duration<double>(total_end - total_start).count();

    std::sort(latencies_ns.begin(), latencies_ns.end());
    uint64_t p50 = latencies_ns[NUM_ORDERS * 0.50];
    uint64_t p99 = latencies_ns[NUM_ORDERS * 0.99];
    uint64_t p999 = latencies_ns[NUM_ORDERS * 0.999];
    double mean = std::accumulate(latencies_ns.begin(), latencies_ns.end(), 0.0) / NUM_ORDERS;

    std::cout << "--------------------------------------------------\n";
    std::cout << "Total Orders Processed : " << NUM_ORDERS << "\n";
    std::cout << "Total Trades Executed  : " << book.get_trades().size() << "\n";
    std::cout << "Throughput             : " << static_cast<size_t>(NUM_ORDERS / total_sec) << " orders/sec\n";
    std::cout << "Mean Latency           : " << mean << " ns\n";
    std::cout << "p50 Latency            : " << p50 << " ns\n";
    std::cout << "p99 Tail Latency       : " << p99 << " ns\n";
    std::cout << "p99.9 Tail Latency     : " << p999 << " ns\n";
    std::cout << "--------------------------------------------------\n";

    return 0;
}
