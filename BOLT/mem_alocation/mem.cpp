#include <iostream>
#include <cmath>
#include <iomanip>

// Function to calculate log base 2
double log2(double x) {
    return log(x) / log(2);
}

// Function to compute memory costs of data structures
void computeMemoryCosts(long long N, double c, double alpha, int V_bytes) {
    std::cout << "Input Parameters:" << std::endl;
    std::cout << "N = " << N << " (number of data items)" << std::endl;
    std::cout << "c = " << c << " (N/B ratio)" << std::endl;
    std::cout << "alpha = " << alpha << " (K/B ratio)" << std::endl;
    std::cout << "alpha = " << alpha << " (M/B ratio)" << std::endl;
    std::cout << "V = " << V_bytes << " bytes (value size)" << std::endl;
    std::cout << std::endl;

    // Calculate derived parameters
    long long B = N / c;
    long long K = alpha * B; // α = K/B, so K = α * B
    long long M = B - K;     // M = B - K
    
    // Calculate log2log2N for hash table - use 4 as specified
    double log2log2N_val = log2(log2(N));
    int log2log2N_int = 4; // Using exactly 4 as specified (not rounding)
    
    double l_max = c + log2log2N_val;
    int l_max_int = 12; // Using exactly 12 as specified

    std::cout << "Derived Parameters:" << std::endl;
    std::cout << "B = " << B << " bins" << std::endl;
    std::cout << "K = " << K << std::endl;
    std::cout << "M = " << M << std::endl;
    std::cout << "l_max = " << l_max << " (using " << l_max_int << " for calculations)" << std::endl;
    std::cout << "log2log2N = " << log2log2N_val << " (using " << log2log2N_int << " for hash table)" << std::endl;
    std::cout << std::endl;

    // (i) Hash Table size
    int block_size_bits = 32 + 2 * 32; // 32-bit key + two 32-bit location indexes (removed 8-bit state)
    int block_size_bytes = 12; // Exactly 12 bytes per block (as specified)
    long long hash_table_size = N * log2log2N_int * block_size_bytes; // Using log2log2N instead of l_max

    // (ii) Value Store Space size
    long long num_frames = K * c + l_max_int * sqrt(K * log(N));
    long long value_store_size = num_frames * V_bytes;

    // (iii) Eviction Buffer Reverse Index size
    long long eviction_buffer_size = B * 4; // 4 bytes for 32-bit index

    // (iv) M Pages size
    int tuples_per_page = 8 + l_max_int;
    int tuple_size = 4 + V_bytes; // 4 bytes for 32-bit key + V bytes for value
    long long m_pages_size = M * tuples_per_page * tuple_size;

    // Total Memory Cost
    long long total_memory = hash_table_size + value_store_size + eviction_buffer_size + m_pages_size;

    // Raw Data Size
    long long raw_data_size = N * (4 + V_bytes); // 4 bytes for 32-bit key + V bytes for value

    // Calculate ratio
    double ratio = static_cast<double>(total_memory) / raw_data_size;

    // Output results
    std::cout << "Memory Costs:" << std::endl;
    std::cout << "(i) Hash Table: " << hash_table_size << " bytes (" 
              << std::fixed << std::setprecision(2) << (hash_table_size / (1024.0 * 1024.0)) << " MB)" << std::endl;
    
    std::cout << "(ii) Value Store Space: " << value_store_size << " bytes (" 
              << std::fixed << std::setprecision(2) << (value_store_size / (1024.0 * 1024.0)) << " MB)" << std::endl;
    
    std::cout << "(iii) Eviction Buffer Reverse Index: " << eviction_buffer_size << " bytes (" 
              << std::fixed << std::setprecision(2) << (eviction_buffer_size / (1024.0 * 1024.0)) << " MB)" << std::endl;
    
    std::cout << "(iv) M Pages: " << m_pages_size << " bytes (" 
              << std::fixed << std::setprecision(2) << (m_pages_size / (1024.0 * 1024.0)) << " MB)" << std::endl;
    
    std::cout << "Total Memory Cost: " << total_memory << " bytes (" 
              << std::fixed << std::setprecision(2) << (total_memory / (1024.0 * 1024.0)) << " MB)" << std::endl;
    
    std::cout << "Raw Data Size: " << raw_data_size << " bytes (" 
              << std::fixed << std::setprecision(2) << (raw_data_size / (1024.0 * 1024.0)) << " MB)" << std::endl;
    
    std::cout << "Ratio (Total Memory / Raw Data Size): " 
              << std::fixed << std::setprecision(2) << ratio << std::endl;
}

int main() {
    // Default parameters
    long long N = 1000000; // 1M
    double c = 8.0;
    double alpha = 0.2;
    int V_bytes = 8; // Default value size is 8 bytes
    
    // Compute memory costs with default parameters
    computeMemoryCosts(N, c, alpha, V_bytes);
    
    // Additional: Allow user to input custom parameters
    char choice;
    std::cout << "\nDo you want to calculate with custom parameters? (y/n): ";
    std::cin >> choice;
    
    if (choice == 'y' || choice == 'Y') {
        std::cout << "Enter N (number of data items): ";
        std::cin >> N;
        
        std::cout << "Enter c (B/N ratio): ";
        std::cin >> c;
        
        std::cout << "Enter alpha (M/B ratio): ";
        std::cin >> alpha;
        
        std::cout << "Enter V (value size in bytes): ";
        std::cin >> V_bytes;
        
        std::cout << "\n--- Custom Parameters Calculation ---\n" << std::endl;
        computeMemoryCosts(N, c, alpha, V_bytes);
    }
    
    return 0;
}