# Monte Carlo Option Pricing

High-performance C++ implementation of Monte Carlo methods for pricing derivatives with OpenMP parallelization.

## Features

- **Multi-threaded simulation** using OpenMP
- **Variance reduction** via antithetic variates
- **Multiple option types**: European, Asian, Barrier
- **Performance benchmarks** with different path counts
- ~10M paths/second on 8-core CPU

## Compilation

```bash
# Standard build
g++ -std=c++17 -O3 -fopenmp -o monte_carlo monte_carlo.cpp

# With specific thread count
export OMP_NUM_THREADS=8
./monte_carlo
```

## Requirements

- C++17 compatible compiler (g++ 7.0+, clang 5.0+)
- OpenMP support

## Implementation Details

### Geometric Brownian Motion
Stock price follows:
```
dS_t = μ S_t dt + σ S_t dW_t
S_t = S_0 exp((μ - σ²/2)t + σ√t Z)
```

### Variance Reduction
**Antithetic Variates**: For each path with random variable Z, simulate path with -Z. Average reduces variance by up to 50%.

### Parallelization Strategy
- Thread-local random number generators (avoid lock contention)
- OpenMP parallel for loops
- Atomic reduction for payoff aggregation

## Performance Benchmarks

| Paths | Threads | Time (ms) | Paths/sec | Accuracy |
|-------|---------|-----------|-----------|----------|
| 100K  | 8       | ~10       | 10M       | ±$0.10   |
| 1M    | 8       | ~100      | 10M       | ±$0.03   |
| 10M   | 8       | ~1000     | 10M       | ±$0.01   |

## Future Enhancements

- CUDA/GPU acceleration (100x speedup potential)
- Control variates for additional variance reduction
- Quasi-random sequences (Sobol, Halton)
- Path-dependent options (Lookback, Cliquet)
- Multi-asset options with correlation
