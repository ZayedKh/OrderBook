#ifndef ORDER_BOOK_H
#define ORDER_BOOK_H
#include "TradeRequest.h"
#include <vector>
#include <unordered_map>
#include <map>
#include <list>


class OrderBook {
public:
    std::vector<TradeRequest> addOrder(Order &order);

    void removeOrder(OrderId orderId);

    std::map<Price, std::list<Order>, std::greater<Price> > getBids() { return bids; }
    std::map<Price, std::list<Order> > getAsks() { return asks; }

private:
    std::map<Price, std::list<Order>, std::greater<Price> > bids;
    std::map<Price, std::list<Order> > asks;

    // Lookup table to find orders by their ID, for efficient removal O(1) compared to O(n)
    std::unordered_map<OrderId, std::list<Order>::iterator> orderIdLookup;
};

#endif
