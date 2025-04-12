#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <cstring>
#include <unordered_map>
#include <cmath>

// Constants
constexpr uint32_t N = 1 << 20;  // Number of data elements (2^20)
constexpr uint32_t DIVISOR = 8;  // Configurable divisor (2, 4, or 8)
constexpr uint32_t M = N / DIVISOR; // Total number of bins is N/DIVISOR
constexpr double ALPHA = 0.5;   // Partition coefficient (fraction of bins in HBM)
constexpr uint32_t K = static_cast<uint32_t>(M * ALPHA); // Number of HBM bins
constexpr uint32_t B = M - K;    // Number of Host bins
constexpr uint64_t NUM_ACCESSES = 1000000000;  // 1 billion accesses
constexpr uint64_t SAMPLE_INTERVAL = 100000000;  // Print every 10 million accesses

// Structure to hold simulation state
struct SimulationState {
    uint16_t* bin_loads;         // Array of bin loads
    uint32_t* data_location;     // Array mapping data elements to their current bins
    uint32_t max_bin_load;       // Maximum load of any single bin
    uint64_t total_hbm_load;     // Current total load of all HBM bins
    uint64_t max_hbm_load;       // Maximum total HBM load observed
    
    // Optimized queue implementation - counts by label instead of storing individual elements
    std::unordered_map<uint32_t, uint32_t> queue_counts; // Maps label -> count
    uint64_t total_queue_size;   // Total number of elements in queue
    uint64_t max_queue_size;     // Maximum queue size observed

    // HBM usage tracking
    uint64_t max_hbm_usage;      // Maximum HBM usage (HBM load + queue size) observed
};

// Structure to hold theoretical values
struct TheoreticalValues {
    double max_bin_load;
    double max_hbm_load;
    double max_queue_size;
    double max_hbm_usage;        // Theoretical maximum HBM usage
};

// Fast random number generator using PCG algorithm
class FastRandom {
private:
    uint64_t state;
    uint64_t inc;

public:
    FastRandom(uint64_t seed) {
        state = seed + 1442695040888963407ULL;
        inc = seed + 6364136223846793005ULL;
    }
    
    // PCG algorithm - faster than mt19937 for this use case
    inline uint32_t next() {
        uint64_t oldstate = state;
        state = oldstate * 6364136223846793005ULL + inc;
        uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
        uint32_t rot = oldstate >> 59u;
        return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
    }
    
    // Get random number within range
    inline uint32_t next_range(uint32_t range) {
        return next() % range;
    }
    
    // Get random number within range starting from 1
    inline uint32_t next_range_one_based(uint32_t range) {
        return next_range(range) + 1;
    }
    
    // Get two different random numbers efficiently
    inline void next_two_different(uint32_t& a, uint32_t& b, uint32_t range) {
        a = next_range(range);
        b = next_range(range - 1);
        if (b >= a) b++; // Avoid generating the same number
    }
    
    // Generate a random double between 0 and 1
    inline double next_double() {
        return static_cast<double>(next()) / static_cast<double>(UINT32_MAX);
    }
};

// Function to check if a bin is in HBM region
inline bool is_hbm_bin(uint32_t bin_index) {
    return bin_index < K;
}

// Function to check if a bin is in Host region
inline bool is_host_bin(uint32_t bin_index) {
    return bin_index >= K;
}

// Calculate theoretical values based on the formulas
TheoreticalValues calculate_theoretical_values() {
    TheoreticalValues theory;
    
    // (1) theoretical max bin load = c + log_2(log_2(N)) + 1, where c = N/M
    double c = static_cast<double>(N) / M;
    theory.max_bin_load = c + log2(log2(N)) + 1;
    
    // (2) theoretical HBM max = K * theoretical bin max
    theory.max_hbm_load = K * c + (theory.max_bin_load) * sqrt(K * log(N));
    
    // (3) theoretical queue = (1+alpha)B/2 + (theoretical_bin_max) * sqrt(B * ln(N))
    theory.max_queue_size = (1 + ALPHA) * B / 2 + theory.max_bin_load * sqrt(B * log(N));
    
    // (4) theoretical HBM usage = theoretical max HBM load + theoretical max queue size
    theory.max_hbm_usage = theory.max_hbm_load + theory.max_queue_size;
    
    return theory;
}

