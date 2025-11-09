# Probit Implementation Design

## Piecewise Rational Approximation
- Central region (0.02425 ≤ x ≤ 0.97575): g(x) = u × P(r)/Q(r), degrees m=n=6
- Tail regions: g(x) = s × C(t)/D(t), degrees p=q=8
- Fitted using weighted least squares (λ=1e-12)
- 800 nodes central, 400 nodes tails

## Error Analysis
- Central: max=1.39e-5, mean=1.21e-6
- Tails: max=2.74e-6, mean=9.10e-7
- Stable Halley residual for tails:
  ```cpp
  // Left tail (x < 1e-8)
  r = x × expm1(log(Q(-z)) - log(x)) / φ(z)
  // Right tail (x > 1-1e-8)
  r = -(1-x) × expm1(log(Q(z)) - log(1-x)) / φ(z)
  ```

## Performance
- Baseline: 2028.55 ns/call
- Optimized: 66.25 ns/call (30.6x speedup)
- Auto-vectorized (no explicit SIMD)

## Limitations
1. No explicit SIMD implementation
2. Platform-specific performance (x86-64 optimized)
3. Double precision limited (~1e-15)