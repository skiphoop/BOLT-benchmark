# 🚀 KV Store Benchmark (CPU Baseline)

This repository contains a **CPU baseline benchmark** for evaluating a custom in-memory **Key-Value Store** implementation. It is designed to simulate a variety of access patterns with configurable read/write ratios, data sizes, and value sizes to test the performance under realistic workloads.

This benchmark serves as a **reference point for comparing CPU-only performance**.

---

## 📌 Purpose

The primary goal of this experiment is to:

- Evaluate CPU-based performance of a hash-based KV store
- Measure throughput and latency under varying key/value configurations
- Serve as a **baseline** for comparing with FPGA/HBM implementations (e.g., BOLT)

---

## 📁 Project Structure

| File | Description |
|------|-------------|
| `benchmark.cpp` | Main benchmark driver for evaluating KV store performance under different scenarios |
| `kvstore.hpp` | Header-only cache-aware KV store with prefetching and chaining |
| `benchmark_utils.hpp` | Timer, random data generator, operation generator, and hex dump tools |

---

## 🧪 Benchmark Scenarios

The benchmark runs three standard tests:

### ✅ Benchmark (i): Fixed Key Size, Fixed Value Size

- **Key count:** 1,000,000
- **Value size:** 8 bytes
- **Operations:** 2,000,000 mixed `get` and `update`
- **Purpose:** Measure baseline latency and throughput

---

### 📊 Benchmark (ii): Varying Keyspace, Fixed Value Size

- **Key counts:** 100K, 500K, 1M, 5M, 10M
- **Value size:** 8 bytes
- **Operations:** 2,000,000 per test
- **Purpose:** Study the scalability of hash table insertions and lookups

---

### 🧱 Benchmark (iii): Fixed Keyspace, Varying Value Size

- **Key count:** 1,000,000
- **Value sizes:** 8B, 16B, 32B, 64B, 128B, 256B
- **Operations:** 2,000,000 per test
- **Purpose:** Measure the impact of payload size on latency and performance

---

## 🛠️ Building

Compile with any C++17-compatible compiler (e.g., `g++`, `clang++`):

```bash
g++ -O3 -std=c++17 benchmark.cpp -o kv_benchmark
```
## ▶️ Running the Benchmark
You can pass an optional read ratio as a command-line argument:

```bash
./kv_benchmark [read_ratio]
```
.

## Example:
```bash
./kv_benchmark 0.7
→ Runs the benchmark with 70% get and 30% update operations.
```
If no value is provided, the default read ratio is 0.5.

## 💡 Sample Output
```sql
KV Store Benchmarks
===================
Benchmark (i): Fixed 1M data, 8-byte value size
Read/Write Ratio: 0.7 / 0.3
Initial insertion time for 1000000 entries: 128.301 milliseconds
Average insertion time per entry: 0.128 microseconds

Mixed operations results:
Total operations: 2000000
Get operations: 1400000 (70%)
Update operations: 600000 (30%)
Total time for mixed operations: 256.711 milliseconds
Average time per operation: 0.128 microseconds
Each benchmark prints detailed results or a summarized table depending on the mode.
```