// Initialize the simulation state
SimulationState initialize_simulation() {
    SimulationState state;
    
    // Allocate memory for bin loads and data locations
    state.bin_loads = new uint16_t[M]();
    state.data_location = new uint32_t[N];
    
    // Initialize data location (evenly distribute data elements among bins)
    for (uint32_t i = 0; i < N; ++i) {
        uint32_t initial_bin = i % M;  // Distribute elements evenly
        state.data_location[i] = initial_bin;
        state.bin_loads[initial_bin]++;
    }
    
    // Find initial max load and HBM load
    state.max_bin_load = 0;
    state.total_hbm_load = 0;
    
    for (uint32_t j = 0; j < M; ++j) {
        if (state.bin_loads[j] > state.max_bin_load) {
            state.max_bin_load = state.bin_loads[j];
        }
        
        // Sum up elements in HBM region
        if (is_hbm_bin(j)) {
            state.total_hbm_load += state.bin_loads[j];
        }
    }
    
    state.max_hbm_load = state.total_hbm_load;
    
    // Initialize queue stats
    state.total_queue_size = 0;
    state.max_queue_size = 0;
    
    // Initialize HBM usage
    state.max_hbm_usage = state.total_hbm_load;
    
    return state;
}

// Update the maximum HBM usage
inline void update_max_hbm_usage(SimulationState& state) {
    uint64_t current_hbm_usage = state.total_hbm_load + state.total_queue_size;
    if (current_hbm_usage > state.max_hbm_usage) {
        state.max_hbm_usage = current_hbm_usage;
    }
}

// Handle the queue operations (enqueue and dequeue) - optimized version
void process_queue(SimulationState& state, FastRandom& rng, uint32_t new_bin) {
    // Enqueue operation - if new bin is in Host region, add a dummy element to queue
    if (is_host_bin(new_bin)) {
        uint32_t label = rng.next_range_one_based(B);  // Label from 1 to B
        state.queue_counts[label]++;
        state.total_queue_size++;
        update_max_hbm_usage(state); // Update max HBM usage after increasing queue size
    }
    
    // Update max queue size if needed
    if (state.total_queue_size > state.max_queue_size) {
        state.max_queue_size = state.total_queue_size;
    }
    
    // Skip dequeue if queue is empty
    if (state.total_queue_size == 0) {
        return;
    }
    
    // Dequeue operation based on probabilities
    double random_val = rng.next_double();
    double prob_first_case = 2 * ALPHA * (1 - ALPHA);
    double prob_second_case = prob_first_case + (1 - ALPHA) * (1 - ALPHA);
    
    if (random_val < prob_first_case) {
        // Case 1: With probability 2*alpha*(1-alpha), remove all elements with a single random label
        uint32_t rd = rng.next_range_one_based(B);
        auto it = state.queue_counts.find(rd);
        if (it != state.queue_counts.end()) {
            state.total_queue_size -= it->second;
            state.queue_counts.erase(it);
        }
    } 
    else if (random_val < prob_second_case) {
        // Case 2: With probability (1-alpha)^2, remove all elements with either of two random labels
        uint32_t rd1, rd2;
        rd1 = rng.next_range_one_based(B);
        do {
            rd2 = rng.next_range_one_based(B);
        } while (rd1 == rd2 && B > 1); // Avoid same label if possible
        
        // Remove elements with label rd1
        auto it1 = state.queue_counts.find(rd1);
        if (it1 != state.queue_counts.end()) {
            state.total_queue_size -= it1->second;
            state.queue_counts.erase(it1);
        }
        
        // Remove elements with label rd2
        auto it2 = state.queue_counts.find(rd2);
        if (it2 != state.queue_counts.end()) {
            state.total_queue_size -= it2->second;
            state.queue_counts.erase(it2);
        }
    }
    // Case 3: With probability alpha^2, do nothing (rest probability)
}

