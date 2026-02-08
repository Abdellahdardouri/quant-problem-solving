/**
 * @file monte_carlo.cpp
 * @brief High-performance Monte Carlo option pricing with OpenMP parallelization
 * 
 * Features:
 * - Multi-threaded random number generation
 * - Variance reduction techniques (antithetic variates)
 * - European, Asian, and Barrier options
 * - Performance benchmarking
 * 
 * Compile: g++ -std=c++17 -O3 -fopenmp -o monte_carlo monte_carlo.cpp
 * Run: ./monte_carlo
 */

#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <omp.h>

// Option payoff functions
namespace Payoffs {
    // European Call option
    double europeanCall(double S, double K) {
        return std::max(S - K, 0.0);
    }
    
    // European Put option
    double europeanPut(double S, double K) {
        return std::max(K - S, 0.0);
    }
    
    // Asian Call (arithmetic average)
    double asianCall(const std::vector<double>& path, double K) {
        double avg = 0.0;
        for (double S : path) avg += S;
        avg /= path.size();
        return std::max(avg - K, 0.0);
    }
    
    // Barrier Down-and-Out Call
    double barrierDownOutCall(const std::vector<double>& path, double K, double barrier) {
        // Check if barrier was hit
        for (double S : path) {
            if (S <= barrier) return 0.0;
        }
        return std::max(path.back() - K, 0.0);
    }
}

class MonteCarloEngine {
private:
    double S0;      // Initial stock price
    double K;       // Strike price
    double T;       // Time to maturity
    double r;       // Risk-free rate
    double sigma;   // Volatility
    int n_paths;    // Number of Monte Carlo paths
    int n_steps;    // Time steps per path
    
public:
    MonteCarloEngine(double S0_, double K_, double T_, double r_, double sigma_, 
                     int n_paths_, int n_steps_ = 252)
        : S0(S0_), K(K_), T(T_), r(r_), sigma(sigma_), 
          n_paths(n_paths_), n_steps(n_steps_) {}
    
    /**
     * @brief Generate stock price path using geometric Brownian motion
     * @param path Output vector to store price path
     * @param rng Random number generator
     * @param antithetic If true, use antithetic variate
     */
    void generatePath(std::vector<double>& path, std::mt19937& rng, bool antithetic = false) {
        std::normal_distribution<double> dist(0.0, 1.0);
        
        double dt = T / n_steps;
        double drift = (r - 0.5 * sigma * sigma) * dt;
        double diffusion = sigma * std::sqrt(dt);
        
        path[0] = S0;
        
        for (int i = 1; i <= n_steps; ++i) {
            double Z = dist(rng);
            if (antithetic) Z = -Z;  // Antithetic variate
            
            double S_prev = path[i - 1];
            path[i] = S_prev * std::exp(drift + diffusion * Z);
        }
    }
    
    /**
     * @brief Price European option using standard Monte Carlo
     * @param option_type "call" or "put"
     * @return Option price
     */
    double priceEuropean(const std::string& option_type) {
        double payoff_sum = 0.0;
        
        #pragma omp parallel
        {
            // Thread-local random number generator
            std::mt19937 rng(std::random_device{}() + omp_get_thread_num());
            std::vector<double> path(n_steps + 1);
            double local_sum = 0.0;
            
            #pragma omp for
            for (int i = 0; i < n_paths; ++i) {
                generatePath(path, rng);
                double S_T = path.back();
                
                double payoff = (option_type == "call") ? 
                    Payoffs::europeanCall(S_T, K) : Payoffs::europeanPut(S_T, K);
                
                local_sum += payoff;
            }
            
            #pragma omp atomic
            payoff_sum += local_sum;
        }
        
        double option_price = std::exp(-r * T) * (payoff_sum / n_paths);
        return option_price;
    }
    
    /**
     * @brief Price European option with antithetic variance reduction
     */
    double priceEuropeanAntithetic(const std::string& option_type) {
        double payoff_sum = 0.0;
        int half_paths = n_paths / 2;
        
        #pragma omp parallel
        {
            std::mt19937 rng(std::random_device{}() + omp_get_thread_num());
            std::vector<double> path(n_steps + 1);
            std::vector<double> path_anti(n_steps + 1);
            double local_sum = 0.0;
            
            #pragma omp for
            for (int i = 0; i < half_paths; ++i) {
                // Regular path
                generatePath(path, rng, false);
                double S_T = path.back();
                double payoff1 = (option_type == "call") ? 
                    Payoffs::europeanCall(S_T, K) : Payoffs::europeanPut(S_T, K);
                
                // Antithetic path
                generatePath(path_anti, rng, true);
                double S_T_anti = path_anti.back();
                double payoff2 = (option_type == "call") ? 
                    Payoffs::europeanCall(S_T_anti, K) : Payoffs::europeanPut(S_T_anti, K);
                
                local_sum += (payoff1 + payoff2) / 2.0;
            }
            
            #pragma omp atomic
            payoff_sum += local_sum;
        }
        
        double option_price = std::exp(-r * T) * (payoff_sum / half_paths);
        return option_price;
    }
    
    /**
     * @brief Price Asian option
     */
    double priceAsian() {
        double payoff_sum = 0.0;
        
        #pragma omp parallel
        {
            std::mt19937 rng(std::random_device{}() + omp_get_thread_num());
            std::vector<double> path(n_steps + 1);
            double local_sum = 0.0;
            
            #pragma omp for
            for (int i = 0; i < n_paths; ++i) {
                generatePath(path, rng);
                double payoff = Payoffs::asianCall(path, K);
                local_sum += payoff;
            }
            
            #pragma omp atomic
            payoff_sum += local_sum;
        }
        
        double option_price = std::exp(-r * T) * (payoff_sum / n_paths);
        return option_price;
    }
    
