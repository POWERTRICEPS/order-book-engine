#include "../include/OrderBook.h"
#include <algorithm>
#include <iostream>

OrderBook::OrderBook() = default;

void OrderBook::addOrder(const Order& order) {
    if (order.type == OrderType::LIMIT) {
        processLimitOrder(order);
    } else {
        processMarketOrder(order);
    }

    // default execution to match opposing orders
    matchOrders();
}

void OrderBook::modifyOrder(OrderId id, Quantity newQuantity) {
    // zero quantity means to cancel the order entirely
    if (newQuantity == 0) {
        cancelOrder(id);
        return;
    }
    
    auto it = m_orders.find(id);
    if (it == m_orders.end()) return;

    Order old = it->second;
    cancelOrder(id);

    // Recreates the old order with the new quantity
    Order updated{
        old.id, old.side, old.type, newQuantity,
        old.price, old.timestamp + 1, old.symbol
    };
    addOrder(updated);
}

void OrderBook::cancelOrder(OrderId id) {
    auto it = m_orders.find(id);
    if (it == m_orders.end()) return;

    const Order& ord = it->second;

    auto pit = m_orderPrices.find(id);
    if (pit == m_orderPrices.end()) {
        m_orders.erase(it);
        return;
    }

    Price px = pit->second;

    if (ord.side == Side::BUY) {
        // Remove from bids at this price
        auto lvlIt = m_bids.find(px);
        if (lvlIt != m_bids.end()) {
            auto& dq = lvlIt->second.orders;
            for (auto qit = dq.begin(); qit != dq.end(); ++qit) {
                if (qit->id == id) {
                    // Updates the quantity at this price level
                    lvlIt->second.totalQty =
                        (lvlIt->second.totalQty >= qit->quantity)
                        ? (lvlIt->second.totalQty - qit->quantity) : 0;
                    dq.erase(qit);
                    break;
                }
            }
            if (dq.empty()) m_bids.erase(lvlIt);
        }
    } else {
        auto lvlIt = m_asks.find(px);
        // Remove from asks at this price
        if (lvlIt != m_asks.end()) {
            auto& dq = lvlIt->second.orders;
            for (auto qit = dq.begin(); qit != dq.end(); ++qit) {
                if (qit->id == id) {
                    // Updates the quantity at this price level
                    lvlIt->second.totalQty =
                        (lvlIt->second.totalQty >= qit->quantity)
                        ? (lvlIt->second.totalQty - qit->quantity) : 0;
                    dq.erase(qit);
                    break;
                }
            }
            if (dq.empty()) m_asks.erase(lvlIt);
        }
    }

    m_orders.erase(id);
    m_orderPrices.erase(id);
}

void OrderBook::matchOrders() {
    while (!m_bids.empty() && !m_asks.empty()) {
        auto bIt = m_bids.begin();
        auto aIt = m_asks.begin();

        // If best bid < best ask, no match possible
        if (bIt->first < aIt->first) break;

        auto& bidDepth = bIt->second;
        auto& askDepth = aIt->second;

        // Match oldest orders at best bid and best ask
        Order& buy  = bidDepth.orders.front();
        Order& sell = askDepth.orders.front();

        Quantity traded = std::min(buy.quantity, sell.quantity);
        Price tradePx   = sell.price;

        std::cout << "Trade: " << traded << " @ " << tradePx << "\n";

        buy.quantity  -= traded;
        sell.quantity -= traded;

        bidDepth.totalQty = (bidDepth.totalQty >= traded) ? bidDepth.totalQty - traded : 0;
        askDepth.totalQty = (askDepth.totalQty >= traded) ? askDepth.totalQty - traded : 0;

        // Remove or update buy order
        if (buy.quantity == 0)  {
            bidDepth.orders.pop_front();
            m_orders.erase(buy.id);
            m_orderPrices.erase(buy.id);
        } else {
            auto itB = m_orders.find(buy.id);
            if (itB != m_orders.end()) itB->second.quantity = buy.quantity;
        }

        if (sell.quantity == 0) {
            askDepth.orders.pop_front();
            m_orders.erase(sell.id);
            m_orderPrices.erase(sell.id);
        } else {
            auto itS = m_orders.find(sell.id);
            if (itS != m_orders.end()) itS->second.quantity = sell.quantity;
        }

        if (bidDepth.orders.empty()) m_bids.erase(bIt);
        if (askDepth.orders.empty()) m_asks.erase(aIt);
    }
}