// Perform a single access operation in the simulation
void perform_access(SimulationState& state, FastRandom& rng) {
    // Select a random data element (0 to N-1)
    uint32_t data_element = rng.next_range(N);
    
    // Get and update current bin
    uint32_t current_bin = state.data_location[data_element];
    state.bin_loads[current_bin]--;
    
    // Update HBM load if element is leaving HBM
    if (is_hbm_bin(current_bin)) {
        state.total_hbm_load--;
        update_max_hbm_usage(state); // Update max HBM usage after decreasing HBM load
    }
    
    // Map to two random bins
    uint32_t bin1, bin2;
    rng.next_two_different(bin1, bin2, M);
    
    // Choose bin with smaller load
    uint32_t new_bin = (state.bin_loads[bin1] <= state.bin_loads[bin2]) ? bin1 : bin2;
    
    // Place element in new bin
    state.bin_loads[new_bin]++;
    state.data_location[data_element] = new_bin;
    
    // Update HBM load if element is entering HBM
    if (is_hbm_bin(new_bin)) {
        state.total_hbm_load++;
        if (state.total_hbm_load > state.max_hbm_load) {
            state.max_hbm_load = state.total_hbm_load;
        }
        update_max_hbm_usage(state); // Update max HBM usage after increasing HBM load
    }
    
    // Update max bin load if potentially exceeded
    if (state.bin_loads[new_bin] > state.max_bin_load) {
        state.max_bin_load = state.bin_loads[new_bin];
    }
    
    // Process queue operations for this access
    process_queue(state, rng, new_bin);
}

// Scan all bins to verify accurate statistics
void perform_full_scan(SimulationState& state) {
    uint32_t scan_max_bin_load = 0;
    uint64_t scan_total_hbm_load = 0;
    
    for (uint32_t j = 0; j < M; ++j) {
        if (state.bin_loads[j] > scan_max_bin_load) {
            scan_max_bin_load = state.bin_loads[j];
        }
        
        if (is_hbm_bin(j)) {
            scan_total_hbm_load += state.bin_loads[j];
        }
    }
    
    state.max_bin_load = scan_max_bin_load;
    state.total_hbm_load = scan_total_hbm_load;
    if (state.total_hbm_load > state.max_hbm_load) {
        state.max_hbm_load = state.total_hbm_load;
    }
    
    // Update max HBM usage after full scan
    update_max_hbm_usage(state);
}

// Print initial simulation parameters
void print_initial_info(const SimulationState& state, const TheoreticalValues& theory) {
    std::cout << "Simulating " << NUM_ACCESSES << " random accesses on " << N 
              << " data elements across " << M << " bins (N/" << DIVISOR << ")\n";
    std::cout << "HBM bins: " << K << " (" << (ALPHA * 100) << "%), Host bins: " << B << " (" << ((1 - ALPHA) * 100) << "%)\n";
    std::cout << "Initial elements per bin: " << N / M << "\n";
    std::cout << "Initial maximum bin load: " << state.max_bin_load << "\n";
    std::cout << "Initial total HBM load: " << state.total_hbm_load << " elements\n";
    std::cout << "-----------------------------------------------------------\n";
    std::cout << "Theoretical upper bounds:\n";
    std::cout << "  - Max bin load: " << std::fixed << std::setprecision(2) << theory.max_bin_load << "\n";
    std::cout << "  - Max HBM load: " << std::fixed << std::setprecision(2) << theory.max_hbm_load << "\n";
    std::cout << "  - Max queue size: " << std::fixed << std::setprecision(2) << theory.max_queue_size << "\n";
    std::cout << "  - Max HBM usage: " << std::fixed << std::setprecision(2) << theory.max_hbm_usage << "\n";
    std::cout << "-----------------------------------------------------------\n";
    std::cout << "Accesses (M) | Max Bin Load | Total HBM Load | Queue Size\n";
    std::cout << "-----------------------------------------------------------\n";
}

// Print progress update
void print_progress(const SimulationState& state, uint64_t access_count) {
    std::cout << std::setw(11) << access_count / 1000000 
              << " | " << std::setw(12) << state.max_bin_load 
              << " | " << std::setw(14) << state.total_hbm_load 
              << " | " << std::setw(10) << state.total_queue_size << "\n";
}

