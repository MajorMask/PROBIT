# ...existing code...
CXX = g++
CXXFLAGS = -std=c++17 -O3 -march=native -Wall -Wextra
LDFLAGS = -lm
PYTHON = python3
PIP = pip3
PY_DEPS = numpy scipy
BREW_PKGS = gcc

# Generated files
COEFF_JSON = coefficients.json
HEADER = InverseCumulativeNormal.h

# Source files
EXPORT_SCRIPT = export_coefficients.py
HEADER_GEN = json_to_header.py

# Executables
TARGETS = test_benchmark benchmark_comparison example_usage test_simple

.PHONY: all test clean regenerate help install install_python_deps install_system_deps

UNAME_S := $(shell uname -s)

# Install Python deps into a local virtualenv (.venv) to avoid system package issues
install_python_deps:
	@echo "Ensuring Python and creating virtualenv (.venv) if needed..."
	@if ! command -v $(PYTHON) >/dev/null 2>&1; then \
	    echo "python3 not found. Install Python 3 and retry."; \
	    exit 1; \
	fi; \
	if [ ! -d .venv ]; then \
	    echo "Creating virtualenv at .venv"; \
	    $(PYTHON) -m venv .venv; \
	fi; \
	.venv/bin/python -m pip install --upgrade pip; \
	.venv/bin/python -m pip install $(PY_DEPS);
	@echo "Python deps installed into .venv. Activate with: source .venv/bin/activate"

# Minimal system deps check and guidance (no destructive installs)
install_system_deps:
	@echo "Checking for C++ toolchain..."
	@if command -v g++ >/dev/null 2>&1; then \
	    echo "g++ found."; \
	    exit 0; \
	fi; \
	echo "g++ not found on this system."; \
	case "$(UNAME_S)" in \
	    Darwin) \
	        echo "On macOS: install Xcode command line tools (xcode-select --install) or Homebrew and run 'brew install gcc'."; \
	        ;; \
	    Linux) \
	        echo "On Linux: use your distro package manager, e.g. 'sudo apt-get install build-essential' or 'sudo dnf install gcc-c++'."; \
	        ;; \
	    MINGW*|MSYS*|CYGWIN*|Windows_NT) \
	        echo "On Windows: install MSYS2/MinGW or Visual Studio (C++)."; \
	        ;; \
	    *) \
	        echo "Unsupported platform: $(UNAME_S). Install a C++ compiler manually."; \
	        ;; \
	esac;
	@false

install: install_system_deps install_python_deps
	@echo "All dependency checks complete."

# Default target: build all executables
all: $(TARGETS)

# Dependency chain: Python → JSON → Header → Executables
$(COEFF_JSON): $(EXPORT_SCRIPT)
	@echo "Generating coefficients..."
	.venv/bin/python $(EXPORT_SCRIPT) $(COEFF_JSON)

$(HEADER): $(COEFF_JSON) $(HEADER_GEN)
	@echo "Generating C++ header..."
	.venv/bin/python $(HEADER_GEN) $(COEFF_JSON) $(HEADER)

# Build executables
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

# Run tests
test: test_benchmark
	@echo "Running test suite..."
	./test_benchmark

regenerate:
	@echo "Regenerating from source..."
	rm -f $(COEFF_JSON) $(HEADER)
	$(MAKE) all

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(TARGETS) $(HEADER) $(COEFF_JSON) *.o
help:
	@echo "Available targets:"
	@echo "  all                  - Build all programs (default)"
	@echo "  install              - Check system C++ toolchain and create .venv with Python deps"
	@echo "  install_python_deps  - Create .venv and install Python deps (numpy, scipy)"
	@echo "  install_system_deps  - Check for C++ toolchain and print install guidance"
	@echo "  test                 - Build and run test suite"
	@echo "  regenerate           - Rebuild coefficients and header from scratch"
	@echo "  clean                - Remove all generated files"
# ...existing code...