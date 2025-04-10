#!/bin/bash
ulimit -s unlimited
# Set environment variables for Vitis
# source /tools/Xilinx/Vitis/2024.1/settings64.sh
# source /opt/xilinx/xrt/setup.sh

HOST_FILE="/home/yitoguo/Desktop/benchmark/OEE/scaling_experiment/20_percent_hbm/value_size_32/host_32.cpp"
PLATFORM="/opt/xilinx/platforms/xilinx_u55c_gen3x16_xdma_3_202210_1/xilinx_u55c_gen3x16_xdma_3_202210_1.xpfm"
XRT_INCLUDE_PATH="/opt/xilinx/xrt/include/"
XRT_LIB_PATH="/opt/xilinx/xrt/lib/"

echo "Building Exe file.."
g++ host_256.cpp -o host_256 \
    -I $XRT_INCLUDE_PATH \
    -L $XRT_LIB_PATH\
    -lxrt_coreutil -lxrt_core \
    -w \
    -Wl,-rpath,$XRT_LIB_PATH \
    -std=c++17 \
    -Wl,-z,stack-size=10048576000000000
# echo "Run SW Emulation"
# export XCL_EMULATION_MODE=hw_emu
# export XRT_PRINT_LEVEL=4
./host_256 load_balance_1d_array_100_percent_hbm_256_value_size.xclbin