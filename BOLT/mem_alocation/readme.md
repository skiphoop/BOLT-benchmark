# Memory Cost Analysis Tool for Key-Value Storage

This document introduces a C++ program that calculates memory cost estimates for a hash-based key-value storage system using a set of customizable parameters.

## 💡 Purpose

The program models the memory footprint of different components in a storage system and estimates how efficient the memory usage is, compared to the raw data size. It helps system designers understand overhead from data structures like hash tables, eviction buffers, and structured storage pages.

---

## ⚙️ Input Parameters

| Parameter | Description                              | Default Value |
|----------:|------------------------------------------|:--------------|
| `N`       | Number of data items                     | 1,000,000     |
| `c`       | Ratio of number of bins to data items (N/B) | 8.0        |
| `alpha`   | Ratio of `K/B` and `M/B` (key/value split) | 0.2        |
| `V`       | Size of value in bytes                   | 8 bytes       |

Users are prompted to use custom values after the initial default calculation.

---

## 📊 Output Breakdown

The program prints out:

- Derived Parameters:
  - `B` (number of bins)
  - `K`, `M` (split of bins)
  - `l_max` (maximum allowed load per bin)
  - `log2log2N` (used for hash table depth)

- Memory Cost of:
  1. **Hash Table**
  2. **Value Store Space**
  3. **Eviction Buffer Reverse Index**
  4. **M Pages**

- **Total memory cost**
- **Raw data size**
- **Memory usage ratio** (Total memory / Raw data)

Each component is printed in both **bytes** and **megabytes**.

---

## 🧮 Formula Highlights

- `B = N / c`
- `K = α * B`, `M = B - K`
- `Hash Table Size = N * log2log2N * 12`
- `Value Store Size = (K * c + l_max * sqrt(K * log(N))) * V`
- `Eviction Buffer Size = B * 4`
- `M Pages = M * (8 + l_max) * (4 + V)`

---

## 🏁 How to Run

Compile the code using any modern C++ compiler:

```bash
g++ -o memory_cost memory_cost.cpp -lm
./memory_cost
