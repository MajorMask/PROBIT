#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>
#include <random>

using namespace std;

// Baseline bisection implementation
namespace baseline {
    inline double Phi(double z) {
        constexpr double INV_SQRT_2 = 0.707106781186547524400844362104849039284835937688474036588;
        return 0.5 * erfc(-z * INV_SQRT_2);
    }
    
    inline double invert_bisect(double x) {
        double lo = -12.0;
        double hi =  12.0;
        if (x < 0.5) {
            hi = 0.0;
        } else {
            lo = 0.0;
        }
        
        for (int iter = 0; iter < 80; ++iter) {
            double mid = 0.5 * (lo + hi);
            double cdf = Phi(mid);
            if (cdf < x) {
                lo = mid;
            } else {
                hi = mid;
            }
        }
        return 0.5 * (lo + hi);
    }
}

// Include optimized implementation
#include "InverseCumulativeNormal.h"

class Timer {
    chrono::high_resolution_clock::time_point start_time;
public:
    void start() { start_time = chrono::high_resolution_clock::now(); }
    double elapsed_ns() {
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double, nano>(end - start_time).count();
    }
};

int main() {
    cout << "======================================================================\n";
    cout << "  Performance Comparison: Optimized vs Baseline Bisection\n";
    cout << "======================================================================\n\n";
    
    const int n_calls = 10000000;
    random_device rd;
    mt19937 gen(42);
    uniform_real_distribution<double> dist(1e-10, 1.0 - 1e-10);
    
    // Generate test data
    vector<double> x_values(n_calls);
    for (int i = 0; i < n_calls; ++i) {
        x_values[i] = dist(gen);
    }
    
    cout << "Testing with " << n_calls << " random values in (1e-10, 1-1e-10)\n\n";
    
    // Benchmark baseline
    Timer timer;
    timer.start();
    double sum_baseline = 0.0;
    for (int i = 0; i < n_calls; ++i) {
        sum_baseline += baseline::invert_bisect(x_values[i]);
    }
    double time_baseline_ns = timer.elapsed_ns() / n_calls;
    
    // Benchmark optimized
    quant::InverseCumulativeNormal icn_opt;
    timer.start();
    double sum_opt = 0.0;
    for (int i = 0; i < n_calls; ++i) {
        sum_opt += icn_opt(x_values[i]);
    }
    double time_opt_ns = timer.elapsed_ns() / n_calls;
    
    cout << fixed << setprecision(2);
    cout << "BASELINE (Bisection 80 iterations):\n";
    cout << "  Time per call: " << time_baseline_ns << " ns\n";
    cout << "  Throughput:    " << (1e9 / time_baseline_ns) / 1e6 << " M calls/sec\n";
    cout << "  (checksum: " << sum_baseline << ")\n\n";
    
    cout << "OPTIMIZED (Rational + Halley):\n";
    cout << "  Time per call: " << time_opt_ns << " ns\n";
    cout << "  Throughput:    " << (1e9 / time_opt_ns) / 1e6 << " M calls/sec\n";
    cout << "  (checksum: " << sum_opt << ")\n\n";
    
    double speedup = time_baseline_ns / time_opt_ns;
    cout << fixed << setprecision(1);
    cout << "SPEEDUP: " << speedup << "x faster\n";
    cout << "TARGET:  >10x (assignment requirement)\n";
    cout << "STATUS:  " << (speedup >= 10.0 ? "✓ PASS" : "✗ FAIL") << "\n\n";
    
    // Accuracy comparison
    cout << "Accuracy comparison (sample):\n";
    cout << fixed << setprecision(15);
    for (int i = 0; i < 5; ++i) {
        double x = x_values[i * (n_calls / 5)];
        double z_baseline = baseline::invert_bisect(x);
        double z_opt = icn_opt(x);
        double diff = abs(z_opt - z_baseline);
        cout << "  x=" << x << ": baseline=" << z_baseline 
             << ", opt=" << z_opt << ", diff=" << scientific << diff << fixed << "\n";
    }
    
    cout << "\n======================================================================\n";
    
    return 0;
}
