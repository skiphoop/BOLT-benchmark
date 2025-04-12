#ifndef BENCHMARK_UTILS_HPP
#define BENCHMARK_UTILS_HPP

#include <chrono>
#include <random>
#include <array>
#include <iostream>
#include <iomanip>
#include <utility>
#include <vector>

// Helper function to generate random data of any size
template<size_t Size>
std::array<uint8_t, Size> generateRandomData(std::mt19937& gen) {
    std::array<uint8_t, Size> value;
    std::uniform_int_distribution<uint16_t> dist(0, 255);
    for (size_t i = 0; i < Size; i++) {
        value[i] = static_cast<uint8_t>(dist(gen));
    }
    return value;
}

// Helper function to print data in hex
template<size_t Size>
void printHexData(const std::array<uint8_t, Size>& value) {
    for (uint8_t byte : value) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }
    std::cout << std::dec;
}

// Generate random operations (0 = get, 1 = update)
inline std::vector<std::pair<int, int>> generateRandomOperations(size_t numOperations, size_t dataSize, double readRatio = 0.5) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<std::pair<int, int>> operations(numOperations);
    std::uniform_int_distribution<> keyDis(0, dataSize - 1);
    std::uniform_real_distribution<> ratioDistr(0.0, 1.0);

    for (size_t i = 0; i < numOperations; i++) {
        int op = (ratioDistr(gen) < readRatio) ? 0 : 1; // 0 for get, 1 for update
        operations[i] = {op, keyDis(gen)};
    }

    return operations;
}

// Timer utility
class Timer {
private:
    std::chrono::high_resolution_clock::time_point startTime;
    
public:
    void start() {
        startTime = std::chrono::high_resolution_clock::now();
    }
    
    double elapsedMicroseconds() const {
        auto endTime = std::chrono::high_resolution_clock::now();
        return static_cast<double>(std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count());
    }
    
    double elapsedMilliseconds() const {
        return elapsedMicroseconds() / 1000.0;
    }
};

#endif // BENCHMARK_UTILS_HPP