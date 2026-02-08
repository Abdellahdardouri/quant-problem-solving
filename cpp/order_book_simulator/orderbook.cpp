/**
 * @file orderbook.cpp
 * @brief High-performance limit order book matching engine
 * 
 * Features:
 * - Price-time priority matching
 * - Market, Limit, and Cancel orders
 * - Efficient data structures (map for price levels, deque for orders)
 * - Real-time trade execution and order book state
 * 
 * Compile: g++ -std=c++17 -O3 -o orderbook orderbook.cpp
 * Run: ./orderbook
 */

#include <iostream>
#include <map>
#include <deque>
#include <memory>
#include <string>
#include <iomanip>
#include <chrono>
#include <random>

// Order types
enum class OrderType {
    LIMIT,
    MARKET
};

enum class Side {
    BUY,
    SELL
};

// Order structure
struct Order {
    uint64_t order_id;
    Side side;
    OrderType type;
    double price;
    uint64_t quantity;
    std::chrono::system_clock::time_point timestamp;
    
    Order(uint64_t id, Side s, OrderType t, double p, uint64_t q)
        : order_id(id), side(s), type(t), price(p), quantity(q),
          timestamp(std::chrono::system_clock::now()) {}
};

// Trade execution record
struct Trade {
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    double price;
    uint64_t quantity;
    std::chrono::system_clock::time_point timestamp;
    
    Trade(uint64_t bid, uint64_t sid, double p, uint64_t q)
        : buy_order_id(bid), sell_order_id(sid), price(p), quantity(q),
          timestamp(std::chrono::system_clock::now()) {}
};

/**
 * @class OrderBook
 * @brief Limit order book with price-time priority matching
 */
class OrderBook {
private:
    // Price level: price -> queue of orders at that price (FIFO for time priority)
    std::map<double, std::deque<std::shared_ptr<Order>>> bids;  // Buy orders (descending)
    std::map<double, std::deque<std::shared_ptr<Order>>, std::greater<double>> asks;  // Sell orders (ascending)
    
    // Order lookup by ID
    std::map<uint64_t, std::shared_ptr<Order>> order_map;
    
    // Trade history
    std::vector<Trade> trade_history;
    
    uint64_t next_order_id = 1;
    uint64_t total_orders_processed = 0;
    uint64_t total_trades = 0;
    
    /**
     * @brief Match a market buy order against the ask side
     */
    void matchMarketBuy(std::shared_ptr<Order> order) {
        while (!asks.empty() && order->quantity > 0) {
            auto& [price, orders] = *asks.begin();
            
            while (!orders.empty() && order->quantity > 0) {
                auto& sell_order = orders.front();
                
                uint64_t trade_qty = std::min(order->quantity, sell_order->quantity);
                
                // Execute trade
                trade_history.emplace_back(order->order_id, sell_order->order_id, price, trade_qty);
                total_trades++;
                
                order->quantity -= trade_qty;
                sell_order->quantity -= trade_qty;
                
                // Remove filled order
                if (sell_order->quantity == 0) {
                    order_map.erase(sell_order->order_id);
                    orders.pop_front();
                }
            }
            
            // Remove empty price level
            if (orders.empty()) {
                asks.erase(asks.begin());
            }
        }
    }
    
    /**
     * @brief Match a market sell order against the bid side
     */
    void matchMarketSell(std::shared_ptr<Order> order) {
        while (!bids.empty() && order->quantity > 0) {
            auto& [price, orders] = *bids.begin();
            
            while (!orders.empty() && order->quantity > 0) {
                auto& buy_order = orders.front();
                
                uint64_t trade_qty = std::min(order->quantity, buy_order->quantity);
                
                // Execute trade
                trade_history.emplace_back(buy_order->order_id, order->order_id, price, trade_qty);
                total_trades++;
                
                order->quantity -= trade_qty;
                buy_order->quantity -= trade_qty;
                
                // Remove filled order
                if (buy_order->quantity == 0) {
                    order_map.erase(buy_order->order_id);
                    orders.pop_front();
                }
            }
            
            // Remove empty price level
            if (orders.empty()) {
                bids.erase(bids.begin());
            }
        }
    }
    
    /**
     * @brief Match a limit buy order
     */
    void matchLimitBuy(std::shared_ptr<Order> order) {
        // Try to match against existing sell orders
        while (!asks.empty() && order->quantity > 0) {
            auto& [best_ask, orders] = *asks.begin();
            
            // Can only match if our bid price >= best ask price
            if (order->price < best_ask) break;
            
            while (!orders.empty() && order->quantity > 0) {
                auto& sell_order = orders.front();
                
                uint64_t trade_qty = std::min(order->quantity, sell_order->quantity);
                
                // Execute trade at the ask price (price-time priority)
                trade_history.emplace_back(order->order_id, sell_order->order_id, best_ask, trade_qty);
                total_trades++;
                
                order->quantity -= trade_qty;
                sell_order->quantity -= trade_qty;
                
                if (sell_order->quantity == 0) {
                    order_map.erase(sell_order->order_id);
                    orders.pop_front();
                }
            }
            
            if (orders.empty()) {
                asks.erase(asks.begin());
            }
        }
        
        // Add remaining quantity to bid book
        if (order->quantity > 0) {
            bids[order->price].push_back(order);
            order_map[order->order_id] = order;
        }
    }
    
