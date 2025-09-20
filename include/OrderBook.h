#pragma once

#include <unordered_map>
#include <map>
#include <deque>
#include <string>
#include "Order.h"

struct Depth {
    std::deque<Order> orders;   // FIFO queue of orders at this price
    Quantity totalQty = 0;      // sum of all orders at this price
};

class OrderBook {
public:

    OrderBook();
    
    void addOrder(const Order& order);
    void modifyOrder(OrderId id, Quantity newQuantity);
    void cancelOrder(OrderId id);
    
    // Matching engine functions
    void matchOrders();
    
    // Getters for market data
    Price getBestBid() const;
    Price getBestAsk() const;
    Depth getDepth(Price price) const;

    void printBook(int depth = 5) const;
    
private:
    
    std::map<Price, Depth, std::greater<Price>> m_bids;

    std::map<Price, Depth> m_asks; // ascending
    
    std::unordered_map<OrderId, Order> m_orders;
    std::unordered_map<OrderId, Price> m_orderPrices;
    
    // Handle new limit orders:
    void processLimitOrder(const Order& order);

    // Handle new market orders:
    void processMarketOrder(const Order& order);
};
