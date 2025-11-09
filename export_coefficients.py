#!/usr/bin/env python3
"""
Export coefficients to JSON for C++ consumption.
This is the cleanest bridge between Python and C++.
"""

import json
import numpy as np
from scipy.stats import norm
from datetime import datetime

def fit_central_region(m=6, n=6, num_samples=800):
    """Fit central region coefficients"""
    x_uniform = np.linspace(1e-6, 1-1e-6, num_samples)
    x_near_low = np.linspace(0.02, 0.025, 50)
    x_near_high = np.linspace(0.975, 0.98, 50)
    x_samples = np.concatenate([x_uniform, x_near_low, x_near_high])
    x_samples = np.unique(np.clip(x_samples, 1e-10, 1-1e-10))
    
    mask = (x_samples >= 0.02425) & (x_samples <= 1 - 0.02425)
    x_samples = x_samples[mask]
    z_true = norm.ppf(x_samples)
    
    u = x_samples - 0.5
    r = u * u
    
    num_vars = (m + 1) + n
    A = np.zeros((len(x_samples), num_vars))
    
    for i, (ui, ri, zi) in enumerate(zip(u, r, z_true)):
        for j in range(m + 1):
            A[i, j] = ui * (ri ** j)
        for k in range(1, n + 1):
            A[i, m + 1 + k - 1] = -zi * (ri ** k)
    
    b = z_true
    weights = np.ones_like(x_samples)
    boundary_dist = np.minimum(x_samples - 0.02425, (1 - 0.02425) - x_samples)
    weights[boundary_dist < 0.01] *= 3.0
    W = np.diag(weights)
    
    lambda_ridge = 1e-12
    AW = W @ A
    bW = W @ b
    AtA = AW.T @ AW + lambda_ridge * np.eye(num_vars)
    Atb = AW.T @ bW
    
    theta = np.linalg.solve(AtA, Atb)
    a_coeffs = theta[:m+1]
    b_coeffs = np.concatenate([[1.0], theta[m+1:]])
    
    # Compute error
    z_approx = np.zeros_like(z_true)
    for i, (ui, ri) in enumerate(zip(u, r)):
        P = sum(a_coeffs[j] * (ri ** j) for j in range(m + 1))
        Q = sum(b_coeffs[k] * (ri ** k) for k in range(n + 1))
        z_approx[i] = ui * P / Q
    
    errors = np.abs(z_true - z_approx)
    
    return {
        'coefficients_a': a_coeffs.tolist(),
        'coefficients_b': b_coeffs.tolist(),
        'degree_m': m,
        'degree_n': n,
        'max_error': float(np.max(errors)),
        'mean_error': float(np.mean(errors)),
        'num_samples': len(x_samples)
    }

def fit_tail_region(p=8, q=8, num_samples=400):
    """Fit tail region coefficients"""
    x_low_log = np.logspace(-16, np.log10(0.02425/10), 100)
    x_low_linear = np.linspace(0.002, 0.02425, 100)
    x_tail = np.concatenate([x_low_log, x_low_linear])
    x_tail = np.unique(np.clip(x_tail, 1e-16, 0.02425))
    
    z_true = norm.ppf(x_tail)
    m = x_tail
    t = np.sqrt(-2.0 * np.log(m))
    s = -1.0
    
    num_vars = (p + 1) + q
    A = np.zeros((len(x_tail), num_vars))
    
    for i, (ti, zi) in enumerate(zip(t, z_true)):
        for j in range(p + 1):
            A[i, j] = s * (ti ** j)
        for k in range(1, q + 1):
            A[i, p + 1 + k - 1] = -zi * (ti ** k)
    
    b = z_true
    weights = np.ones_like(x_tail)
    weights[x_tail < 1e-12] *= 5.0
    weights[x_tail > 0.02] *= 3.0
    W = np.diag(weights)
    
    lambda_ridge = 1e-12
    AW = W @ A
    bW = W @ b
    AtA = AW.T @ AW + lambda_ridge * np.eye(num_vars)
    Atb = AW.T @ bW
    
    theta = np.linalg.solve(AtA, Atb)
    c_coeffs = theta[:p+1]
    d_coeffs = np.concatenate([[1.0], theta[p+1:]])
    
    # Compute error
    z_approx = np.zeros_like(z_true)
    for i, ti in enumerate(t):
        C = sum(c_coeffs[j] * (ti ** j) for j in range(p + 1))
        D = sum(d_coeffs[k] * (ti ** k) for k in range(q + 1))
        z_approx[i] = s * C / D
    
    errors = np.abs(z_true - z_approx)
    
    return {
        'coefficients_c': c_coeffs.tolist(),
        'coefficients_d': d_coeffs.tolist(),
        'degree_p': p,
        'degree_q': q,
        'max_error': float(np.max(errors)),
        'mean_error': float(np.mean(errors)),
        'num_samples': len(x_tail)
    }

def export_coefficients(output_path='coefficients.json'):
    """Export all coefficients to JSON"""
    
    print("Deriving coefficients...")
    central = fit_central_region(m=6, n=6)
    tail = fit_tail_region(p=8, q=8)
    
    # Create complete configuration
    config = {
        'metadata': {
            'generated_at': datetime.now().isoformat(),
            'version': '1.0',
            'description': 'Rational approximation coefficients for inverse normal CDF'
        },
        'central_region': central,
        'tail_region': tail,
        'parameters': {
            'x_low': 0.02425,
            'x_high': 1.0 - 0.02425
        }
    }
    
    # Write to JSON with nice formatting
    with open(output_path, 'w') as f:
        json.dump(config, f, indent=2)
    
    print(f"\n Coefficients exported to: {output_path}")
    print(f"\n Statistics:")
    print(f"   Central region: m={central['degree_m']}, n={central['degree_n']}")
    print(f"     - Max error: {central['max_error']:.6e}")
    print(f"     - Mean error: {central['mean_error']:.6e}")
    print(f"\n   Tail region: p={tail['degree_p']}, q={tail['degree_q']}")
    print(f"     - Max error: {tail['max_error']:.6e}")
    print(f"     - Mean error: {tail['mean_error']:.6e}")
    
    return config

if __name__ == '__main__':
    import sys
    output = sys.argv[1] if len(sys.argv) > 1 else 'coefficients.json'
    export_coefficients(output)
