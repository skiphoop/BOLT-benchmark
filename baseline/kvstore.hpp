#ifndef KV_STORE_HPP
#define KV_STORE_HPP

#include <vector>
#include <array>
#include <cstdint>
#include <algorithm>
#include <memory>

// Generic KVStore template that can handle values of any size
template<typename K, size_t ValueSize>
class KVStore {
private:
    static constexpr size_t CHAIN_SIZE = 16; // Fixed chain size
    static constexpr size_t PREFETCH_DISTANCE = 2; // Prefetch distance for chain traversal
    
    struct alignas(64) Entry { // Cache line alignment (typically 64 bytes)
        K key;
        std::unique_ptr<std::array<uint8_t, ValueSize>> value;
        bool isOccupied;
        
        Entry() : isOccupied(false), value(nullptr) {}
        
        Entry(K k, const std::array<uint8_t, ValueSize>& v) :
            key(k),
            value(std::make_unique<std::array<uint8_t, ValueSize>>(v)),
            isOccupied(true) {}
    };
    
    // Using fixed-size arrays instead of vectors for chains
    struct alignas(64) Chain { // Cache line alignment
        std::array<Entry, CHAIN_SIZE> entries;
        size_t size; // Number of occupied entries in this chain
        
        Chain() : size(0) {}
    };
    
    std::vector<Chain> table;
    size_t tableSize;
    
    // Enhanced hash function for better distribution at scale
    size_t hash(K key) const {
        uint64_t x = static_cast<uint64_t>(key);
        // MurmurHash3 finalizer
        x ^= x >> 33;
        x *= 0xff51afd7ed558ccdULL;
        x ^= x >> 33;
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= x >> 33;
        return x % tableSize;
    }
    
    // Find the next prime number (for table sizing)
    static size_t nextPrime(size_t n) {
        if (n <= 2) return 2;
        if (!(n & 1)) n++; // Make sure it's odd
        
        while (!isPrime(n)) {
            n += 2;
        }
        return n;
    }
    
    // Simple prime checker
    static bool isPrime(size_t n) {
        if (n <= 1) return false;
        if (n <= 3) return true;
        if (n % 2 == 0 || n % 3 == 0) return false;
        
        for (size_t i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0) {
                return false;
            }
        }
        return true;
    }