    /**
     * @brief Match a limit sell order
     */
    void matchLimitSell(std::shared_ptr<Order> order) {
        // Try to match against existing buy orders
        while (!bids.empty() && order->quantity > 0) {
            auto& [best_bid, orders] = *bids.begin();
            
            // Can only match if our ask price <= best bid price
            if (order->price > best_bid) break;
            
            while (!orders.empty() && order->quantity > 0) {
                auto& buy_order = orders.front();
                
                uint64_t trade_qty = std::min(order->quantity, buy_order->quantity);
                
                // Execute trade at the bid price
                trade_history.emplace_back(buy_order->order_id, order->order_id, best_bid, trade_qty);
                total_trades++;
                
                order->quantity -= trade_qty;
                buy_order->quantity -= trade_qty;
                
                if (buy_order->quantity == 0) {
                    order_map.erase(buy_order->order_id);
                    orders.pop_front();
                }
            }
            
            if (orders.empty()) {
                bids.erase(bids.begin());
            }
        }
        
        // Add remaining quantity to ask book
        if (order->quantity > 0) {
            asks[order->price].push_back(order);
            order_map[order->order_id] = order;
        }
    }
    
public:
    /**
     * @brief Add a new order to the book
     * @return Order ID
     */
    uint64_t addOrder(Side side, OrderType type, double price, uint64_t quantity) {
        auto order = std::make_shared<Order>(next_order_id++, side, type, price, quantity);
        total_orders_processed++;
        
        if (type == OrderType::MARKET) {
            if (side == Side::BUY) {
                matchMarketBuy(order);
            } else {
                matchMarketSell(order);
            }
        } else {  // LIMIT
            if (side == Side::BUY) {
                matchLimitBuy(order);
            } else {
                matchLimitSell(order);
            }
        }
        
        return order->order_id;
    }
    
    /**
     * @brief Cancel an existing order
     */
    bool cancelOrder(uint64_t order_id) {
        auto it = order_map.find(order_id);
        if (it == order_map.end()) return false;
        
        auto order = it->second;
        
        // Remove from price level
        if (order->side == Side::BUY) {
            auto& orders = bids[order->price];
            orders.erase(std::remove(orders.begin(), orders.end(), order), orders.end());
            if (orders.empty()) bids.erase(order->price);
        } else {
            auto& orders = asks[order->price];
            orders.erase(std::remove(orders.begin(), orders.end(), order), orders.end());
            if (orders.empty()) asks.erase(order->price);
        }
        
        order_map.erase(it);
        return true;
    }
    
    /**
     * @brief Get best bid price
     */
    double getBestBid() const {
        return bids.empty() ? 0.0 : bids.begin()->first;
    }
    
    /**
     * @brief Get best ask price
     */
    double getBestAsk() const {
        return asks.empty() ? 0.0 : asks.begin()->first;
    }
    
    /**
     * @brief Get mid price
     */
    double getMidPrice() const {
        if (bids.empty() || asks.empty()) return 0.0;
        return (getBestBid() + getBestAsk()) / 2.0;
    }
    
    /**
     * @brief Get spread
     */
    double getSpread() const {
        if (bids.empty() || asks.empty()) return 0.0;
        return getBestAsk() - getBestBid();
    }
    
    /**
     * @brief Print order book state
     */
    void printOrderBook(int depth = 5) const {
        std::cout << "\n=== Order Book ===" << std::endl;
        std::cout << std::fixed << std::setprecision(2);
        
        // Ask side (sell orders)
        std::cout << "\n--- ASKS (Sell) ---" << std::endl;
        std::cout << std::setw(12) << "Price" << std::setw(15) << "Quantity" << std::setw(15) << "Orders" << std::endl;
        std::cout << std::string(42, '-') << std::endl;
        
        int count = 0;
        for (auto it = asks.rbegin(); it != asks.rend() && count < depth; ++it, ++count) {
            uint64_t total_qty = 0;
            for (const auto& order : it->second) {
                total_qty += order->quantity;
            }
            std::cout << std::setw(12) << it->first 
                      << std::setw(15) << total_qty 
                      << std::setw(15) << it->second.size() << std::endl;
        }
        
        // Spread
        std::cout << "\n" << std::string(42, '=') << std::endl;
        std::cout << "Spread: $" << getSpread() << " | Mid: $" << getMidPrice() << std::endl;
        std::cout << std::string(42, '=') << "\n" << std::endl;
        
        // Bid side (buy orders)
        std::cout << "--- BIDS (Buy) ---" << std::endl;
        std::cout << std::setw(12) << "Price" << std::setw(15) << "Quantity" << std::setw(15) << "Orders" << std::endl;
        std::cout << std::string(42, '-') << std::endl;
        
        count = 0;
        for (const auto& [price, orders] : bids) {
            if (count >= depth) break;
            uint64_t total_qty = 0;
            for (const auto& order : orders) {
                total_qty += order->quantity;
            }
            std::cout << std::setw(12) << price 
                      << std::setw(15) << total_qty 
                      << std::setw(15) << orders.size() << std::endl;
            count++;
        }
        std::cout << std::endl;
    }
    