    /**
     * @brief Price Barrier option
     */
    double priceBarrier(double barrier) {
        double payoff_sum = 0.0;
        
        #pragma omp parallel
        {
            std::mt19937 rng(std::random_device{}() + omp_get_thread_num());
            std::vector<double> path(n_steps + 1);
            double local_sum = 0.0;
            
            #pragma omp for
            for (int i = 0; i < n_paths; ++i) {
                generatePath(path, rng);
                double payoff = Payoffs::barrierDownOutCall(path, K, barrier);
                local_sum += payoff;
            }
            
            #pragma omp atomic
            payoff_sum += local_sum;
        }
        
        double option_price = std::exp(-r * T) * (payoff_sum / n_paths);
        return option_price;
    }
};

// Black-Scholes analytical formula (for comparison)
double blackScholesCall(double S0, double K, double T, double r, double sigma) {
    double d1 = (std::log(S0 / K) + (r + 0.5 * sigma * sigma) * T) / (sigma * std::sqrt(T));
    double d2 = d1 - sigma * std::sqrt(T);
    
    auto norm_cdf = [](double x) {
        return 0.5 * std::erfc(-x * M_SQRT1_2);
    };
    
    return S0 * norm_cdf(d1) - K * std::exp(-r * T) * norm_cdf(d2);
}

int main() {
    std::cout << "=== Monte Carlo Option Pricing ===" << std::endl;
    std::cout << "Compiled with OpenMP support" << std::endl;
    std::cout << "Number of threads: " << omp_get_max_threads() << std::endl << std::endl;
    
    // Market parameters
    double S0 = 100.0;      // Current stock price
    double K = 100.0;       // Strike price
    double T = 1.0;         // Time to maturity (1 year)
    double r = 0.05;        // Risk-free rate (5%)
    double sigma = 0.20;    // Volatility (20%)
    
    std::cout << "Market Parameters:" << std::endl;
    std::cout << "  S0 = $" << S0 << std::endl;
    std::cout << "  K = $" << K << std::endl;
    std::cout << "  T = " << T << " years" << std::endl;
    std::cout << "  r = " << r * 100 << "%" << std::endl;
    std::cout << "  σ = " << sigma * 100 << "%" << std::endl << std::endl;
    
    // Black-Scholes analytical price
    double bs_price = blackScholesCall(S0, K, T, r, sigma);
    std::cout << "Black-Scholes Call Price: $" << std::fixed << std::setprecision(4) << bs_price << std::endl << std::endl;
    
    // Monte Carlo simulation parameters
    std::vector<int> path_counts = {100000, 1000000, 10000000};
    int n_steps = 252;  // Daily steps
    
    std::cout << "=== European Call Option ===" << std::endl;
    std::cout << std::setw(15) << "Paths" 
              << std::setw(15) << "MC Price" 
              << std::setw(15) << "Error" 
              << std::setw(15) << "Time (ms)" 
              << std::setw(20) << "Paths/sec" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (int n_paths : path_counts) {
        MonteCarloEngine engine(S0, K, T, r, sigma, n_paths, n_steps);
        
        auto start = std::chrono::high_resolution_clock::now();
        double mc_price = engine.priceEuropean("call");
        auto end = std::chrono::high_resolution_clock::now();
        
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        double error = std::abs(mc_price - bs_price);
        double paths_per_sec = n_paths / (duration.count() / 1000.0);
        
        std::cout << std::setw(15) << n_paths
                  << std::setw(15) << mc_price
                  << std::setw(15) << error
                  << std::setw(15) << duration.count()
                  << std::setw(20) << std::scientific << paths_per_sec << std::fixed
                  << std::endl;
    }
    
    // Test antithetic variance reduction
    std::cout << std::endl << "=== Variance Reduction (Antithetic Variates) ===" << std::endl;
    int n_paths_test = 1000000;
    MonteCarloEngine engine_test(S0, K, T, r, sigma, n_paths_test, n_steps);
    
    auto start_std = std::chrono::high_resolution_clock::now();
    double price_std = engine_test.priceEuropean("call");
    auto end_std = std::chrono::high_resolution_clock::now();
    auto duration_std = std::chrono::duration_cast<std::chrono::milliseconds>(end_std - start_std);
    
    auto start_anti = std::chrono::high_resolution_clock::now();
    double price_anti = engine_test.priceEuropeanAntithetic("call");
    auto end_anti = std::chrono::high_resolution_clock::now();
    auto duration_anti = std::chrono::duration_cast<std::chrono::milliseconds>(end_anti - start_anti);
    
    std::cout << "Standard MC:   Price = $" << price_std << ", Error = $" << std::abs(price_std - bs_price) << std::endl;
    std::cout << "Antithetic MC: Price = $" << price_anti << ", Error = $" << std::abs(price_anti - bs_price) << std::endl;
    std::cout << "Variance reduction improves accuracy by " 
              << std::setprecision(2) << ((std::abs(price_std - bs_price) - std::abs(price_anti - bs_price)) / std::abs(price_std - bs_price) * 100)
              << "%" << std::endl << std::endl;
    
    // Test exotic options
    std::cout << "=== Exotic Options ===" << std::endl;
    MonteCarloEngine engine_exotic(S0, K, T, r, sigma, 1000000, n_steps);
    
    double asian_price = engine_exotic.priceAsian();
    std::cout << "Asian Call Option: $" << std::setprecision(4) << asian_price << std::endl;
    
    double barrier = 90.0;
    double barrier_price = engine_exotic.priceBarrier(barrier);
    std::cout << "Barrier Down-and-Out Call (Barrier=$" << barrier << "): $" << barrier_price << std::endl;
    
    return 0;
}
