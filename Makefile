# Makefile for Inverse Cumulative Normal Implementation
# Automated build system with coefficient generation pipeline

CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -Wall -Wextra
LDFLAGS = -lm
PYTHON = python3

# Generated files
COEFF_JSON = coefficients.json
HEADER = InverseCumulativeNormal.h

# Source files
EXPORT_SCRIPT = export_coefficients.py
HEADER_GEN = json_to_header.py

# Executables
TARGETS = test_benchmark benchmark_comparison example_usage test_simple

.PHONY: all test clean regenerate help

# Default target: build all executables
all: $(TARGETS)

# Dependency chain: Python → JSON → Header → Executables

# Step 1: Generate coefficients from Python
$(COEFF_JSON): $(EXPORT_SCRIPT)
	@echo "Generating coefficients..."
	$(PYTHON) $(EXPORT_SCRIPT) $(COEFF_JSON)

# Step 2: Generate C++ header from JSON
$(HEADER): $(COEFF_JSON) $(HEADER_GEN)
	@echo "Generating C++ header..."
	$(PYTHON) $(HEADER_GEN) $(COEFF_JSON) $(HEADER)

# Step 3: Build executables
test_benchmark: test_benchmark.cpp $(HEADER)
	@echo "Compiling test suite..."
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

benchmark_comparison: benchmark_comparison.cpp $(HEADER)
	@echo "Compiling benchmark..."
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

example_usage: example_usage.cpp $(HEADER)
	@echo "Compiling examples..."
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test_simple: test_simple.cpp $(HEADER)
	@echo "Compiling simple test..."
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

# Run all tests
test: test_benchmark
	@echo "Running test suite..."
	./test_benchmark

# Force regeneration from scratch
regenerate:
	@echo "Regenerating from source..."
	rm -f $(COEFF_JSON) $(HEADER)
	$(MAKE) all

# Clean all generated files
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGETS) $(HEADER) $(COEFF_JSON) *.o

# Display help
help:
	@echo "Available targets:"
	@echo "  all          - Build all programs (default)"
	@echo "  test         - Build and run test suite"
	@echo "  regenerate   - Rebuild coefficients and header from scratch"
	@echo "  clean        - Remove all generated files"
	@echo ""
	@echo "Build process:"
	@echo "  1. Python generates coefficients (export_coefficients.py)"
	@echo "  2. Python creates C++ header (json_to_header.py)"
	@echo "  3. C++ compiler builds executables"
	@echo ""
	@echo "Requirements:"
	@echo "  - Python 3.6+ with NumPy and SciPy"
	@echo "  - C++17 compatible compiler"
