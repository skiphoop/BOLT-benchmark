# Data Set Size Experiment

This project explores the impact of dataset size on the performance of BOLT.

The experiment compares two groups:

- **BOLT (20% HBM)**: 20% of key-value pairs are placed in HBM; the rest are stored in host memory.
- **FPGA baseline (100% HBM)**: All key-value pairs are stored in HBM and it is used as the baseline, because there is hardly no interaction between DRAM and HBM using PCIe and no obvlivious primitives. 

---

## 📁 Folder Structure

```
data_set_size_experiment/
├── 20_percent_hbm/
│   ├── host.cpp
|   ├── kernel file
│   └── run.sh
├── baseline
│   ├── host.cpp
|   ├── kernel file
│   └── run.sh
```


---

## 📂 Folder Descriptions

### `20_percent_hbm/`

This folder contains the code for the **20% HBM** experiment.

- **`host.cpp`**:  
  - Initializes the data structures.
  - Randomly assigns 20% of the key-value pairs to HBM and the remaining 80% to host memory pages.
  - Prepares command packets (GET/PUT), runs `init_kernel` and `chain_kernel`, and measures latency.
- **`run.sh`**:  
  - **Source Vitis and XRT** 
  - Compiles `host.cpp`.
  - Executes the program with the pre-compiled FPGA binary:
    ```
    load_balance_1d_array_20_percent_hbm_8_value_size.xclbin
    ```

### `100_percent_hbm/`

This folder contains the code for the **100% HBM** experiment.

- **`host.cpp`**:  
  - Assigns **all** key-value pairs into HBM.
  - No host memory is used for storing data.
  - Runs initialization and execution kernels similarly to the 20% case.
- **`run.sh`**:  
  - Compiles and runs `host.cpp` using the binary:
    ```
    load_balance_1d_array_full_hbm_8_value_size.xclbin
    ```

---

## 🛠 How to Run the Experiments

### ✅ Prerequisites

- FPGA hardware (e.g., Xilinx U55C)
- Xilinx Vitis 2024.1 and XRT properly installed
- Compiled FPGA binaries:
  - `load_balance_1d_array_20_percent_hbm_8_value_size.xclbin`
  - `load_balance_1d_array_full_hbm_8_value_size.xclbin`

### ▶️ Run Instructions
Run 20 percent hbm experiments:
```bash
cd data_set_size_experiment/20_percent_hbm
./run.sh
```
Run fpga baseline:
```bash
cd data_set_size_experiment/100_percent_hbm
./run.sh
```
### Sample Termianl Output 
```bash
-----------
3
Start testing 20% HBM
Init tuples size is 5000000
The value size is 8
Finish to generate bos
HBM count is 1322552
Host count is 5277448
Insert error count is 0
The hbm_key_to_index size is 1002233
The host_key_to_index size is 3997767
The init time is 4.20469s
Generated 5000 uniform commands:
  GET commands: 2549 (50.98%)
  PUT commands: 2451 (49.02%)
  HBM access: 967 (19.34%)
  Host access: 4033 (80.66%)
Command size is 5000
total time = 0.0289754s
AVERAGE LATENCY = 5.79508e-06s
-----------
```



