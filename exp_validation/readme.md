# Analytical Bound Validation Experiment

This is a validation experiment for the theoretical upper bounds proposed in the paper on dynamic bin allocation between HBM (High Bandwidth Memory) and host memory.

## Overview

The simulation validates several key theoretical bounds:
- Maximum bin load
- Maximum HBM region load
- Maximum queue size
- Maximum HBM usage (HBM load + queue size)

The implementation simulates a dynamic binning system where:
- Data elements are distributed across bins in HBM and host memory
- On each access, elements randomly move between two bins based on load balancing
- A queue system manages evictions between HBM and host memory

## Configuration

The code includes several configurable parameters:
- `N`: Number of data elements (default: 2^20)
- `DIVISOR`: Configurable bin division factor (default: 8)
- `ALPHA`: Partition coefficient determining HBM fraction (default: 0.5)
- `NUM_ACCESSES`: Total number of simulated accesses (default: 1 billion)

## Compilation

Compile the simulation with:

```bash
g++ -O3 -std=c++17 hbm_validation.cpp -o hbm_validation
```

For additional performance, you can use compiler optimizations:

```bash
g++ -O3 -march=native -flto -std=c++17 hbm_validation.cpp -o hbm_validation
```

## Running the Validation

Execute the compiled binary:

```bash
./hbm_validation
```

To modify parameters, edit the constants at the top of the source file:

```cpp
constexpr uint32_t N = 1 << 20;  // Number of data elements
constexpr uint32_t DIVISOR = 8;  // Bin division factor (2, 4, or 8)
constexpr double ALPHA = 0.5;    // Partition coefficient
```

## Output

The simulation produces:
1. Initial simulation parameters and theoretical bounds
2. Progress updates at regular intervals
3. Final statistics comparing experimental results with theoretical bounds
4. Performance metrics including max bin load, HBM load, queue size, and overall HBM usage

