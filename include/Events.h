#pragma once
#include <cstdint>
#include <variant>
#include <string>


struct NewOrder {
    uint64_t id;
    Side     side;
    double   price;
    uint64_t qty;
};

struct CancelOrder {
    uint64_t id;
};

struct Shutdown {};   

using Event = std::variant<NewOrder, CancelOrder, Shutdown>;