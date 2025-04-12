#include <benchmark/benchmark.h>
#include <vector>
#include <array>
#include <cstring>
#include <algorithm>
#include <random>
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

// Fixture for 16B value size
class OMapDataFixture16B : public benchmark::Fixture {
public:
    size_t n;
    using IndexType = size_t;
    std::vector<std::pair<IndexType, Value16B>> raw_data;
    std::random_device rd;
    ORAM::ObliviousMap<IndexType, Value16B> omap;
    
    void SetUp(const ::benchmark::State &state) override {
        std::mt19937 gen(rd());
        n = state.range(0);
        raw_data.resize(n);
        std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
        
        // Generate random data
        for (size_t i = 0; i < n; i++) {
            Value16B val;
            uint32_t random_val = dist(gen);
            std::memset(val.data.data(), 0, val.data.size());
            std::memcpy(val.data.data(), &random_val, sizeof(random_val));
            raw_data[i] = {dist(gen), val};
        }
        
        std::shuffle(raw_data.begin(), raw_data.end(), gen);
        
        // Pre-populate OMAP
        for (const auto& [key, value] : raw_data) {
            omap.insert(key, value);
        }
    }
    
    void TearDown(const ::benchmark::State &) override {
        raw_data.clear();
        raw_data.shrink_to_fit();
    }
};

// Fixture for 32B value size
class OMapDataFixture32B : public benchmark::Fixture {
public:
    size_t n;
    using IndexType = size_t;
    std::vector<std::pair<IndexType, Value32B>> raw_data;
    std::random_device rd;
    ORAM::ObliviousMap<IndexType, Value32B> omap;
    
    void SetUp(const ::benchmark::State &state) override {
        std::mt19937 gen(rd());
        n = state.range(0);
        raw_data.resize(n);
        std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
        
        // Generate random data
        for (size_t i = 0; i < n; i++) {
            Value32B val;
            uint32_t random_val = dist(gen);
            std::memset(val.data.data(), 0, val.data.size());
            std::memcpy(val.data.data(), &random_val, sizeof(random_val));
            raw_data[i] = {dist(gen), val};
        }
        
        std::shuffle(raw_data.begin(), raw_data.end(), gen);
        
        // Pre-populate OMAP
        for (const auto& [key, value] : raw_data) {
            omap.insert(key, value);
        }
    }
    
    void TearDown(const ::benchmark::State &) override {
        raw_data.clear();
        raw_data.shrink_to_fit();
    }
};

// Fixture for 64B value size
class OMapDataFixture64B : public benchmark::Fixture {
public:
    size_t n;
    using IndexType = size_t;
    std::vector<std::pair<IndexType, Value64B>> raw_data;
    std::random_device rd;
    ORAM::ObliviousMap<IndexType, Value64B> omap;
    
    void SetUp(const ::benchmark::State &state) override {
        std::mt19937 gen(rd());
        n = state.range(0);
        raw_data.resize(n);
        std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
        
        // Generate random data
        for (size_t i = 0; i < n; i++) {
            Value64B val;
            uint32_t random_val = dist(gen);
            std::memset(val.data.data(), 0, val.data.size());
            std::memcpy(val.data.data(), &random_val, sizeof(random_val));
            raw_data[i] = {dist(gen), val};
        }
        
        std::shuffle(raw_data.begin(), raw_data.end(), gen);
        
        // Pre-populate OMAP
        for (const auto& [key, value] : raw_data) {
            omap.insert(key, value);
        }
    }
    
    void TearDown(const ::benchmark::State &) override {
        raw_data.clear();
        raw_data.shrink_to_fit();
    }
};

// Fixture for 128B value size
class OMapDataFixture128B : public benchmark::Fixture {
public:
    size_t n;
    using IndexType = size_t;
    std::vector<std::pair<IndexType, Value128B>> raw_data;
    std::random_device rd;
    ORAM::ObliviousMap<IndexType, Value128B> omap;
    
    void SetUp(const ::benchmark::State &state) override {
        std::mt19937 gen(rd());
        n = state.range(0);
        raw_data.resize(n);
        std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
        
        // Generate random data
        for (size_t i = 0; i < n; i++) {
            Value128B val;
            uint32_t random_val = dist(gen);
            std::memset(val.data.data(), 0, val.data.size());
            std::memcpy(val.data.data(), &random_val, sizeof(random_val));
            raw_data[i] = {dist(gen), val};
        }
        
        std::shuffle(raw_data.begin(), raw_data.end(), gen);
        
        // Pre-populate OMAP
        for (const auto& [key, value] : raw_data) {
            omap.insert(key, value);
        }
    }
    
