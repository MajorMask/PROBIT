#include "InverseCumulativeNormal.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <chrono>
#include <random>
#include <algorithm>

using namespace std;
using namespace quant;

// High-precision timer
class Timer {
    chrono::high_resolution_clock::time_point start_time;
public:
    void start() { start_time = chrono::high_resolution_clock::now(); }
    double elapsed_ns() {
        auto end = chrono::high_resolution_clock::now();
        return chrono::duration<double, nano>(end - start_time).count();
    }
    double elapsed_us() { return elapsed_ns() / 1000.0; }
    double elapsed_ms() { return elapsed_us() / 1000.0; }
};

// Standard normal CDF for validation
double standard_normal_cdf(double z) {
    constexpr double INV_SQRT_2 = 0.707106781186547524400844362104849039284835937688474036588;
    return 0.5 * erfc(-z * INV_SQRT_2);
}

// Test symmetry: Φ^{-1}(1-x) = -Φ^{-1}(x)
void test_symmetry() {
    cout << "\n=== Symmetry Test ===\n";
    InverseCumulativeNormal icn;
    
    vector<double> test_points = {
        0.001, 0.01, 0.05, 0.1, 0.2, 0.3, 0.4, 0.49
    };
    
    double max_sym_error = 0.0;
    for (double x : test_points) {
        double z1 = icn(x);
        double z2 = icn(1.0 - x);
        double sym_error = abs(z1 + z2);
        max_sym_error = max(max_sym_error, sym_error);
        
        if (sym_error > 1e-12) {
            cout << "x = " << x << ": z(x) = " << z1 
                 << ", z(1-x) = " << z2 
                 << ", error = " << sym_error << "\n";
        }
    }
    
    cout << "Max symmetry error: " << scientific << max_sym_error << "\n";
    cout << "Symmetry test: " << (max_sym_error < 1e-10 ? "PASS" : "FAIL") << "\n";
}

// Test round-trip: Φ(Φ^{-1}(x)) ≈ x
void test_roundtrip() {
    cout << "\n=== Round-Trip Test ===\n";
    InverseCumulativeNormal icn;
    
    // Test across the full range
    vector<double> test_points;
    
    // Extreme tails
    for (int i = 12; i >= 1; --i) {
        test_points.push_back(pow(10.0, -i));
    }
    
    // Central region
    for (double x = 0.01; x < 1.0; x += 0.01) {
        test_points.push_back(x);
    }
    
    // Upper tail
    for (int i = 1; i <= 12; ++i) {
        test_points.push_back(1.0 - pow(10.0, -i));
    }
    
    double max_error = 0.0;
    double mean_error = 0.0;
    
    for (double x : test_points) {
        double z = icn(x);
        double x_recovered = standard_normal_cdf(z);
        double error = abs(x - x_recovered);
        
        max_error = max(max_error, error);
        mean_error += error;
        
        if (error > 1e-10) {
            cout << fixed << setprecision(15)
                 << "x = " << x << " → z = " << z 
                 << " → x' = " << x_recovered 
                 << ", error = " << scientific << error << "\n";
        }
    }
    
    mean_error /= test_points.size();
    
    cout << "\nRound-trip statistics:\n";
    cout << "  Max error:  " << scientific << max_error << "\n";
    cout << "  Mean error: " << scientific << mean_error << "\n";
    cout << "Round-trip test: " << (max_error < 1e-10 ? "PASS" : "FAIL") << "\n";
}

// Test monotonicity
void test_monotonicity() {
    cout << "\n=== Monotonicity Test ===\n";
    InverseCumulativeNormal icn;
    
    const int n_points = 10000;
    bool is_monotonic = true;
    
    double prev_z = -numeric_limits<double>::infinity();
    for (int i = 1; i < n_points; ++i) {
        double x = i / double(n_points);
        double z = icn(x);
        
        if (z <= prev_z) {
            cout << "Monotonicity violation at x = " << x 
                 << ": z = " << z << " <= prev_z = " << prev_z << "\n";
            is_monotonic = false;
        }
        prev_z = z;
    }
    
    cout << "Monotonicity test: " << (is_monotonic ? "PASS" : "FAIL") << "\n";
}

