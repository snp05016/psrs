#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <thread>
#include <vector>

using namespace std;

void generateChunk(vector<long>& data, long start, long end, unsigned int seed) {
    // Each thread uses a different seed based on thread id
    srandom(seed + start);
    for (long i = start; i < end; i++) {
        data[i] = random();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <output_file> <num_elements> <seed>" << endl;
        return 1;
    }
    
    const char* filename = argv[1];
    long n = atol(argv[2]); 
    unsigned int seed = atoi(argv[3]);   
    
    // Pre-allocate vector for all data
    vector<long> data(n);
    
    // Use all available cores for maximum speed
    unsigned int num_threads = thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 8; // higher fallback for speed
    // For large datasets, use more threads for better parallelism
    if (n > 10000000) {
        num_threads = min(num_threads * 2, 16u); // up to 16 threads for large files
    }
    
    vector<thread> threads;
    long chunk_size = (n + num_threads - 1) / num_threads;
    for (unsigned int t = 0; t < num_threads; t++) {
        long start = t * chunk_size;
        long end = min(start + chunk_size, n);
        if (start < n) {
            threads.emplace_back(generateChunk, ref(data), start, end, seed);
        }
    }
    for (auto& thread : threads) {
        thread.join();
    }
    ofstream outfile(filename);
    if (!outfile.is_open()) {
        cerr << "Error: Cannot open file " << filename << endl;
        return 1;
    }
    
    for (long i = 0; i < n; i++) {
        outfile << data[i];
        if (i < n - 1) {
            outfile << "\n";
        }
    }
    
    outfile.close();
    cout << "Done! File " << filename << " created successfully." << endl;
    return 0;
}