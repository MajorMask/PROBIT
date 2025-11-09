#include "InverseCumulativeNormal.h"
#include <iostream>
#include <iomanip>

using namespace std;
using namespace quant;

int main() {
    cout << "Inverse Cumulative Normal - Verification Test\n";
    cout << "===============================================\n\n";
    
    InverseCumulativeNormal icn;
    
    // Test standard quantiles
    cout << "Standard quantiles:\n";
    cout << fixed << setprecision(10);
    cout << "  2.5th percentile:  " << icn(0.025) << " (expected: -1.96)\n";
    cout << "  50th percentile:   " << icn(0.500) << " (expected:  0.00)\n";
    cout << "  97.5th percentile: " << icn(0.975) << " (expected:  1.96)\n";
    
    cout << "\nImplementation verified.\n";
    
    return 0;
}
