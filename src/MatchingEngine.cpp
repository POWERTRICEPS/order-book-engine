#include "../include/MatchingEngine.h"
#include "../include/Order.h"            
#include <variant>
#include <chrono>
#include <type_traits>

static constexpr uint64_t PRICE_SCALE = 10000;   // 1 = 0.0001

static Timestamp now_ts() {
    using std::chrono::duration_cast;
    using std::chrono::nanoseconds;
    using std::chrono::steady_clock;
    return static_cast<Timestamp>(
        duration_cast<nanoseconds>(steady_clock::now().time_since_epoch()).count()
    );
}

static Price to_price_uint(double p) {
    return static_cast<Price>(p * PRICE_SCALE + 0.5);
}
static double to_price_double(Price p) {
    return static_cast<double>(p) / PRICE_SCALE;
}

static void apply_new_order(OrderBook& ob, const NewOrder& e) {
    Order o(
        static_cast<OrderId>(e.id),
        e.side,                       
        OrderType::LIMIT,
        static_cast<Quantity>(e.qty),
        to_price_uint(e.price),
        now_ts(),
        "DEMO"
    );
    ob.addOrder(o);
    ob.matchOrders();
}

static void apply_cancel(OrderBook& ob, const CancelOrder& e) {
    ob.cancelOrder(static_cast<OrderId>(e.id));
}

static double best_bid_price(const OrderBook& ob) { return to_price_double(ob.getBestBid()); }
static double best_ask_price(const OrderBook& ob) { return to_price_double(ob.getBestAsk()); }


MatchingEngine::MatchingEngine(ThreadQ<Event>& queue) : 
    q_(queue) {}

MatchingEngine::~MatchingEngine() { stop(); }

void MatchingEngine::start() {
    if (running_.exchange(true)) return;
    th_ = std::thread(&MatchingEngine::run, this);
}

void MatchingEngine::stop() {
    if (!running_.exchange(false)) return;
    q_.push(Shutdown{});
    if (th_.joinable()) th_.join();
}

void MatchingEngine::join() {
    if (th_.joinable()) th_.join();
}

TopSnapshot MatchingEngine::topOfBook() const {
    std::lock_guard<std::mutex> lk(snapMu_);
    return snap_;
}


void MatchingEngine::run() {
    Event ev;
    while (q_.pop(ev)) {
        bool done = false;

        std::visit([&](auto&& msg){
            using T = std::decay_t<decltype(msg)>;
            if constexpr (std::is_same_v<T, NewOrder>)         apply_new_order(book_, msg);
            else if constexpr (std::is_same_v<T, CancelOrder>) apply_cancel(book_, msg);
            else done = true; 
        }, ev);

        processed_.fetch_add(1, std::memory_order_relaxed);

        TopSnapshot s;
        s.bestBid = best_bid_price(book_);
        s.bestAsk = best_ask_price(book_);
        { std::lock_guard<std::mutex> lk(snapMu_); snap_ = s; }

        if (done) break;
    }
}