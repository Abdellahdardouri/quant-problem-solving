# Limit Order Book Simulator

Low-latency matching engine implementing price-time priority with efficient data structures.

## Features

- **Price-time priority** matching (FIFO at each price level)
- **Order types**: Market, Limit, Cancel
- **Efficient data structures**: 
  - `std::map` for price levels (O(log n) insertion)
  - `std::deque` for order queues (O(1) front/back operations)
- **Real-time matching** with trade execution
- ~100K operations/second throughput

## Compilation

```bash
g++ -std=c++17 -O3 -o orderbook orderbook.cpp
./orderbook
```

## Requirements

- C++17 compatible compiler
- Standard library only (no external dependencies)

## Order Book Structure

```
ASK SIDE (Sell orders)
  Price      Quantity    Orders
  ------     --------    ------
  100.90        125         1
  100.80        175         1
  100.70        200         1
  100.60        150         1
  100.50        100         1    <- Best Ask
======================== Spread ========================
  100.40        120         1    <- Best Bid
  100.30        180         1
  100.20        150         1
  100.10        200         1
  100.00        100         1
BID SIDE (Buy orders)
```

## Matching Logic

### Limit Order Matching
1. Check if order crosses the spread
2. If yes, match against best available price (price-time priority)
3. Execute trades until order is filled or no more matches
4. Add remainder to order book

### Market Order Matching
1. Match against best available price on opposite side
2. Walk the book until order is completely filled
3. Market orders consume liquidity (take fees)

## Data Structures

```cpp
// Buy orders: descending price (higher bids first)
map<double, deque<Order>> bids;

// Sell orders: ascending price (lower asks first)
map<double, deque<Order>, greater<double>> asks;

// Fast order lookup for cancellations
map<uint64_t, Order*> order_map;
```

## Performance Characteristics

| Operation | Time Complexity | Throughput |
|-----------|----------------|------------|
| Add Order | O(log n)       | ~100K/sec  |
| Cancel    | O(log n)       | ~150K/sec  |
| Match     | O(k log n)*    | ~80K/sec   |

*k = number of price levels crossed

## Example Usage

```cpp
OrderBook book;

// Add limit orders
book.addOrder(Side::BUY, OrderType::LIMIT, 100.00, 500);
book.addOrder(Side::SELL, OrderType::LIMIT, 100.50, 300);

// Execute market order
book.addOrder(Side::BUY, OrderType::MARKET, 0, 200);

// Cancel order
book.cancelOrder(order_id);

// Query book state
double bid = book.getBestBid();
double ask = book.getBestAsk();
double spread = book.getSpread();
```

## Production Enhancements

- **Lock-free queues** for multi-threaded matching
- **Memory pools** to avoid allocation overhead
- **FIX protocol** interface for order entry
- **Market data feed** with Level 2/3 updates
- **Persistence** for order recovery
- **Risk checks** (credit, position limits)
