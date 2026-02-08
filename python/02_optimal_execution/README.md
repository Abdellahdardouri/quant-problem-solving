# Optimal Execution: Almgren-Chriss Framework

Solve the optimal execution problem for large orders using the Almgren-Chriss mean-variance framework.

## Problem

Execute X shares over time horizon T while minimizing:
- **Market impact costs** (permanent + temporary)
- **Timing risk** (variance from price uncertainty)

## Almgren-Chriss Model

**Objective**:
```
min E[cost] + λ × Var[cost]
```

**Market impact**:
- Permanent: γ Σx_j (cumulative volume)
- Temporary: ε Σx_j² (quadratic in trade size)

**Solution**: Optimal trajectory follows `n(t) ∝ sinh(κ(T-t))`

## Key Parameters

- `λ`: Risk aversion (higher → faster execution)
- `γ`: Permanent impact coefficient
- `ε`: Temporary impact coefficient
- `σ`: Volatility (timing risk)

## Results

- **Optimal vs TWAP**: Typically 5-15% cost reduction
- **Trajectory shape**: Exponential decay (front-loaded)
- **Performance**: Validated via Monte Carlo simulation

## Files

- `optimal_execution.ipynb`: Full implementation
- `execution_strategies.png`: Trajectory comparison
- `risk_analysis.png`: Efficient frontier
- `cost_distribution.png`: Monte Carlo results

## Usage

```python
# Compute optimal trajectory
kappa = sqrt(lambda_risk * sigma² / epsilon)
n(t) = X * sinh(kappa(T-t)) / sinh(kappa*T)

# Get trade sizes
x_j = n(t_j) - n(t_{j+1})
```

## Extensions

- **Time-varying volatility**: Adjust σ(t) intraday
- **Limit orders**: Model fill probability
- **Multi-asset execution**: Correlation matters
- **RL approaches**: Adaptive strategies

## Production Tips

- Estimate γ, ε from historical execution data
- Update parameters throughout the day (volatility changes)
- Monitor slippage vs model predictions
- Combine with alpha signals for timing
