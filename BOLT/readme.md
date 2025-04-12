# ⚡ BOLT prototypes and Experiments

This repository contains all the experiments conducted for our **BOLT (Bandwidth-Optimized Lightning-Fast Oblivious Map)**. Each subfolder represents a different type of experiment discussed in our paper and with some are additional tests (e.g., accuracy validations). The FPGA baselines are also provided (which are the groups that uses full hbm for KVS and without any oblivious primitives.)

## 📁 Folder Structure

- `accuracy_test/`:  
  Tests the **memory remapping logic** in our algorithm, showing how (Key, Value) pairs are dynamically redirected into different memory regions such as:
  - `host` (DRAM)
  - `kv_cache` (HBM)
  - `eviction_buffer` (HBM)

- `dataset_size_experiment/`:  
  Evaluates **BOLT latency trends** across varying dataset sizes to understand scalability.

- `value_scaling_experiment/`:  
  Investigates how **value size impacts the efficiency** of BOLT.

- `hbm_distribution_experiment/`:  
  Analyzes the **influence of data placement** (between DRAM and HBM) on BOLT’s performance.

- `mem_allocation_experiment/`:  
  Estimates the **memory consumption** of a hash map–based key-value storage system under different configurations.

## ⚙️ Prototype Overview

All FPGA kernels used in these experiments are **pre-compiled into `.xclbin` binaries** and are ready for deployment on **Xilinx U55C** FPGAs. No additional HLS or synthesis steps are required for running the benchmarks.

To use the kernels:
- Please ensure that the U55C FPGA card is correctly installed and configured.
- Follow the [official U55C setup guide](https://www.xilinx.com/products/boards-and-kits/alveo/u55c.html) provided by AMD/Xilinx to initialize and validate your device.
- All benchmarks have been tested on **PCIe Gen3 x16** interfaces for consistent and high-throughput communication with the host.

## 🚀 Usage

To run a specific experiment:

```bash
cd ./<folder_name>
