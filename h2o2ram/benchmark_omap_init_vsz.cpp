#include <benchmark/benchmark.h>
#include <vector>
#include <array>
#include <cstring>
#include "hash_planner.hpp"
#include "omap.hpp"
#include "types.hpp"

// Value types of different sizes
struct Value16B {
    std::array<uint8_t, 16> data;
};

struct Value32B {
    std::array<uint8_t, 32> data;
};

struct Value64B {
    std::array<uint8_t, 64> data;
};

struct Value128B {
    std::array<uint8_t, 128> data;
};

struct Value256B {
    std::array<uint8_t, 256> data;
};

// First time initialization for 16B values
static void ValueSizeInit16B(benchmark::State& state) {
    size_t n = state.range(0);
    
    // Setup phase (not timed)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::pair<size_t, Value16B>> init_data;
    init_data.reserve(n);
    
    // Generate random data (not timed)
    std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
    for (size_t i = 0; i < n; i++) {
        Value16B val;
        uint32_t random_val = dist(gen);
        std::memset(val.data.data(), 0, val.data.size());
        std::memcpy(val.data.data(), &random_val, sizeof(random_val));
        init_data.push_back({dist(gen), val});
    }
    
    system("rm -r hash_*");  // Clear any existing hash files
    
    // Benchmark first time initialization
    for (auto _ : state) {
        ORAM::ObliviousMap<size_t, Value16B> omap;  // First time construction
        for (const auto& [key, value] : init_data) {
            omap.insert(key, value);
        }
    }
}

// First time initialization for 32B values
static void ValueSizeInit32B(benchmark::State& state) {
    size_t n = state.range(0);
    
    // Setup phase (not timed)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::pair<size_t, Value32B>> init_data;
    init_data.reserve(n);
    
    // Generate random data (not timed)
    std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
    for (size_t i = 0; i < n; i++) {
        Value32B val;
        uint32_t random_val = dist(gen);
        std::memset(val.data.data(), 0, val.data.size());
        std::memcpy(val.data.data(), &random_val, sizeof(random_val));
        init_data.push_back({dist(gen), val});
    }
    
    system("rm -r hash_*");  // Clear any existing hash files
    
    // Benchmark first time initialization
    for (auto _ : state) {
        ORAM::ObliviousMap<size_t, Value32B> omap;  // First time construction
        for (const auto& [key, value] : init_data) {
            omap.insert(key, value);
        }
    }
}

// First time initialization for 64B values
static void ValueSizeInit64B(benchmark::State& state) {
    size_t n = state.range(0);
    
    // Setup phase (not timed)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::pair<size_t, Value64B>> init_data;
    init_data.reserve(n);
    
    // Generate random data (not timed)
    std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
    for (size_t i = 0; i < n; i++) {
        Value64B val;
        uint32_t random_val = dist(gen);
        std::memset(val.data.data(), 0, val.data.size());
        std::memcpy(val.data.data(), &random_val, sizeof(random_val));
        init_data.push_back({dist(gen), val});
    }
    
    system("rm -r hash_*");  // Clear any existing hash files
    
    // Benchmark first time initialization
    for (auto _ : state) {
        ORAM::ObliviousMap<size_t, Value64B> omap;  // First time construction
        for (const auto& [key, value] : init_data) {
            omap.insert(key, value);
        }
    }
}

// First time initialization for 128B values
static void ValueSizeInit128B(benchmark::State& state) {
    size_t n = state.range(0);
    
    // Setup phase (not timed)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::pair<size_t, Value128B>> init_data;
    init_data.reserve(n);
    
    // Generate random data (not timed)
    std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
    for (size_t i = 0; i < n; i++) {
        Value128B val;
        uint32_t random_val = dist(gen);
        std::memset(val.data.data(), 0, val.data.size());
        std::memcpy(val.data.data(), &random_val, sizeof(random_val));
        init_data.push_back({dist(gen), val});
    }
    
    system("rm -r hash_*");  // Clear any existing hash files
    
    // Benchmark first time initialization
    for (auto _ : state) {
        ORAM::ObliviousMap<size_t, Value128B> omap;  // First time construction
        for (const auto& [key, value] : init_data) {
            omap.insert(key, value);
        }
    }
}

// First time initialization for 256B values
static void ValueSizeInit256B(benchmark::State& state) {
    size_t n = state.range(0);
    
    // Setup phase (not timed)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::pair<size_t, Value256B>> init_data;
    init_data.reserve(n);
    
    // Generate random data (not timed)
    std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
    for (size_t i = 0; i < n; i++) {
        Value256B val;
        uint32_t random_val = dist(gen);
        std::memset(val.data.data(), 0, val.data.size());
        std::memcpy(val.data.data(), &random_val, sizeof(random_val));
        init_data.push_back({dist(gen), val});
    }
    
    system("rm -r hash_*");  // Clear any existing hash files
    
    // Benchmark first time initialization
    for (auto _ : state) {
        ORAM::ObliviousMap<size_t, Value256B> omap;  // First time construction
        for (const auto& [key, value] : init_data) {
            omap.insert(key, value);
        }
    }
}

// Register benchmarks with specific sizes
static void CustomizedArgsN(benchmark::internal::Benchmark *b) {
    for (size_t i = 16; i <= 23; i++) // n := 2**i
    {
        size_t n = 1ll << i;
        b->Args({(int64_t)n});
    }
}

// Fixed 1M size
static void FixedSize1M(benchmark::internal::Benchmark *b) {
    b->Args({1ll << 20}); // 1M elements (2^20)
}

// Register benchmarks with fixed 1M data size
BENCHMARK(ValueSizeInit16B)->Apply(FixedSize1M)->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->UseRealTime()->Name("ValueSizeInit_16B");
BENCHMARK(ValueSizeInit32B)->Apply(FixedSize1M)->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->UseRealTime()->Name("ValueSizeInit_32B");
BENCHMARK(ValueSizeInit64B)->Apply(FixedSize1M)->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->UseRealTime()->Name("ValueSizeInit_64B");
BENCHMARK(ValueSizeInit128B)->Apply(FixedSize1M)->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->UseRealTime()->Name("ValueSizeInit_128B");
BENCHMARK(ValueSizeInit256B)->Apply(FixedSize1M)->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->UseRealTime()->Name("ValueSizeInit_256B");