    void TearDown(const ::benchmark::State &) override {
        raw_data.clear();
        raw_data.shrink_to_fit();
    }
};

// Fixture for 256B value size
class OMapDataFixture256B : public benchmark::Fixture {
public:
    size_t n;
    using IndexType = size_t;
    std::vector<std::pair<IndexType, Value256B>> raw_data;
    std::random_device rd;
    ORAM::ObliviousMap<IndexType, Value256B> omap;
    
    void SetUp(const ::benchmark::State &state) override {
        std::mt19937 gen(rd());
        n = state.range(0);
        raw_data.resize(n);
        std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
        
        // Generate random data
        for (size_t i = 0; i < n; i++) {
            Value256B val;
            uint32_t random_val = dist(gen);
            std::memset(val.data.data(), 0, val.data.size());
            std::memcpy(val.data.data(), &random_val, sizeof(random_val));
            raw_data[i] = {dist(gen), val};
        }
        
        std::shuffle(raw_data.begin(), raw_data.end(), gen);
        
        // Pre-populate OMAP
        for (const auto& [key, value] : raw_data) {
            omap.insert(key, value);
        }
    }
    
    void TearDown(const ::benchmark::State &) override {
        raw_data.clear();
        raw_data.shrink_to_fit();
    }
};

// Benchmark for 16B value size query
BENCHMARK_DEFINE_F(OMapDataFixture16B, OMapQuery16B)(benchmark::State &state) {
    for (auto _ : state) {
        for (size_t i = 0; i < 5000; i++) {
            auto value = omap[raw_data[i].first];
            benchmark::DoNotOptimize(value);
        }
    }
}

// Benchmark for 32B value size query
BENCHMARK_DEFINE_F(OMapDataFixture32B, OMapQuery32B)(benchmark::State &state) {
    for (auto _ : state) {
        for (size_t i = 0; i < 5000; i++) {
            auto value = omap[raw_data[i].first];
            benchmark::DoNotOptimize(value);
        }
    }
}

// Benchmark for 64B value size query
BENCHMARK_DEFINE_F(OMapDataFixture64B, OMapQuery64B)(benchmark::State &state) {
    for (auto _ : state) {
        for (size_t i = 0; i < 5000; i++) {
            auto value = omap[raw_data[i].first];
            benchmark::DoNotOptimize(value);
        }
    }
}

// Benchmark for 128B value size query
BENCHMARK_DEFINE_F(OMapDataFixture128B, OMapQuery128B)(benchmark::State &state) {
    for (auto _ : state) {
        for (size_t i = 0; i < 5000; i++) {
            auto value = omap[raw_data[i].first];
            benchmark::DoNotOptimize(value);
        }
    }
}

// Benchmark for 256B value size query
BENCHMARK_DEFINE_F(OMapDataFixture256B, OMapQuery256B)(benchmark::State &state) {
    for (auto _ : state) {
        for (size_t i = 0; i < 5000; i++) {
            auto value = omap[raw_data[i].first];
            benchmark::DoNotOptimize(value);
        }
    }
}

// Fixed 1M size
static void FixedSize1M(benchmark::internal::Benchmark *b) {
    b->Args({1ll << 20}); // 1M elements (2^20)
}

// Register benchmarks with fixed 1M data size
BENCHMARK_REGISTER_F(OMapDataFixture16B, OMapQuery16B)
    ->Apply(FixedSize1M)
    ->Unit(benchmark::kMicrosecond)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("ValueSizeQuery_16B");

BENCHMARK_REGISTER_F(OMapDataFixture32B, OMapQuery32B)
    ->Apply(FixedSize1M)
    ->Unit(benchmark::kMicrosecond)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("ValueSizeQuery_32B");

BENCHMARK_REGISTER_F(OMapDataFixture64B, OMapQuery64B)
    ->Apply(FixedSize1M)
    ->Unit(benchmark::kMicrosecond)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("ValueSizeQuery_64B");

BENCHMARK_REGISTER_F(OMapDataFixture128B, OMapQuery128B)
    ->Apply(FixedSize1M)
    ->Unit(benchmark::kMicrosecond)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("ValueSizeQuery_128B");

BENCHMARK_REGISTER_F(OMapDataFixture256B, OMapQuery256B)
    ->Apply(FixedSize1M)
    ->Unit(benchmark::kMicrosecond)
    ->MeasureProcessCPUTime()
    ->UseRealTime()
    ->Name("ValueSizeQuery_256B");