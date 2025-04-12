# Benchmark Repository Overview

This repository contains a prototype implementation of BOLT—a bandwidth-optimized, lightning-fast Oblivious Map—along with benchmarking code for performance comparisons. Each subfolder corresponds to a specific experiment or artifact test code. Please refer to the individual folders for more detailed instructions on how to run each benchmark. We have provided readme in each folder.

## ⚠️ Notice on Benchmark Code Reuse

Our benchmarking framework reuses and builds upon the original benchmark or test code provided by the respective projects referenced in this repository. Only minor modifications were made to adapt the experiments to our testing scenarios—for example, incorporating YCSB-like workloads, using pre-loaded datasets to reach specific sizes, or aligning with our evaluation metrics. We acknowledge and appreciate the contributions of the original authors.

## 📁 Folder Structure

- **`baseline/`**  
  Contains the CPU baseline experiments referenced in our paper.

- **`engimap/`**  
  Contains experiments based on the artifact [Enigmap repository](https://github.com/odslib/EnigMap) from the Enigmap paper[Paper Link](https://www.usenix.org/system/files/usenixsecurity23-tinoco.pdf).

- **`facebook/`**  
  Contains the results and artifacts from the Facebook repository.
  [Facebook ORAM repository](https://github.com/facebook/oram)

- **`h2o2ram/`**  
  Contains experiments based on the artifact [H2O2RAM repository](https://github.com/55199789/H2O2RAM) from the H2O2RAM paper [Paper Link](https://doi.org/10.48550/arXiv.2409.07167).

- **`BOLT/`**  
  Contains all the experiments referenced in our paper under the BOLT framework as well as the FPGA baseline.


## Additional artifacts

- **`exp_validation/`**  
  This folder contains validation experiments for the analytical upper bounds proposed in our paper regarding the bin load, and stash sizes.

## 🚀 Usage

To get started with a specific experiment:

```bash
cd ./$Folder_name$
```
And follow the instructions inside the specific folder