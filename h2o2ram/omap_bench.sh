#!/bin/bash

./ORAMBenchmark --benchmark_filter=OMapRInit/131072/ --benchmark_format=json --benchmark_out="h2o2ram_init_100k.json" --benchmark_out_format=json
./ORAMBenchmark --benchmark_filter=OMapRInit/1048576/ --benchmark_format=json --benchmark_out="h2o2ram_init_1m.json" --benchmark_out_format=json
./ORAMBenchmark --benchmark_filter=OMapRInit/8388608/ --benchmark_format=json --benchmark_out="h2o2ram_init_10m.json" --benchmark_out_format=json

./ORAMBenchmark --benchmark_filter=OMapDataFixture/OMapR/131072/ --benchmark_format=json --benchmark_out="h2o2ram_query_100k.json" --benchmark_out_format=json
./ORAMBenchmark --benchmark_filter=OMapDataFixture/OMapR/1048576/ --benchmark_format=json --benchmark_out="h2o2ram_query_1m.json" --benchmark_out_format=json
./ORAMBenchmark --benchmark_filter=OMapDataFixture/OMapR/8388608/ --benchmark_format=json --benchmark_out="h2o2ram_query_10m.json" --benchmark_out_format=json