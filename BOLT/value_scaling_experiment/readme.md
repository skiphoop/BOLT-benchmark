# Value Scaling Experiment for BOLT System

This repository contains the experimental results for analyzing the **impact of value size on query latency and query slowdown in the BOLT system**. The evaluation is structured into two groups:

- `20_percent_hbm/` BOLT
- `baseline/` FPGA baseline

## 🧪 Experiment Overview

The goal of this experiment is to investigate how the **value size** (ranging from 16B to 256B) affects the query latency in the BOLT system. And the full hbm group is FPGA baseline without any host interactions and oblivious primitives. 

## 📁 Folder Structure

<pre> value_scaling_experiment/ 
├── 20_percent_hbm/ │ 
    ├── value_size_16/ │ 
    ├── value_size_32/ │ 
    ├── value_size_64/ │ 
    ├── value_size_128/│ 
    └── value_size_256/ 
├── baseline/ │ 
    ├── value_size_16/ │ 
    ├── value_size_32/ │ 
    ├── value_size_64/ │ 
    ├── value_size_128/ │ 
    └── value_size_256/ </pre>



Each folder contains:

- **`host.cpp`**:  
  - Generates distributed key-value tuples
  - Initializes HBM and host memory based on ratio
  - Generates uniformally distributed GET/PUT commands
  - Runs FPGA kernels (init_kernel, chain_kernel)
- **`run.sh`**:  
  - Compile host.cpp using correct XRT paths and flags
  - Launch host binary with specific .xclbin target


## Terminal Output
```bash
3
Start testing 20% HBM
Init tuples size is 1000000
The value size is 64
The block size is 4
Finish to generate bos
HBM count is 200143
Host count is 799857
Insert error count is 0
The hbm_key_to_index size is 200143
The host_key_to_index size is 799857
The init time is 2.09948s
Generated 5000 uniform commands:
  GET commands: 2487 (49.74%)
  PUT commands: 2513 (50.26%)
  HBM access: 1020 (20.4%)
  Host access: 3980 (79.6%)
Command size is 5000
total time = 0.0349336s
AVERAGE LATENCY = 6.98673e-06s
```