public:
    using ValueType = std::array<uint8_t, ValueSize>;
    
    // Constructor that scales table size based on expected data size
    KVStore(size_t dataSize = 1000000) {
        // Scale the table size based on expected data size
        // Using 1.5x the data size to reduce collisions
        tableSize = nextPrime(static_cast<size_t>(dataSize * 1.5));
        table.resize(tableSize);
    }
    
    ValueType get(K key) {
        size_t index = hash(key);
        const auto& chain = table[index];
        
        ValueType result{};
        bool found = false;
        
        // Always search the entire chain with prefetching
        for (size_t i = 0; i < CHAIN_SIZE; ++i) {
            // Prefetch ahead to reduce cache misses
            if (i + PREFETCH_DISTANCE < CHAIN_SIZE) {
                __builtin_prefetch(&chain.entries[i + PREFETCH_DISTANCE], 0, 1);
                if (chain.entries[i + PREFETCH_DISTANCE].value) {
                    __builtin_prefetch(chain.entries[i + PREFETCH_DISTANCE].value.get(), 0, 1);
                }
            }
            
            const auto& entry = chain.entries[i];
            if (entry.isOccupied && entry.key == key) {
                if (entry.value) {
                    result = *entry.value;
                }
                found = true;
                // No break - continue searching to the end
            }
        }
        
        return result;  // Return the found value or empty array if not found
    }
    
    void insert(K key, const ValueType& value) {
        size_t index = hash(key);
        auto& chain = table[index];
        
        bool keyExists = false;
        size_t existingIndex = 0;
        
        // Check if key already exists - always scan the entire chain
        for (size_t i = 0; i < CHAIN_SIZE; ++i) {
            // Prefetch ahead
            if (i + PREFETCH_DISTANCE < CHAIN_SIZE) {
                __builtin_prefetch(&chain.entries[i + PREFETCH_DISTANCE], 0, 1);
            }
            
            auto& entry = chain.entries[i];
            if (entry.isOccupied && entry.key == key) {
                keyExists = true;
                existingIndex = i;
                // No break - continue searching to the end
            }
        }
        
        // Update existing entry if found
        if (keyExists) {
            auto& entry = chain.entries[existingIndex];
            if (!entry.value) {
                entry.value = std::make_unique<ValueType>();
            }
            *entry.value = value;
            return;
        }
        
        // If chain is full, replace the last entry
        if (chain.size >= CHAIN_SIZE) {
            // Replace the last entry
            auto& entry = chain.entries[CHAIN_SIZE - 1];
            entry.key = key;
            if (!entry.value) {
                entry.value = std::make_unique<ValueType>();
            }
            *entry.value = value;
            entry.isOccupied = true;
        } else {
            // Add to the chain
            auto& entry = chain.entries[chain.size];
            entry.key = key;
            entry.value = std::make_unique<ValueType>(value);
            entry.isOccupied = true;
            chain.size++;
        }
    }
    
    bool remove(K key) {
        size_t index = hash(key);
        auto& chain = table[index];
        
        bool found = false;
        size_t foundIndex = 0;
        
        // Always scan the entire chain with prefetching
        for (size_t i = 0; i < CHAIN_SIZE; ++i) {
            // Prefetch ahead
            if (i + PREFETCH_DISTANCE < CHAIN_SIZE) {
                __builtin_prefetch(&chain.entries[i + PREFETCH_DISTANCE], 0, 1);
            }
            
            if (chain.entries[i].isOccupied && chain.entries[i].key == key) {
                found = true;
                foundIndex = i;
                // No break - continue searching to the end
            }
        }
        
        if (found) {
            // Mark as not occupied
            chain.entries[foundIndex].isOccupied = false;
            
            // Shift entries to keep them contiguous
            if (foundIndex < chain.size - 1) {
                // Move the last entry to this position
                chain.entries[foundIndex] = std::move(chain.entries[chain.size - 1]);
                // Clear the last entry
                chain.entries[chain.size - 1] = Entry();
            }
            
            chain.size--;
            return true;
        }
        
        return false;
    }
    
    void update(K key, const ValueType& newValue) {
        size_t index = hash(key);
        auto& chain = table[index];
        
        bool keyExists = false;
        size_t existingIndex = 0;
        
        // Always scan the entire chain with prefetching
        for (size_t i = 0; i < CHAIN_SIZE; ++i) {
            // Prefetch ahead
            if (i + PREFETCH_DISTANCE < CHAIN_SIZE) {
                __builtin_prefetch(&chain.entries[i + PREFETCH_DISTANCE], 0, 1);
            }
            
            auto& entry = chain.entries[i];
            if (entry.isOccupied && entry.key == key) {
                keyExists = true;
                existingIndex = i;
                // No break - continue searching to the end
            }
        }
        
        // Update existing entry if found
        if (keyExists) {
            auto& entry = chain.entries[existingIndex];
            if (!entry.value) {
                entry.value = std::make_unique<ValueType>();
            }
            *entry.value = newValue;
            return;
        }
        
        // Key not found, handle similar to insert
        if (chain.size >= CHAIN_SIZE) {
            // Replace the last entry
            auto& entry = chain.entries[CHAIN_SIZE - 1];
            entry.key = key;
            if (!entry.value) {
                entry.value = std::make_unique<ValueType>();
            }
            *entry.value = newValue;
            entry.isOccupied = true;
        } else {
            // Add to the chain
            auto& entry = chain.entries[chain.size];
            entry.key = key;
            entry.value = std::make_unique<ValueType>(newValue);
            entry.isOccupied = true;
            chain.size++;
        }
    }
};

#endif // KV_STORE_HPP