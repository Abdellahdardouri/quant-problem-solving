# Quantitative Finance: Algorithmic Problem Solving

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/)
[![Python](https://img.shields.io/badge/Python-3.8+-blue.svg)](https://www.python.org/)

A collection of high-performance implementations for classical algorithmic problems in quantitative finance, featuring both Python and C++ solutions with focus on computational efficiency and numerical accuracy.

## 🎯 Overview

This repository demonstrates practical solutions to core problems faced by quantitative developers in modern finance, including:

- **Derivatives Pricing** - Monte Carlo methods with variance reduction
- **Model Calibration** - Stochastic volatility models (Heston)
- **Optimal Execution** - Almgren-Chriss framework for smart order routing
- **Portfolio Optimization** - Mean-variance with realistic constraints
- **Market Microstructure** - Order book simulation and modeling

## 📂 Repository Structure

```
quant-algorithmic-problems/
├── python/                          # Python implementations (Jupyter notebooks)
│   ├── 01_heston_calibration/      # Calibrate Heston model to market data
│   ├── 02_optimal_execution/       # Almgren-Chriss optimal execution
│   └── 03_portfolio_optimization/  # Mean-variance portfolio construction
│
├── cpp/                             # C++ high-performance implementations
│   ├── monte_carlo_pricing/        # Multi-threaded option pricing
│   └── order_book_simulator/       # Limit order book engine
│
├── data/                            # Sample datasets
└── requirements.txt                 # Python dependencies
```

## 🚀 Quick Start

### Python Notebooks

```bash
# Install dependencies
pip install -r requirements.txt

# Launch Jupyter
jupyter notebook

# Navigate to python/ directory and open any notebook
```

### C++ Programs

```bash
# Monte Carlo Pricing (requires OpenMP)
cd cpp/monte_carlo_pricing
g++ -std=c++17 -O3 -fopenmp -o monte_carlo monte_carlo.cpp
./monte_carlo

# Order Book Simulator
cd cpp/order_book_simulator
g++ -std=c++17 -O3 -o orderbook orderbook.cpp
./orderbook
```

## 📊 Problem Details

### 1. Heston Model Calibration (Python)
**Problem**: Calibrate the Heston stochastic volatility model to market-observed option prices

**Approach**:
- Non-linear least squares optimization
- Characteristic function method for pricing
- Regularization to ensure parameter stability
- Visualization of calibrated volatility surface

**Key Techniques**: FFT pricing, scipy.optimize, numerical stability handling

---

### 2. Optimal Execution - Almgren-Chriss (Python)
**Problem**: Execute large orders while minimizing market impact and timing risk

**Approach**:
- Solve the Almgren-Chriss optimal execution problem
- Balance permanent vs. temporary market impact
- Compute optimal trading trajectory
- Backtest against real execution strategies

**Key Techniques**: Quadratic programming, mean-variance optimization, market impact modeling

---

### 3. Portfolio Optimization (Python)
**Problem**: Construct mean-variance optimal portfolios with realistic constraints

**Approach**:
- Efficient frontier computation
- Transaction cost modeling
- Position limits and sector constraints
- Risk parity alternative
- Sharpe ratio maximization

**Key Techniques**: cvxpy/scipy optimization, covariance estimation, constraint handling

---

### 4. Monte Carlo Option Pricing (C++)
**Problem**: Price exotic derivatives using Monte Carlo simulation with high performance

**Approach**:
- Multi-threaded random number generation (MT19937)
- OpenMP parallelization for path simulation
- Variance reduction techniques (antithetic variates, control variates)
- European, Asian, Barrier options

**Key Techniques**: Parallel computing, SIMD optimization, statistical variance reduction

**Performance**: ~10M paths/second on 8-core CPU

---

### 5. Order Book Simulator (C++)
**Problem**: Build a low-latency limit order book matching engine

**Approach**:
- Price-time priority matching
- Efficient data structures (std::map for price levels, deque for orders)
- Order types: Market, Limit, Cancel
- Real-time trade execution

**Key Techniques**: STL containers, event-driven architecture, cache-friendly design

**Performance**: ~100K order operations/second

---

## 🧪 Key Algorithms & Techniques

| Problem Domain | Algorithms | Implementation |
|---------------|------------|----------------|
| **Derivatives Pricing** | Monte Carlo, Finite Differences, FFT | C++, Python |
| **Calibration** | Levenberg-Marquardt, Gradient Descent | Python (scipy) |
| **Optimization** | Quadratic Programming, Interior Point | Python (cvxpy) |
| **HPC** | OpenMP, SIMD, Cache Optimization | C++ |
| **Statistics** | Variance Reduction, MLE, Bayesian | Python (NumPy) |

## 📈 Performance Benchmarks

| Implementation | Language | Performance | Notes |
|---------------|----------|-------------|-------|
| Monte Carlo Pricing | C++ (OpenMP) | 10M paths/sec | 8 cores, AVX2 |
| Heston Calibration | Python | ~2s per calibration | 100 strikes, scipy.optimize |
| Order Book | C++ | 100K ops/sec | Single-threaded |
| Portfolio Optimization | Python | <1s | 100 assets, cvxpy |

## 🛠️ Technologies

**Python Stack**:
- NumPy, SciPy, Pandas - Numerical computing
- Matplotlib, Seaborn - Visualization
- cvxpy - Convex optimization
- Jupyter - Interactive development

**C++ Stack**:
- C++17 standard
- OpenMP - Parallel computing
- STL - Data structures
- g++/clang - Compilation

## 📚 References

**Books**:
- *Options, Futures, and Other Derivatives* - John Hull
- *Algorithmic and High-Frequency Trading* - Cartea, Jaimungal, Penalva
- *Optimal Trading Strategies* - Kissell

**Papers**:
- Almgren & Chriss (2000) - Optimal Execution of Portfolio Transactions
- Heston (1993) - A Closed-Form Solution for Options with Stochastic Volatility
- Gatheral (2006) - The Volatility Surface

## 🎓 Author

**Abdellah Dardouri**
- Master's in High Performance Computing & Simulation (CHPS) - university of paris saclay
- Engineering in Cloud & Distributed Systems - INPT 


## 📝 License

MIT License - see LICENSE file for details

## 🤝 Contributing

Contributions welcome! Feel free to:
- Add new problem implementations
- Optimize existing solutions
- Improve documentation
- Report bugs or suggest features

---

**Note**: This repository is for educational purposes. Real production systems require additional considerations for risk management, regulatory compliance, and operational robustness.
