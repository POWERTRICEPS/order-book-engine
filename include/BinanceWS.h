#pragma once
#include <functional>
#include <string>

class BinanceWS {
public:
    using BestBidAskHandler = std::function<void(const std::string& symbol,
                                                 const std::string& bestBid,
                                                 const std::string& bestAsk,
                                                 const std::string& bidQty,
                                                 const std::string& askQty)>;

    explicit BinanceWS(std::string symbolLower);
    void setHandler(BestBidAskHandler h);
    void run(); 

private:
    std::string m_uri;
    BestBidAskHandler m_handler;

};
