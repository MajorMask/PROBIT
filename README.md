# Inverse Cumulative Normal Implementation

## Overview

This project implements a fast, accurate inverse cumulative normal distribution function (probit function) using piecewise rational approximations with Halley refinement.

Performance: 30.6x faster than baseline bisection method
Accuracy: Machine precision (max error ~1e-15)

## Build Instructions

### Quick Start

```bash
# Generate coefficients and build
make

# Run tests
make test

# Clean build artifacts
make clean
```

### Manual Build

```bash
# Step 1: Generate coefficients (requires Python with NumPy, SciPy)
python3 export_coefficients.py

# Step 2: Generate C++ header from coefficients
python3 json_to_header.py

# Step 3: Compile
g++ -std=c++17 -O3 -march=native -o test test_benchmark.cpp -lm

# Step 4: Run
./test
```

## Project Structure

```
PROBIT/
├── export_coefficients.py      # Derives rational approximation coefficients
├── json_to_header.py            # Generates C++ header from JSON data
├── coefficients.json            # Coefficient data (generated)
├── InverseCumulativeNormal.h    # Main implementation header (generated)
├── test_benchmark.cpp           # Comprehensive test suite
├── benchmark_comparison.cpp     # Performance comparison with baseline
├── example_usage.cpp            # Usage examples
├── Makefile                     # Build automation
├── README.md                    # This file
└── DESIGN.md                    # Technical documentation
```

## Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- Python 3.6+ with NumPy and SciPy (for coefficient generation)
- Standard math library

## Usage

### Basic Usage

```cpp
#include "InverseCumulativeNormal.h"

using namespace quant;

int main() {
    InverseCumulativeNormal icn;  // Standard normal (mean=0, sigma=1)
    
    // Compute z-score for given probability
    double z = icn(0.975);  // Returns ~1.96
    
    // Batch processing
    double probabilities[] = {0.025, 0.5, 0.975};
    double z_scores[3];
    icn(probabilities, z_scores, 3);
    
    return 0;
}
```

### Custom Distribution

```cpp
InverseCumulativeNormal custom(100.0, 15.0);  // mean=100, sigma=15
double value = custom(0.95);  // 95th percentile
```

## Implementation Approach

### Coefficient Generation Pipeline

The implementation uses a three-stage build process:

1. Python script derives optimal coefficients via weighted least-squares fitting
2. Coefficients are exported to JSON format for portability and inspection
3. Second Python script reads JSON and generates complete C++ header file

This approach ensures reproducibility while maintaining zero runtime overhead.

### Numerical Method

The implementation uses piecewise rational approximations:

**Central Region (0.02425 ≤ x ≤ 0.97575)**
- Form: g(x) = (x - 0.5) × P(r) / Q(r) where r = (x - 0.5)²
- Degree: (6, 6) rational function
- Exploits symmetry for improved numerical properties

**Tail Regions (x < 0.02425 or x > 0.97575)**
- Form: g(x) = sign(x - 0.5) × C(t) / D(t) where t = sqrt(-2×log(m))
- Degree: (8, 8) rational function
- Tail transformation improves conditioning at extremes

**Refinement**
- Two iterations of Halley's method (third-order convergence)
- Stable residual computation using log-space arithmetic
- Avoids catastrophic cancellation in extreme tails

### Performance Optimizations

- Horner's method for polynomial evaluation (O(n) instead of O(n²))
- Compile-time constant coefficients (constexpr arrays)
- Minimal branching in hot paths
- Cache-friendly memory access patterns

## Test Results

All tests pass with the following accuracy:

- Symmetry: max error 3.1e-15
- Round-trip (Φ(Φ⁻¹(x)) = x): max error 1.1e-16
- Monotonicity: strictly increasing on (0,1)
- Derivative consistency: max relative error 9.3e-10

Performance comparison (10 million calls):

- Baseline (bisection): 2028.55 ns per call
- Optimized: 66.25 ns per call
- Speedup: 30.6x

## Design Decisions

### Choice of JSON Bridge

The coefficient generation uses JSON as an intermediate format because:

1. Reproducibility: Single source of truth for coefficients
2. Inspectability: Human-readable format for verification
3. Version control: Track coefficient changes in git
4. Language independence: JSON parsable by any language
5. Build automation: Makefile handles dependency chain automatically

### Degree Selection

Central region uses (6,6) and tail region uses (8,8) based on:

- Error analysis: Raw approximation achieves ~1e-5 error before refinement
- Performance: Higher degrees show diminishing returns
- Refinement: Two Halley steps bring error to machine precision

### Tail Transformation

The transformation t = sqrt(-2×log(m)) is standard in tail approximations because:

- Maps x → 0 to t → infinity, improving polynomial behavior
- Reduces condition number of least-squares system
- Allows lower-degree approximations in transformed space

## Known Limitations

1. Vector overload uses compiler auto-vectorization rather than explicit SIMD
2. Precision fundamentally limited by double precision (~1e-15)
3. Performance measurements are architecture-dependent

## Regenerating Coefficients

To experiment with different approximation degrees:

```bash
# Edit export_coefficients.py parameters
# For example, change m=6 to m=8 for higher degree

# Regenerate
make regenerate

# Test new coefficients
make test
```

## API Reference

### Constructor

```cpp
InverseCumulativeNormal(double average = 0.0, double sigma = 1.0)
```

Creates instance for normal distribution with specified mean and standard deviation.

### Scalar Operator

```cpp
double operator()(double x) const
```

Returns z such that Φ(z) = x for the configured distribution.
Input x must be in (0, 1); returns ±infinity at boundaries.

### Vector Operator

```cpp
void operator()(const double* in, double* out, std::size_t n) const
```

Batch processing: computes out[i] = Φ⁻¹(in[i]) for all i in [0, n).

### Static Method

```cpp
static double standard_value(double x)
```

Returns Φ⁻¹(x) for standard normal distribution (mean=0, sigma=1).

## Assumptions

- IEEE 754 floating-point arithmetic
- Input probabilities in open interval (0, 1)
- C++17 standard library available
- Optimization flags (-O3) used for compilation

## Trade-offs

The implementation prioritizes:

1. Speed over flexibility (header-only, template-free)
2. Accuracy over simplicity (Halley refinement adds complexity)
3. Compile-time over runtime (coefficients embedded at build time)
4. Automation over manual control (build system handles generation)

Alternative approaches considered but rejected:

- Single global rational: worse conditioning than piecewise
- Newton iteration: slower convergence than Halley
- Runtime JSON loading: unnecessary overhead for static data
- Manual coefficient entry: error-prone and not reproducible

## References

- Wichura, M. J. (1988). Algorithm AS 241: The percentage points of the normal distribution. Applied Statistics.
- Acklam, P. J. (2003). An algorithm for computing the inverse normal cumulative distribution function.
- Assignment specification: OP Kiitorata Trainee Program

## License

Submitted as part of OP Kiitorata Trainee Program evaluation.

## Contact

Refer to assignment submission guidelines for contact information.
