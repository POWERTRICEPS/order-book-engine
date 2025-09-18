#include "../include/OrderBook.h"
#include <algorithm> 

OrderBook::OrderBook() = default;

void OrderBook::addOrder(const Order& order){
    if(order.type == OrderType::LIMIT){
        processLimitOrder(order);
    }else{
        processMarketOrder(order);
    }
    matchOrders();
}

void OrderBook::modifyOrder(OrderId id, Quantity newQuantity){
    // if newQuantity is 0, treat as cancellation
    if(newQuantity == 0){
        cancelOrder(id);
        return;
    }

    // find existing order within book
    auto it = m_orders.find(id);
    if(it == m_orders.end())  return;

    Order old = it->second;
    cancelOrder(id);

    Order updated{
        old.id, old.side, old.type, newQuantity, old.price, /*timestamp*/ old.timestamp + 1, old.symbol
    };
    addOrder(updated);
}

void OrderBook::cancelOrder(OrderId id){
     auto it = m_orders.find(id);
    if (it == m_orders.end()) return;  // not found

    const Order& ord = it->second;
    // Find the price level this order belongs to
    auto pit = m_orderPrices.find(id);
    if (pit == m_orderPrices.end()) {
        m_orders.erase(it);
        return;
    }

    Price px = pit->second;

    // ---------------- BUY SIDE ----------------
    if (ord.side == Side::BUY) {
        auto lvlIt = m_bids.find(px);
        if (lvlIt != m_bids.end()) {
            auto& dq = lvlIt->second.orders;
            // Search the deque at this price level
            for (auto qit = dq.begin(); qit != dq.end(); ++qit) {
                if (qit->id == id) {
                    // Adjust aggregate depth
                    if (lvlIt->second.totalQty >= qit->quantity)
                        lvlIt->second.totalQty -= qit->quantity;
                    else
                        lvlIt->second.totalQty = 0;

                    dq.erase(qit);
                    break;
                }
            }
            // If no orders left at this price, remove the price level
            if (dq.empty()) m_bids.erase(lvlIt);
        }
    }
    // ---------------- SELL SIDE ----------------
    else {
        auto lvlIt = m_asks.find(px);
        if (lvlIt != m_asks.end()) {
            auto& dq = lvlIt->second.orders;
            for (auto qit = dq.begin(); qit != dq.end(); ++qit) {
                if (qit->id == id) {
                    if (lvlIt->second.totalQty >= qit->quantity)
                        lvlIt->second.totalQty -= qit->quantity;
                    else
                        lvlIt->second.totalQty = 0;

                    dq.erase(qit);
                    break;
                }
            }
            if (dq.empty()) m_asks.erase(lvlIt);
        }
    }

    // Remove from lookup maps
    m_orders.erase(id);
    m_orderPrices.erase(id);
}

void OrderBook::matchOrders(){
    
}