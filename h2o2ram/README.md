# H2O2RAM Benchmarking Extension

This repository extends the [H2O2RAM project](https://github.com/55199789/H2O2RAM) with additional benchmarking capabilities for the ORAM-based hashmap implementation.

## Overview

We have added five benchmark files to evaluate the performance of the OMap implementation:

1. `benchmark_omap_init.cpp`: Measures initialization time for an OMap of a given size, including both hash planning and OHash build time.
2. `benchmark_omap.cpp`: Tests an OMap with a fixed size under YCSB-like workloads (50% read, 50% write operations).
3. `benchmark_omap_raw.cpp`: A duplicate of the original benchmark for comparison, which creates an empty OHashMap and measures the time to insert a specified number of elements.
4. `benchmark_omap_init_vsz.cpp`: Tests initialization performance with different value sizes (4B to 4KB) for a fixed number of elements (1M).
5. `benchmark_omap_vsz.cpp`: Similar to the init value size benchmark but designed to measure query performance with different value sizes.

## Installation

### Prerequisites

- C++ compiler with C++17 support
- CMake 3.10 or higher
- Git

### Setup Steps

1. Clone the original H2O2RAM repository:
   ```bash
   git clone https://github.com/55199789/H2O2RAM.git
   cd H2O2RAM
   ```

2. Add the benchmark files (.cpp) from this repository to the benchmarks folder of H2O2RAM project.

3. Build the project according to the same instruction as H2O2RAM:
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make
   ```

## Running the Benchmarks

Navigate to the `bin` directory after compilation:

```bash
cd ../bin
```

### Important Note

The program automatically saves cached parameters in the `/bin` folder for reuse in multiple computations. To ensure accurate performance measurements, clean these cached files before each benchmark run:

```bash
rm -r hash*
```

### Benchmark Commands

#### OMap Initialization Benchmarks

Test initialization time for different OMap sizes:

```bash
# Specify an OMap size, e.g. 1,048,576 elements (1M) and run tests
./ORAMBenchmark --benchmark_filter=OMapRInit/1048576/ --benchmark_format=json --benchmark_out="h2o2ram_init_1m.json" --benchmark_out_format=json

# Run auto benchmark to test data from 2^16 to 2^23 (~10M)
./ORAMBenchmark --benchmark_filter=OMapRInit --benchmark_format=json --benchmark_out="h2o2ram_init.json" --benchmark_out_format=json
```

#### OMap YCSB-like Workload Benchmarks

Test query performance under mixed read/write workloads for different OMap sizes:

```bash
# Specify an OMap size, e.g. 1,048,576 elements (1M) and run tests
./ORAMBenchmark --benchmark_filter=OMapDataFixture/OMapR/1048576/ --benchmark_format=json --benchmark_out="h2o2ram_query_1m.json" --benchmark_out_format=json

# Run auto benchmark to test data from 2^16 to 2^23 (~10M)
./ORAMBenchmark --benchmark_filter=OMapDataFixture/OMapR/ --benchmark_format=json --benchmark_out="h2o2ram_query.json" --benchmark_out_format=json
```

#### Value Size Scaling Benchmarks

Test initialization and query performance with different value sizes (4B to 4KB) for a fixed dataset of 1M elements:

```bash
# Value size initialization benchmark (all sizes: 4B, 16B, 64B, 256B, 1KB, 4KB)
./ORAMBenchmark --benchmark_filter=ValueSizeInit --benchmark_format=json --benchmark_out="h2o2ram_value_size_init.json" --benchmark_out_format=json

# Run specific value size init benchmark, e.g., for 16B values
./ORAMBenchmark --benchmark_filter=ValueSizeInit_16B --benchmark_format=json --benchmark_out="h2o2ram_value_size_init_16B.json" --benchmark_out_format=json

# Value size query benchmark (all sizes: 4B, 16B, 64B, 256B, 1KB)
./ORAMBenchmark --benchmark_filter=ValueSize --benchmark_format=json --benchmark_out="h2o2ram_value_size_query.json" --benchmark_out_format=json
```

## Benchmark Details

### benchmark_omap_init.cpp

This benchmark measures the time required to initialize an OMap of a specific size. The initialization process includes:
- Hash function planning
- OHash structure building

### benchmark_omap.cpp

This benchmark creates an OMap with a fixed size and then runs YCSB-like workloads with:
- 50% read operations
- 50% write operations

It helps evaluate the performance of the OMap under realistic mixed workloads.

### benchmark_omap_raw.cpp

This is a duplicate of the original benchmark for comparison purposes. It:
1. Creates an empty OHashMap
2. Inserts a given number of elements
3. Measures the total insertion time

### benchmark_omap_init_vsz.cpp

This benchmark measures initialization performance with varying value sizes:
- Tests with six different value sizes: 4B, 16B, 64B, 256B, 1KB, and 4KB
- Fixed dataset size of 1M elements
- Helps understand how value size impacts initialization performance
- Benchmark names follow the pattern `ValueSizeInit_XXB` or `ValueSizeInit_XKB`

### benchmark_omap_vsz.cpp

This benchmark tests query performance with different value sizes:
- Similar value size range as the initialization benchmark (4B to 1KB)
- Note: The 4KB size test is commented out in the source code
- Fixed dataset size of 1M elements
- Measures how value size affects read/write operation performance
- Benchmark names follow the pattern `ValueSize_XXB` or `ValueSize_XKB`

## Output

All benchmarks output results in JSON format to the specified output files, which can be used for further analysis and visualization.

## License

This project is licensed under the same terms as the original H2O2RAM project.
