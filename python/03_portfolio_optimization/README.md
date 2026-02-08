# Mean-Variance Portfolio Optimization

Construct optimal portfolios using Markowitz mean-variance optimization with realistic constraints.

## Problem

Find portfolio weights that maximize risk-adjusted return (Sharpe ratio):

```
max (w^T μ - r_f) / √(w^T Σ w)
subject to: Σw_i = 1, w_i ≥ 0, additional constraints
```

## Methods Implemented

1. **Efficient Frontier**: Min variance for target return levels
2. **Maximum Sharpe**: Tangency portfolio  
3. **Minimum Variance**: Global minimum risk
4. **Constrained**: Position limits + sector constraints

## Key Challenges

- **Estimation error**: Covariance matrix is noisy
- **Concentration**: Solutions often highly concentrated
- **Instability**: Small input changes → large weight shifts
- **Turnover**: Rebalancing costs can exceed gains

## Solutions

- Regularization (L2 penalty on weights)
- Robust optimization (worst-case scenarios)
- Shrinkage estimators (Ledoit-Wolf)
- Black-Litterman (incorporate views)

## Results

- Max Sharpe typically outperforms equal-weight by 20-40%
- Min variance has ~30% lower volatility
- Constraints reduce concentration but hurt Sharpe
- Risk contribution analysis reveals hidden exposures

## Files

- `portfolio_optimization.ipynb`: Full implementation
- `portfolio_optimization.png`: Efficient frontier + allocations
- `risk_contribution.png`: Risk decomposition

## Usage

```python
# Maximum Sharpe portfolio
def neg_sharpe(w):
    return -(w.T @ mu - rf) / sqrt(w.T @ Sigma @ w)

result = minimize(neg_sharpe, w0, bounds=bounds, constraints=constraints)
optimal_weights = result.x
```

## Advanced Topics

- **Risk parity**: Equalize risk contribution
- **Factor models**: Optimize on factor exposures
- **Transaction costs**: Include turnover penalty
- **Hierarchical clustering**: Group correlated assets
- **Backtesting**: Out-of-sample performance

## Production Workflow

1. Clean data (outliers, corporate actions)
2. Estimate expected returns (CAPM, factor models, ML)
3. Estimate covariance (shrinkage, factor decomposition)
4. Optimize with constraints (position, sector, turnover)
5. Backtest on historical data
6. Monitor live performance and rebalance
