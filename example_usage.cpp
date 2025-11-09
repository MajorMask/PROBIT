#include "InverseCumulativeNormal.h"
#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;
using namespace quant;

int main() {
    cout << "======================================================================\n";
    cout << "       Inverse Cumulative Normal (Probit) - Usage Examples\n";
    cout << "======================================================================\n\n";
    
    // Example 1: Standard normal quantiles
    cout << "Example 1: Standard Normal Quantiles (mean=0, sigma=1)\n";
    cout << "--------------------------------------------------------\n";
    
    InverseCumulativeNormal icn_standard;
    
    struct { const char* name; double prob; } quantiles[] = {
        {"1st percentile  ", 0.01},
        {"2.5th percentile", 0.025},
        {"5th percentile  ", 0.05},
        {"10th percentile ", 0.10},
        {"25th percentile ", 0.25},
        {"Median (50th)   ", 0.50},
        {"75th percentile ", 0.75},
        {"90th percentile ", 0.90},
        {"95th percentile ", 0.95},
        {"97.5th percentile", 0.975},
        {"99th percentile ", 0.99},
    };
    
    cout << fixed << setprecision(6);
    for (const auto& q : quantiles) {
        double z = icn_standard(q.prob);
        cout << q.name << " (p=" << q.prob << "): z = " << setw(10) << z << "\n";
    }
    
    // Example 2: Custom distribution
    cout << "\n\nExample 2: Custom Normal Distribution (mean=100, sigma=15)\n";
    cout << "-----------------------------------------------------------\n";
    
    InverseCumulativeNormal icn_custom(100.0, 15.0);
    
    cout << "Probability  IQ Score\n";
    cout << "-----------  --------\n";
    double probs[] = {0.01, 0.05, 0.10, 0.25, 0.50, 0.75, 0.90, 0.95, 0.99};
    for (double p : probs) {
        double iq = icn_custom(p);
        cout << "   " << setw(4) << int(p * 100) << "%     " << setw(6) << iq << "\n";
    }
    
    // Example 3: Vector usage for batch processing
    cout << "\n\nExample 3: Vector/Batch Processing\n";
    cout << "------------------------------------\n";
    
    const double probabilities[] = {0.001, 0.01, 0.1, 0.3, 0.5, 0.7, 0.9, 0.99, 0.999};
    double z_values[9];
    
    icn_standard(probabilities, z_values, 9);
    
    cout << "Probability     Z-score\n";
    cout << "-----------     -------\n";
    for (size_t i = 0; i < 9; ++i) {
        cout << setw(8) << probabilities[i] << "   " << setw(10) << z_values[i] << "\n";
    }
    
    // Example 4: Extreme tails
    cout << "\n\nExample 4: Extreme Tail Behavior\n";
    cout << "---------------------------------\n";
    
    cout << scientific << setprecision(3);
    double extreme_probs[] = {1e-12, 1e-9, 1e-6, 1e-3, 1-1e-3, 1-1e-6, 1-1e-9, 1-1e-12};
    
    cout << "Probability         Z-score\n";
    cout << "-----------         -------\n";
    for (double p : extreme_probs) {
        double z = icn_standard(p);
        cout << setw(11) << p << "   " << fixed << setw(10) << setprecision(6) << z << "\n" << scientific;
    }
    
    // Example 5: Finance application - Value at Risk
    cout << fixed << setprecision(2);
    cout << "\n\nExample 5: Value at Risk (VaR) Calculation\n";
    cout << "-------------------------------------------\n";
    
    double portfolio_value = 1000000.0;  // $1M portfolio
    double daily_volatility = 0.02;       // 2% daily volatility
    double confidence_levels[] = {0.95, 0.99, 0.995};
    
    cout << "Portfolio Value: $" << portfolio_value << "\n";
    cout << "Daily Volatility: " << daily_volatility * 100 << "%\n\n";
    cout << "Confidence Level   VaR (1-day)\n";
    cout << "----------------   -----------\n";
    
    InverseCumulativeNormal var_calc(0.0, daily_volatility * portfolio_value);
    
    for (double conf : confidence_levels) {
        // VaR is the loss at the (1-conf) quantile
        double var = -var_calc(1.0 - conf);
        cout << "    " << conf * 100 << "%         $" << setw(10) << var << "\n";
    }
    
    // Example 6: Monte Carlo - Transform uniform to normal
    cout << "\n\nExample 6: Monte Carlo Transformation\n";
    cout << "--------------------------------------\n";
    cout << "Transforming uniform random to normal distribution:\n\n";
    
    // Simulate some uniform random values
    double uniform_samples[] = {0.1234, 0.5678, 0.9012, 0.3456, 0.7890};
    double normal_samples[5];
    
    icn_standard(uniform_samples, normal_samples, 5);
    
    cout << "Uniform [0,1]   Normal ~ N(0,1)\n";
    cout << "-------------   ---------------\n";
    for (size_t i = 0; i < 5; ++i) {
        cout << setw(8) << setprecision(4) << uniform_samples[i] 
             << "        " << setw(10) << setprecision(6) << normal_samples[i] << "\n";
    }
    
    cout << "\n======================================================================\n";
    cout << "For more information, see README.md and DESIGN.md\n";
    cout << "======================================================================\n";
    
    return 0;
}
