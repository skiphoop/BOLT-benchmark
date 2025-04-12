#include <benchmark/benchmark.h>
#include <vector>
#include "hash_planner.hpp"
#include "omap.hpp"
#include "types.hpp"

// First time initialization
static void OMapRInit(benchmark::State& state) {
    size_t n = state.range(0);
    
    // Setup phase (not timed)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::pair<size_t, size_t>> init_data;
    init_data.reserve(n);
    
    // Generate random data (not timed)
    std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
    for (size_t i = 0; i < n; i++) {
        init_data.push_back({dist(gen), dist(gen)});
    }

    system("rm -r hash_*");  // Clear any existing hash files

    // Benchmark first time initialization
    for (auto _ : state) {
        ORAM::ObliviousMap<size_t, size_t> omap;  // First time construction
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

BENCHMARK(OMapRInit)->Apply(CustomizedArgsN)->Unit(benchmark::kMicrosecond)->MeasureProcessCPUTime()->UseRealTime();

