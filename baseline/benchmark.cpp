#include "kvstore.hpp"
#include "benchmark_utils.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <chrono>

// Constants for default benchmark parameters
const size_t DEFAULT_DATA_SIZE = 1000000; // 1M
const size_t DEFAULT_OPERATIONS = 2000000; // 2M
const double DEFAULT_READ_RATIO = 0.5;

// Warm-up function to ensure consistent system state
void warmupSystem() {
    // Simple computation to warm up the CPU
    volatile double sum = 0.0;
    for (int i = 0; i < 1000000; i++) {
        sum += i * 0.01;
    }
    
    // Small pause to allow system to stabilize
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

// Unified benchmark function that handles all test cases
template<typename K, size_t ValueSize>
void runBenchmark(
    size_t dataSize,
    double readRatio,
    size_t numOperations,
    bool printDetails = false,
    bool printHeader = false,
    bool printRow = false
) {
    // Warm up system before benchmark
    warmupSystem();
    
    // Create KV store with double the data size for better performance
    KVStore<K, ValueSize> kvStore(dataSize * 2);
    
    // Initialize random number generator with fixed seed for reproducibility
    std::mt19937 gen(42);
    
    // Initial data insertion
    if (printDetails) {
        std::cout << "Performing initial data insertion..." << std::endl;
    }
    
    Timer insertTimer;
    insertTimer.start();
    
    for (size_t i = 0; i < dataSize; i++) {
        auto value = generateRandomData<ValueSize>(gen);
        kvStore.insert(static_cast<K>(i), value);
    }
    
    double insertTime = insertTimer.elapsedMilliseconds();
    
    // Generate random operations
    auto operations = generateRandomOperations(numOperations, dataSize, readRatio);
    
    // Test mixed operations
    if (printDetails) {
        std::cout << "Testing mixed operations..." << std::endl;
    }
    
    Timer mixedTimer;
    mixedTimer.start();
    
    size_t gets = 0, updates = 0;
    for (const auto& op : operations) {
        if (op.first == 0) {
            auto result = kvStore.get(static_cast<K>(op.second));
            gets++;
        } else {
            auto newValue = generateRandomData<ValueSize>(gen);
            kvStore.update(static_cast<K>(op.second), newValue);
            updates++;
        }
    }
    
    double mixedTime = mixedTimer.elapsedMilliseconds();
    
    // Print table header if requested
    if (printHeader) {
        if (printRow) {
            std::cout << "| Data Size | Value Size | Insertion Time (ms) | Avg Insert (μs) | Mixed Ops Time (ms) | Avg Op Time (μs) |" << std::endl;
            std::cout << "|-----------|------------|---------------------|-----------------|---------------------|------------------|" << std::endl;
        } else {
            std::cout << std::fixed << std::setprecision(3);
            std::cout << "Initial insertion time for " << dataSize << " entries: " 
                    << insertTime << " milliseconds" << std::endl;
            std::cout << "Average insertion time per entry: " 
                    << insertTime * 1000.0 / dataSize << " microseconds" << std::endl;
            
            std::cout << "\nMixed operations results:" << std::endl;
            std::cout << "Total operations: " << numOperations << std::endl;
            std::cout << "Get operations: " << gets << " (" << (gets * 100.0 / numOperations) << "%)" << std::endl;
            std::cout << "Update operations: " << updates << " (" << (updates * 100.0 / numOperations) << "%)" << std::endl;
            std::cout << "Total time for mixed operations: " 
                    << mixedTime << " milliseconds" << std::endl;
            std::cout << "Average time per operation: " 
                    << mixedTime * 1000.0 / numOperations << " microseconds" << std::endl;
        }
    }
    
    // Print benchmark results as a table row if requested
    if (printRow) {
        std::cout << "| " << std::setw(9) << dataSize << " | "
                << std::setw(10) << ValueSize << " bytes | " 
                << std::setw(19) << insertTime << " | "
                << std::setw(16) << (insertTime * 1000.0 / dataSize) << " | "
                << std::setw(19) << mixedTime << " | "
                << std::setw(16) << (mixedTime * 1000.0 / numOperations) << " |" << std::endl;
    }
    
    // Print detailed results if requested
    if (printDetails && !printRow) {
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "Initial insertion time for " << dataSize << " entries: " 
                << insertTime << " milliseconds" << std::endl;
        std::cout << "Average insertion time per entry: " 
                << insertTime * 1000.0 / dataSize << " microseconds" << std::endl;
        
        std::cout << "\nMixed operations results:" << std::endl;
        std::cout << "Total operations: " << numOperations << std::endl;
        std::cout << "Get operations: " << gets << " (" << (gets * 100.0 / numOperations) << "%)" << std::endl;
        std::cout << "Update operations: " << updates << " (" << (updates * 100.0 / numOperations) << "%)" << std::endl;
        std::cout << "Total time for mixed operations: " 
                << mixedTime << " milliseconds" << std::endl;
        std::cout << "Average time per operation: " 
                << mixedTime * 1000.0 / numOperations << " microseconds" << std::endl;
        
        // Print sample values
        std::cout << "\nSample values:" << std::endl;
        for (int i = 0; i < 3; i++) {
            auto value = kvStore.get(i);
            std::cout << "Key " << i << ": 0x";
            printHexData(value);
            std::cout << std::endl;
        }
    }
}

// Benchmark (i): Fixed 1M data, fixed value size
template<typename K, size_t ValueSize>
void runFixedSizeBenchmark(double readRatio = DEFAULT_READ_RATIO, size_t numOperations = DEFAULT_OPERATIONS) {
    const size_t dataSize = DEFAULT_DATA_SIZE;
    
    std::cout << "\n==========================================================" << std::endl;
    std::cout << "Benchmark (i): Fixed 1M data, " << ValueSize << "-byte value size" << std::endl;
    std::cout << "Read/Write Ratio: " << readRatio << " / " << (1.0 - readRatio) << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    
    // Run the benchmark with detailed output
    runBenchmark<K, ValueSize>(dataSize, readRatio, numOperations, true, false, false);
}

// Benchmark (ii): Variable data size with fixed value size
template<typename K, size_t ValueSize>
void runVaryingDataSizeBenchmark(double readRatio = DEFAULT_READ_RATIO, size_t numOperations = DEFAULT_OPERATIONS) {
    std::vector<size_t> dataSizes = {100000, 500000, 1000000, 5000000, 10000000}; // 100K, 500K, 1M, 5M, 10M
    
    std::cout << "\n==========================================================" << std::endl;
    std::cout << "Benchmark (ii): Varying data size with " << ValueSize << "-byte value" << std::endl;
    std::cout << "Read/Write Ratio: " << readRatio << " / " << (1.0 - readRatio) << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    
    // Print the table header
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "| Data Size | Value Size | Insertion Time (ms) | Avg Insert (μs) | Mixed Ops Time (ms) | Avg Op Time (μs) |" << std::endl;
    std::cout << "|-----------|------------|---------------------|-----------------|---------------------|------------------|" << std::endl;
    
    // Run benchmarks for each data size
    for (const auto& dataSize : dataSizes) {
        // Use consistent operation count for fair comparison
        size_t adjustedOps = numOperations;
        
        // Run the benchmark with row output
        runBenchmark<K, ValueSize>(dataSize, readRatio, adjustedOps, false, false, true);
    }
}

// Benchmark (iii): Fixed data size with varying value sizes
template<typename K>
void runVaryingValueSizeBenchmark(double readRatio = DEFAULT_READ_RATIO, size_t numOperations = DEFAULT_OPERATIONS) {
    const size_t dataSize = DEFAULT_DATA_SIZE; // 1M
    
    std::cout << "\n==========================================================" << std::endl;
    std::cout << "Benchmark (iii): Fixed 1M data with varying value sizes" << std::endl;
    std::cout << "Read/Write Ratio: " << readRatio << " / " << (1.0 - readRatio) << std::endl;
    std::cout << "----------------------------------------------------------" << std::endl;
    
    // Print the table header
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "| Data Size | Value Size | Insertion Time (ms) | Avg Insert (μs) | Mixed Ops Time (ms) | Avg Op Time (μs) |" << std::endl;
    std::cout << "|-----------|------------|---------------------|-----------------|---------------------|------------------|" << std::endl;
    
    // Run benchmarks for different value sizes
    runBenchmark<K, 8>(dataSize, readRatio, numOperations, false, false, true);
    runBenchmark<K, 16>(dataSize, readRatio, numOperations, false, false, true);
    runBenchmark<K, 32>(dataSize, readRatio, numOperations, false, false, true);
    runBenchmark<K, 64>(dataSize, readRatio, numOperations, false, false, true);
    runBenchmark<K, 128>(dataSize, readRatio, numOperations, false, false, true);
    runBenchmark<K, 256>(dataSize, readRatio, numOperations, false, false, true);
}

int main(int argc, char* argv[]) {
    // Default read ratio
    double readRatio = DEFAULT_READ_RATIO;
    
    // Parse read ratio from command line if provided
    if (argc > 1) {
        try {
            readRatio = std::stod(argv[1]);
            if (readRatio < 0.0 || readRatio > 1.0) {
                std::cerr << "Read ratio must be between 0.0 and 1.0" << std::endl;
                return 1;
            }
        } catch (const std::exception& e) {
            std::cerr << "Invalid read ratio: " << argv[1] << std::endl;
            return 1;
        }
    }
    
    std::cout << "KV Store Benchmarks" << std::endl;
    std::cout << "===================" << std::endl;
    
    // Benchmark (i): Fixed 1M data, 8-byte value
    runFixedSizeBenchmark<int, 8>(readRatio);
    
    // Benchmark (ii): Variable data size with fixed 8-byte value
    runVaryingDataSizeBenchmark<int, 8>(readRatio);
    
    // Benchmark (iii): Fixed 1M data with varying value sizes
    runVaryingValueSizeBenchmark<int>(readRatio);
    
    return 0;
}