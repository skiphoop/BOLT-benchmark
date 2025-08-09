# Facebook ORAM Benchmark

This repository contains performance benchmark results for the [Facebook ORAM](https://github.com/facebook/oram) project.  

## Overview

The original Facebook ORAM repository already provides a rigorous performance benchmark in [`/benches/benchmark.rs`](https://github.com/facebook/oram/tree/main/benches).  
We follow the same benchmark logic, adjusting the data size and block size as needed to evaluate different settings.  

## Running the Benchmark

To reproduce our results, please follow the same instructions provided in the [official benchmark guide](https://github.com/facebook/oram/tree/main/benches).  

## Benchmark Results

This repository contains our benchmark outputs in this folder:  

- **`var_data_sz.log`** – Benchmark of initialization cost and query cost as the dataset size increases from **100K** to **10M** entries.  
- **`var_value_sz.log`** – Benchmark of initialization and query time for a fixed **1M** entries, varying the value size from **8B** to **256B**.  
