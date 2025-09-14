#ifndef TRADE_REQUEST_H
#define TRADE_REQUEST_H
#include <cstdint>

using Price = std::uint64_t;
using Quantity = std::uint32_t;
using OrderId = long long;

enum class Side { Buy, Sell };

struct Order {
    OrderId orderId;
    Quantity quantity;
    Price price;
    Side side;
};

struct TradeRequest {
    OrderId aggressorOrderId;
    OrderId restingOrderId;
    Price price;
    Quantity quantity;
};

#endif
