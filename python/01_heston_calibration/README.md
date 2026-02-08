# Heston Model Calibration

Calibrate the Heston stochastic volatility model to market-observed option prices using the characteristic function method.

## Problem

Given market option prices across strikes, find Heston model parameters that best fit the observed volatility surface.

## Heston Model

```
dS_t = μ S_t dt + √v_t S_t dW_t^S
dv_t = κ(θ - v_t)dt + σ√v_t dW_t^v
Corr(dW_t^S, dW_t^v) = ρ dt
```

**Parameters to calibrate**:
- `v0`: Initial variance
- `κ`: Mean reversion speed
- `θ`: Long-term variance
- `σ`: Volatility of volatility  
- `ρ`: Correlation (leverage effect)

## Method

1. **Pricing via characteristic function**: Fast Fourier transform approach
2. **Optimization**: Levenberg-Marquardt least squares
3. **Constraints**: Feller condition (2κθ > σ²) ensures variance stays positive
4. **Regularization**: Prevent overfitting to noisy data

## Key Results

- Typical calibration: ~2 seconds for 10 strikes
- RMSE < 1% of option value
- Recover volatility smile (skew + term structure)

## Files

- `heston_calibration.ipynb`: Main notebook with full implementation
- `heston_calibration_results.png`: Price comparison plot
- `volatility_smile.png`: Implied volatility curve

## Usage

```python
# Calibrate Heston model
result = least_squares(objective_function, initial_guess, bounds=bounds)
v0, kappa, theta, sigma, rho = result.x

# Price options with calibrated parameters
price = heston_call_price(K, S0, v0, kappa, theta, sigma, rho, T, r)
```

## Production Considerations

- **FFT methods**: Carr-Madan for 10-100x speedup
- **Stability**: Use log-price formulation to avoid numerical issues
- **Real-time recalibration**: Update as new market quotes arrive
- **Model risk**: Heston assumes log-normal, fails in extreme tails
