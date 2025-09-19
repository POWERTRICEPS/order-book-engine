#pragma once

#include <cstdint>
#include <string>

// typedefs
using OrderId = uint64_t;
using Quantity = uint64_t;
using Price = uint64_t;
using Timestamp = uint64_t;

enum class Side { BUY, SELL };
enum class OrderType { LIMIT, MARKET };

struct Order {
    OrderId id;           // unique identifiers
    Side side;            // BUY or SELl
    OrderType type;       // LIMIT or MARKET
    Quantity quantity;    // Number of units 
    Price price;          // limit price 
    Timestamp timestamp;  // for time priority
    std::string symbol;   // TIKER
    
    // Constructor
    Order(OrderId id, Side side, OrderType type, Quantity quantity, Price price, Timestamp timestamp, const std::string& symbol)
        : id(id), side(side), type(type), quantity(quantity),
          price(price), timestamp(timestamp), symbol(symbol) {}
};