    /**
     * @brief Print recent trades
     */
    void printRecentTrades(int n = 10) const {
        std::cout << "=== Recent Trades ===" << std::endl;
        std::cout << std::setw(12) << "Buy ID" 
                  << std::setw(12) << "Sell ID" 
                  << std::setw(12) << "Price" 
                  << std::setw(12) << "Quantity" << std::endl;
        std::cout << std::string(48, '-') << std::endl;
        
        int start = std::max(0, (int)trade_history.size() - n);
        for (int i = start; i < trade_history.size(); ++i) {
            const auto& trade = trade_history[i];
            std::cout << std::setw(12) << trade.buy_order_id
                      << std::setw(12) << trade.sell_order_id
                      << std::setw(12) << std::fixed << std::setprecision(2) << trade.price
                      << std::setw(12) << trade.quantity << std::endl;
        }
        std::cout << std::endl;
    }
    
    /**
     * @brief Print statistics
     */
    void printStats() const {
        std::cout << "=== Order Book Statistics ===" << std::endl;
        std::cout << "Total orders processed: " << total_orders_processed << std::endl;
        std::cout << "Total trades executed: " << total_trades << std::endl;
        std::cout << "Active bid orders: " << order_map.size() << std::endl;
        std::cout << "Best bid: $" << std::fixed << std::setprecision(2) << getBestBid() << std::endl;
        std::cout << "Best ask: $" << getBestAsk() << std::endl;
        std::cout << "Spread: $" << getSpread() << std::endl;
        std::cout << std::endl;
    }
};

int main() {
    std::cout << "=== Limit Order Book Simulator ===" << std::endl << std::endl;
    
    OrderBook book;
    
    // Scenario 1: Build initial order book
    std::cout << "Building initial order book..." << std::endl;
    
    // Add sell orders
    book.addOrder(Side::SELL, OrderType::LIMIT, 100.50, 100);
    book.addOrder(Side::SELL, OrderType::LIMIT, 100.60, 150);
    book.addOrder(Side::SELL, OrderType::LIMIT, 100.70, 200);
    book.addOrder(Side::SELL, OrderType::LIMIT, 100.80, 175);
    book.addOrder(Side::SELL, OrderType::LIMIT, 100.90, 125);
    
    // Add buy orders
    book.addOrder(Side::BUY, OrderType::LIMIT, 100.40, 120);
    book.addOrder(Side::BUY, OrderType::LIMIT, 100.30, 180);
    book.addOrder(Side::BUY, OrderType::LIMIT, 100.20, 150);
    book.addOrder(Side::BUY, OrderType::LIMIT, 100.10, 200);
    book.addOrder(Side::BUY, OrderType::LIMIT, 100.00, 100);
    
    book.printOrderBook();
    
    // Scenario 2: Market buy order (takes liquidity)
    std::cout << "\n>>> Executing MARKET BUY order for 250 shares <<<" << std::endl;
    book.addOrder(Side::BUY, OrderType::MARKET, 0, 250);
    book.printOrderBook();
    book.printRecentTrades(3);
    
    // Scenario 3: Aggressive limit buy (crosses spread)
    std::cout << "\n>>> Adding LIMIT BUY at $100.65 for 180 shares (crosses spread) <<<" << std::endl;
    book.addOrder(Side::BUY, OrderType::LIMIT, 100.65, 180);
    book.printOrderBook();
    book.printRecentTrades(3);
    
    // Scenario 4: Passive limit orders
    std::cout << "\n>>> Adding passive LIMIT orders <<<" << std::endl;
    book.addOrder(Side::BUY, OrderType::LIMIT, 100.35, 100);
    book.addOrder(Side::SELL, OrderType::LIMIT, 100.95, 150);
    book.printOrderBook();
    
    // Performance test
    std::cout << "\n=== Performance Test ===" << std::endl;
    OrderBook perf_book;
    
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> price_dist(99.0, 101.0);
    std::uniform_int_distribution<uint64_t> qty_dist(10, 500);
    std::uniform_int_distribution<int> side_dist(0, 1);
    
    int n_orders = 100000;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < n_orders; ++i) {
        Side side = (side_dist(rng) == 0) ? Side::BUY : Side::SELL;
        double price = std::round(price_dist(rng) * 100) / 100.0;  // Round to cents
        uint64_t qty = qty_dist(rng);
        
        perf_book.addOrder(side, OrderType::LIMIT, price, qty);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Processed " << n_orders << " orders in " << duration.count() << " ms" << std::endl;
    std::cout << "Throughput: " << std::fixed << std::setprecision(0) 
              << (n_orders * 1000.0 / duration.count()) << " orders/sec" << std::endl;
    
    perf_book.printStats();
    
    return 0;
}