// Test derivative: d/dx Φ^{-1}(x) = 1 / φ(Φ^{-1}(x))
void test_derivative() {
    cout << "\n=== Derivative Sanity Check ===\n";
    InverseCumulativeNormal icn;
    
    vector<double> test_points = {0.01, 0.1, 0.3, 0.5, 0.7, 0.9, 0.99};
    constexpr double h = 1e-7;
    
    double max_rel_error = 0.0;
    
    for (double x : test_points) {
        double z = icn(x);
        
        // Numerical derivative
        double z_plus = icn(x + h);
        double z_minus = icn(x - h);
        double dz_dx_numerical = (z_plus - z_minus) / (2 * h);
        
        // Analytical: dz/dx = 1/φ(z)
        constexpr double INV_SQRT_2PI = 0.398942280401432677939946059934381868475858631164934657;
        double phi_z = INV_SQRT_2PI * exp(-0.5 * z * z);
        double dz_dx_analytical = 1.0 / phi_z;
        
        double rel_error = abs(dz_dx_numerical - dz_dx_analytical) / dz_dx_analytical;
        max_rel_error = max(max_rel_error, rel_error);
        
        cout << fixed << setprecision(6)
             << "x = " << x << ": numerical = " << dz_dx_numerical
             << ", analytical = " << dz_dx_analytical
             << ", rel_err = " << scientific << rel_error << "\n";
    }
    
    cout << "\nMax relative error: " << scientific << max_rel_error << "\n";
    cout << "Derivative test: " << (max_rel_error < 1e-4 ? "PASS" : "FAIL") << "\n";
}

// Benchmark scalar performance
void benchmark_scalar() {
    cout << "\n=== Scalar Performance Benchmark ===\n";
    
    const int n_calls = 10000000;
    random_device rd;
    mt19937 gen(42);
    uniform_real_distribution<double> dist(1e-10, 1.0 - 1e-10);
    
    // Generate test data
    vector<double> x_values(n_calls);
    for (int i = 0; i < n_calls; ++i) {
        x_values[i] = dist(gen);
    }
    
    // Benchmark optimized version
    InverseCumulativeNormal icn_opt;
    Timer timer;
    
    timer.start();
    double sum_opt = 0.0;
    for (int i = 0; i < n_calls; ++i) {
        sum_opt += icn_opt(x_values[i]);
    }
    double time_opt_ns = timer.elapsed_ns() / n_calls;
    
    cout << fixed << setprecision(2);
    cout << "Optimized implementation:\n";
    cout << "  Time per call: " << time_opt_ns << " ns\n";
    cout << "  Throughput: " << (1e9 / time_opt_ns) / 1e6 << " M calls/sec\n";
    cout << "  (sum = " << sum_opt << " to prevent optimization)\n";
}

// Benchmark vector performance
void benchmark_vector() {
    cout << "\n=== Vector Performance Benchmark ===\n";
    
    const int n_elements = 1000000;
    random_device rd;
    mt19937 gen(42);
    uniform_real_distribution<double> dist(1e-10, 1.0 - 1e-10);
    
    vector<double> x_in(n_elements);
    vector<double> z_out(n_elements);
    
    for (int i = 0; i < n_elements; ++i) {
        x_in[i] = dist(gen);
    }
    
    InverseCumulativeNormal icn;
    Timer timer;
    
    // Measure vector overload
    timer.start();
    icn(x_in.data(), z_out.data(), n_elements);
    double time_vector_ms = timer.elapsed_ms();
    
    // Measure naive loop
    timer.start();
    for (int i = 0; i < n_elements; ++i) {
        z_out[i] = icn(x_in[i]);
    }
    double time_naive_ms = timer.elapsed_ms();
    
    cout << fixed << setprecision(2);
    cout << "Vector overload: " << time_vector_ms << " ms\n";
    cout << "Naive loop:      " << time_naive_ms << " ms\n";
    cout << "Speedup:         " << (time_naive_ms / time_vector_ms) << "x\n";
}

int main() {
    cout << "======================================================================\n";
    cout << "  Inverse Cumulative Normal (Probit) - Test & Benchmark Suite\n";
    cout << "======================================================================\n";
    
    // Correctness tests
    test_symmetry();
    test_roundtrip();
    test_monotonicity();
    test_derivative();
    
    // Performance benchmarks
    benchmark_scalar();
    benchmark_vector();
    
    cout << "\n======================================================================\n";
    cout << "All tests complete!\n";
    cout << "======================================================================\n";
    
    return 0;
}
