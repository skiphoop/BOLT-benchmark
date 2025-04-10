# Accuracy Test for Load-Balanced Key-Value Store on FPGA (HBM + Host Memory)
This test evaluates the correctness of the BOLT accelerator. We provide two main versions of the accuracy test: one for small values (8 bytes) and another for large values (64 bytes), both designed to verify the logical correctness of our implementation. For other value sizes, a generalized version of run_accuracy_test() is already available in the key_scaling_experiment folder.


---

## ⚙️ Overview

This test suite performs an **accuracy validation** of BOLT. The store splits storage between **20% HBM (High Bandwidth Memory)** and **80% host memory pages**.

Each test follows these steps:

1. **Initialization**:
   - Key-value tuples are randomly generated.
   - Keys are inserted into HBM or host memory depending on a probability distribution.
   - Metadata structures like position maps and page tables are prepared.

2. **Command Generation**:
   - Sequences of  `GET` commands are generated using the inserted keys.
   - Two rounds of commands are run to test stability and memory persistence.

3. **Execution**:
   - Kernels `init_kernel` and `chain_kernel` are launched using XRT runtime.
   - Commands are sent, and responses are collected.

4. **Accuracy Check**:
   - Response values are compared against the original tuples.
   - Mismatches are logged and categorized (e.g., insertion errors vs. incorrect values).

---

## 📜 File Details

### `large_value.cpp`
- Uses **64-byte** values per key, split into multiple `Value_Block`s.
- Ideal for simulating **larger data entries** stored across HBM and host pages.
- Accuracy checks compare block-by-block using `memcmp()`.
- Repeats `GET` requests to ensure memory consistency.

### `small_value.cpp`
- Uses **8-byte** values per key, simpler layout.
- First round sends `PUT` commands, followed by `GET` requests.
- Focused on **lightweight performance testing** and memory hit accuracy.

### `run.sh`
- Compiles both programs using `g++` and links to XRT runtime.
- Sets necessary include and library paths.
- Executes both programs with corresponding `.xclbin` files.
- Drops system caches before running for consistency.

---

## 🧪 How to Use
```bash
sudo ./run.sh
```