// Print final statistics and comparison with theoretical bounds
void print_final_stats(const SimulationState& state, const TheoreticalValues& theory, double duration_secs) {
    std::cout << "-----------------------------------------------------------\n";
    std::cout << "Simulation completed in " << duration_secs << " seconds\n";
    
    // Calculate slack (theoretical - experimental)
    double bin_load_slack = theory.max_bin_load - state.max_bin_load;
    double hbm_load_slack = theory.max_hbm_load - state.max_hbm_load;
    double queue_slack = theory.max_queue_size - state.max_queue_size;
    double hbm_usage_slack = theory.max_hbm_usage - state.max_hbm_usage;
    
    // Print comparison table
    std::cout << "\n";
    std::cout << "Comparison of Experimental Results vs. Theoretical Bounds:\n";
    std::cout << "+-----------------+---------------+----------------+----------+\n";
    std::cout << "| Metric          | Experimental  | Theoretical    | Slack    |\n";
    std::cout << "+-----------------+---------------+----------------+----------+\n";
    
    std::cout << "| Max Bin Load    | " 
              << std::setw(13) << state.max_bin_load << " | " 
              << std::fixed << std::setprecision(2) 
              << std::setw(14) << theory.max_bin_load << " | "
              << std::setw(8) << bin_load_slack << " |\n";
    
    std::cout << "| Max HBM Load    | " 
              << std::setw(13) << state.max_hbm_load << " | " 
              << std::fixed << std::setprecision(2) 
              << std::setw(14) << theory.max_hbm_load << " | "
              << std::setw(8) << hbm_load_slack << " |\n";
    
    std::cout << "| Max Queue Size  | " 
              << std::setw(13) << state.max_queue_size << " | " 
              << std::fixed << std::setprecision(2) 
              << std::setw(14) << theory.max_queue_size << " | "
              << std::setw(8) << queue_slack << " |\n";
    
    std::cout << "| Max HBM Usage   | " 
              << std::setw(13) << state.max_hbm_usage << " | " 
              << std::fixed << std::setprecision(2) 
              << std::setw(14) << theory.max_hbm_usage << " | "
              << std::setw(8) << hbm_usage_slack << " |\n";
    
    std::cout << "+-----------------+---------------+----------------+----------+\n";
    
    // Additional statistics
    std::cout << "\nAdditional Statistics:\n";
    std::cout << "Final maximum bin load: " << state.max_bin_load << "\n";
    std::cout << "Final total HBM load: " << state.total_hbm_load << " elements (" 
              << (static_cast<double>(state.total_hbm_load) / N) * 100 << "% of total)\n";
    std::cout << "Maximum total HBM load during simulation: " << state.max_hbm_load << " elements (" 
              << (static_cast<double>(state.max_hbm_load) / N) * 100 << "% of total)\n";
    std::cout << "Final queue size: " << state.total_queue_size << " elements\n";
    std::cout << "Maximum queue size during simulation: " << state.max_queue_size << " elements\n";
    std::cout << "Maximum HBM usage (HBM load + queue size): " << state.max_hbm_usage << " elements\n";
    std::cout << "Number of unique labels in queue: " << state.queue_counts.size() << "\n";
    std::cout << "Theoretical average load per bin: " << static_cast<double>(N) / M << "\n";
}

// Run the main simulation
void run_simulation() {
    // Calculate theoretical values
    TheoreticalValues theory = calculate_theoretical_values();
    
    // Seed random generator with current time
    uint64_t seed = static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    FastRandom rng(seed);
    
    // Start timing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Initialize simulation
    SimulationState state = initialize_simulation();
    print_initial_info(state, theory);
    
    // Reserve capacity for queue map to avoid rehashing
    state.queue_counts.reserve(B + 1);
    
    // Main simulation loop
    for (uint64_t i = 0; i < NUM_ACCESSES; ++i) {
        perform_access(state, rng);
        
        // Output progress at intervals
        if ((i + 1) % SAMPLE_INTERVAL == 0) {
            // Perform full scan occasionally to ensure accuracy
            if (i % (SAMPLE_INTERVAL * 10) == 0) {
                perform_full_scan(state);
            }
            
            print_progress(state, i + 1);
        }
    }
    
    // End timing
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    
    // Final scan to get accurate statistics
    perform_full_scan(state);
    
    // Output final stats
    print_final_stats(state, theory, duration / 1000.0);
    
    // Clean up
    delete[] state.bin_loads;
    delete[] state.data_location;
}

int main() {
    run_simulation();
    return 0;
}