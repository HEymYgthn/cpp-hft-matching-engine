 Ultra Low-Latency C++ HFT Matching Engine & Order Book

A high-performance, low-latency limit order book and matching engine written in Modern C++ (C++20), engineered to eliminate tail latencies and optimize CPU cache utilization for high-frequency trading (HFT) environments.

 Architectural Highlights

- **Lock-Free SPSC Ring Buffer:** Wait-free Single-Producer Single-Consumer queues utilizing memory barriers (`std::atomic` with `memory_order_acquire`/`release`) to bypass kernel context-switch overhead.
- **Cache-Conscious Design:** Prevention of false sharing by aligning critical data structures to 64-byte CPU cache lines (`alignas(64)`), maximizing L1/L2 cache hit rates.
- **O(1) Order Matching & Book Management:** Optimized price-level lookups and order insertions to maintain predictable execution times under heavy throughput.
- **Zero Dynamic Allocation in Hot Path:** All memory is pre-allocated during initialization (`ObjectPool`) to eliminate garbage collection and heap allocation latency spikes.

 Performance Benchmarks (Live Cloud Hardware Results)

Tested on Linux (x86_64), GCC 13.3, optimized with `-O3 -march=native -flto`.

| Metric | Measured Performance |
| :--- | :--- |
| **Total Orders Processed** | 500,000 |
| **Total Trades Executed** | 389,107 |
| **Throughput** | **5,140,104 orders/sec** |
| **Mean Latency** | 162.5 ns |
| **p50 Latency (Median)** | **114 ns** |
| **p99 Tail Latency** | **790 ns** |
| **p99.9 Tail Latency** | **2,414 ns** |

## Build & Run Instructions

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/hft_engine