Price OrderBook::getBestBid() const {
    return m_bids.empty() ? Price{0} : m_bids.begin()->first;
}

Price OrderBook::getBestAsk() const {
    return m_asks.empty() ? Price{0} : m_asks.begin()->first;
}

Depth OrderBook::getDepth(Price price) const {
    if (auto it = m_bids.find(price); it != m_bids.end()) return it->second;
    if (auto it = m_asks.find(price); it != m_asks.end()) return it->second;
    return Depth{};
}

void OrderBook::processLimitOrder(const Order& order) {
    if (order.side == Side::BUY) {
        auto& depth = m_bids[order.price];
        depth.orders.push_back(order);
        depth.totalQty += order.quantity;
    } else {
        auto& depth = m_asks[order.price];
        depth.orders.push_back(order);
        depth.totalQty += order.quantity;
    }

    // Track order by ID for future modification/cancellation
    m_orders.emplace(order.id, order);
    m_orderPrices.emplace(order.id, order.price);
}


void OrderBook::processMarketOrder(const Order& order) {
    if (order.side == Side::BUY) {
        Quantity qtyLeft = order.quantity;
        while (qtyLeft > 0 && !m_asks.empty()) {
            // We want the lowest seller price
            auto aIt = m_asks.begin();
            auto& depth = aIt->second;
            // Go through each seller at this price level
            while (!depth.orders.empty() && qtyLeft > 0) {
                Order& resting = depth.orders.front();
                Quantity tradeQty = std::min(resting.quantity, qtyLeft);
                // Reduce both sides by traded amount
                resting.quantity -= tradeQty;
                qtyLeft -= tradeQty;
                depth.totalQty = (depth.totalQty >= tradeQty) ? depth.totalQty - tradeQty : 0;

                if (resting.quantity == 0) {
                    m_orders.erase(resting.id);
                    m_orderPrices.erase(resting.id);
                    depth.orders.pop_front();
                } else {
                    auto itR = m_orders.find(resting.id);
                    if (itR != m_orders.end()) itR->second.quantity = resting.quantity;
                }
            }

            if (depth.orders.empty()) m_asks.erase(aIt);
        }
    } else {
        Quantity qtyLeft = order.quantity;
        while (qtyLeft > 0 && !m_bids.empty()) {
            auto bIt = m_bids.begin();
            auto& depth = bIt->second;

            while (!depth.orders.empty() && qtyLeft > 0) {
                Order& resting = depth.orders.front();
                Quantity tradeQty = std::min(resting.quantity, qtyLeft);

                resting.quantity -= tradeQty;
                qtyLeft -= tradeQty;
                depth.totalQty = (depth.totalQty >= tradeQty) ? depth.totalQty - tradeQty : 0;

                if (resting.quantity == 0) {
                    m_orders.erase(resting.id);
                    m_orderPrices.erase(resting.id);
                    depth.orders.pop_front();
                } else {
                    auto itR = m_orders.find(resting.id);
                    if (itR != m_orders.end()) itR->second.quantity = resting.quantity;
                }
            }

            if (depth.orders.empty()) m_bids.erase(bIt);
        }
    }
}



void OrderBook::printBook(int depth) const {
    std::cout << "----- ORDER BOOK -----\n";

    std::cout << "Asks:\n";
    int count = 0;
    for (auto it = m_asks.begin(); it != m_asks.end() && count < depth; ++it, ++count) {
        std::cout << "  " << it->first << " x " << it->second.totalQty << "\n";
    }

    std::cout << "Bids:\n";
    count = 0;
    for (auto it = m_bids.begin(); it != m_bids.end() && count < depth; ++it, ++count) {
        std::cout << "  " << it->first << " x " << it->second.totalQty << "\n";
    }

    std::cout << "----------------------\n";
}
