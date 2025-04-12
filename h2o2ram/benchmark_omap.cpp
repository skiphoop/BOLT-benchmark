#include <benchmark/benchmark.h>
#include <vector>
#include "hash_planner.hpp"
#include "omap.hpp"
#include "types.hpp"

class OMapDataFixture : public benchmark::Fixture {
public:
    size_t n;
    using IndexType = uint32_t;
    std::vector<std::pair<IndexType, size_t>> raw_data;
    std::vector<std::pair<IndexType, size_t>> new_data;
    std::random_device rd;
    ORAM::ObliviousMap<IndexType, size_t> omap;

    void SetUp(const ::benchmark::State &state) override
    {
        std::mt19937 gen(rd());
        n = state.range(0);
        raw_data.resize(n);
        new_data.resize(n);
        std::uniform_int_distribution<> dist(0, std::numeric_limits<int>::max());
        for (size_t i = 0; i < n; i++)
            raw_data[i] = {dist(gen), dist(gen)};
        std::shuffle(raw_data.begin(), raw_data.end(), gen);

        for (size_t i = 0; i < n; i++)
            new_data[i] = {dist(gen), dist(gen)};
        std::shuffle(new_data.begin(), new_data.end(), gen);
        
        // Pre-populate OMAP
        for (const auto& [key, value] : raw_data) {
            omap.insert(key, value);
        }
    }

    void TearDown(const ::benchmark::State &) override
    {
        raw_data.clear();
        raw_data.shrink_to_fit();
    }
};

BENCHMARK_DEFINE_F(OMapDataFixture, OMapR)
(benchmark::State &state) 
{
    for (auto _ : state) 
    {        
        for (size_t i = 0; i < 2500; i++) {
            auto value = omap[raw_data[i].first];
        }
        
        for (size_t i = 0; i < 2500; i++) {
            omap.insert(new_data[i].first, new_data[i].second);
        }
    }
}

static void CustomizedArgsN(benchmark::internal::Benchmark *b) {
    for (size_t i = 16; i <= 23; i++) // n := 2**i
    {
        size_t n = 1ll << i;
        b->Args({(int64_t)n});
    }
}

BENCHMARK_REGISTER_F(OMapDataFixture, OMapR)->Apply(CustomizedArgsN)->MeasureProcessCPUTime()->UseRealTime()->Unit(benchmark::kMicrosecond);