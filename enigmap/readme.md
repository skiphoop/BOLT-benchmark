# EnigMap Performance Benchmarks

This repository contains performance benchmark tests for the EnigMap project.

## Overview

The benchmark code is based on the original EnigMap test suite (`perf_ods_large.cpp`) but has been modified to comply with our memory constraints. 

1. **Query Performance** - The time required to perform key lookups in the ORAM-based data structure
2. **Initialization Performance** - The time required to insert data into the ORAM structure

## Test Cases

The benchmark consists of two main test cases:

### PointSearchPartial

Tests query performance by:
1. Creating an ORAM-based tree of a specific size
2. Inserting a small number of random key-value pairs
3. Measuring the time to perform multiple random queries
4. Testing across multiple ORAM sizes ranging from 2 to 262,144

### InsertionsPartial

Tests insertion performance by:
1. Creating an ORAM-based tree of a specific size
2. Measuring the time to insert random key-value pairs
3. Testing across multiple ORAM sizes ranging from 2 to 262,144

## Important Modifications

We made minor edits to the original EnigMap code to control the data size as we encountered program failures when sizes exceeded approximately 270,000. Our modifications ensure all test cases use input sizes within this limit.

## How to Run

To run these benchmarks:

1. Replace the original `perf_ods_large.cpp` test file in the EnigMap project with our modified file
2. Compile the code using the same compilation steps as specified in the EnigMap project
3. Run the benchmark using:
   ```
   ./build/tests/test_large_perf
   ```


## Benchmark Results

The benchmark outputs detailed reports for each test case, including:
- Size of the ORAM structure
- Maximum tree depth
- Number of nodes
- ORAM parameters (N, L, Z)
- Number of operations performed
- Total execution time
- Per-operation time in microseconds

The results are saved in the file `enigmap_benchmark.log`