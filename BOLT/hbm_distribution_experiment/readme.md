# HBM Distribution Experiment

This project explores the performance and correctness of BOLT using various distributions of HBM (High Bandwidth Memory) capacity. It simulates different scenarios where HBM memory capacity varies, to understand its impact on the latency on key-value store operations.

## 📁 Folder Structure
<pre lang="markdown">
text hbm_distribution_experiment/ 
├── 1_percent_hbm/ # Simulates very limited HBM capacity (1%) 
├── 20_percent_hbm/ # Simulates small HBM capacity (20%) 
├── 50_percent_hbm/ # Simulates medium HBM capacity (50%)
├── baseline/ # FPGA baseline (100% HBM and no oblivious primitives) 
└── README.md # This file  </pre>

Each subfolder is used to test how HBM usage impacts the system's performance and behavior under different capacity constraints. Currently, the setup for **1% HBM** (`1_percent_hbm/`) is fully implemented and serves as a reference.



## ⚙️ Requirements

- **FPGA Target**: Xilinx U55C
- **XRT**: Xilinx Runtime installed (e.g., `/opt/xilinx/xrt`)
- **Vitis**: Xilinx Vitis 2024.1
- **C++17**: Compiler support for C++17 standard

Make sure XRT and Vitis environment variables are properly sourced before running.

```bash
source /opt/xilinx/xrt/setup.sh
source /tools/Xilinx/Vitis/2024.1/settings64.sh
```
## 🚀 How to Build and Run (Example: 1% HBM)
Navigate to the folder:

```bash
cd hbm_distribution_experiment/1_percent_hbm
```
Build the host code:

```bash
./run.sh
```
This compiles host.cpp into an executable host using the XRT libraries.

Run the host program with the target .xclbin file:

```bash
./host load_balance_1d_array_1_percent_hbm_8_value_size.xclbin
```


## 📄 Files

- **`host.cpp`**:  
  - Generates distributed key-value tuples
  - Initializes HBM and host memory based on ratio
  - Generates uniformally distributed GET/PUT commands
  - Runs FPGA kernels (init_kernel, chain_kernel)
- **`run.sh`**:  
  - Compile host.cpp using correct XRT paths and flags
  - Launch host binary with specific .xclbin target

## Sample Terminal Output
```bash
3
Start testing 100% HBM
Init tuples size is 1000000
The value size is 8
Finish to generate bos
HBM count is 1000000
Host count is 0
Insert error count is 0
The hbm_key_to_index size is 1000000
The host_key_to_index size is 0
The init time is 1.30715s
Generated 5000 uniform commands:
  GET commands: 2548 (50.96%)
  PUT commands: 2452 (49.04%)
  HBM access: 5000 (100%)
  Host access: 0 (0%)
Command size is 5000
total time = 0.0131507s
AVERAGE LATENCY = 2.63015e-06s
```


