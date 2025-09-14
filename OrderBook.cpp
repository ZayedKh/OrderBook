#include "OrderBook.h"
#include <iostream>
#include <fstream>


// When we add an order we should attempt to match it against any existing orders on the opposite side of the book
std::vector<TradeRequest> OrderBook::addOrder(Order &order) {
    const int priceKey = static_cast<int>(order.price);
    std::vector<TradeRequest> trades;

    if (order.side == Side::Buy) {
        // If Buy price is greater than Sell price, we have a match and can fill the order until either:
        // 1. The incoming order is filled
        // 2. The quantity for the Sell order is exhausted, at which point we should remove that order and move to the next lowest price
        // When we move to the next lowest price we should still check if the prices match
        while (order.quantity > 0 && !asks.empty() && order.price >= asks.begin()->first) {
            // We have a match, can start to fill out the order
            auto &[price, orderList] = *asks.begin();
            auto &restingOrder = orderList.front();
            const Quantity tradeQuantity = std::min(order.quantity, restingOrder.quantity);

            trades.emplace_back(TradeRequest{
                order.orderId,
                restingOrder.orderId,
                price,
                tradeQuantity,
            });

            order.quantity -= tradeQuantity;
            restingOrder.quantity -= tradeQuantity;

            if (restingOrder.quantity <= 0) {
                orderIdLookup.erase(restingOrder.orderId); // Remove from fast lookup table of iterators
                orderList.pop_front(); // Pop from front of list to remove resting order
                if (orderList.empty()) {
                    asks.erase(asks.begin()); // Remove price level if no more orders at that price
                }
            }
        }

        // If we have remaining quantity on the order after attempting to match, we should add it to the book
        if (order.quantity > 0) {
            bids[priceKey].push_back(order);
            const auto it = std::prev(bids[priceKey].end());
            orderIdLookup[order.orderId] = it;
        }
    } else if (order.side == Side::Sell) {
        // If Sell price is lower than the highest Buy price, we have a match and can fill the order until either:
        // 1. The incoming order is filled
        // 2. The quantity for the Buy order price is exhausted, at which point we should remove that order and move to the next highest price
        // When we move to the next highest price we should still check if the prices allow for a tra
        while (order.quantity > 0 && !bids.empty() && order.price <= bids.begin()->first) {
            auto &[price, orderList] = *bids.begin();
            auto &restingOrder = orderList.front();
            const Quantity tradeQuantity = std::min(order.quantity, restingOrder.quantity);

            trades.emplace_back(TradeRequest{
                order.orderId,
                restingOrder.orderId,
                price,
                tradeQuantity,
            });

            order.quantity -= tradeQuantity;
            restingOrder.quantity -= tradeQuantity;

            if (restingOrder.quantity <= 0) {
                orderIdLookup.erase(restingOrder.orderId);
                orderList.pop_front();
                if (orderList.empty()) {
                    bids.erase(bids.begin());
                }
            }
        }

        // If we have remaining quantity on the order after attempting to match, we should add it to the book
        if (order.quantity > 0) {
            asks[priceKey].push_back(order);
            const auto it = std::prev(asks[priceKey].end());
            orderIdLookup[order.orderId] = it;;
        }
    } else {
        std::cerr << "Invalid side, needs to be either Buy or Sell" << std::endl;
    }

    return trades;
}

void OrderBook::removeOrder(OrderId orderId) {
    const auto mapEntry = orderIdLookup.find(orderId);

    if (mapEntry == orderIdLookup.end()) {
        std::cerr << "Non-existent order id " << orderId << std::endl;
    }

    const auto &orderIt = mapEntry->second;
    const Order &order = *orderIt;
    const Price priceKey = order.price;

    if (order.side == Side::Buy) {
        bids.at(priceKey).erase(orderIt);

        if (bids.at(priceKey).empty()) {
            bids.erase(priceKey);
        }
    } else if (order.side == Side::Sell) {
        asks.at(priceKey).erase(orderIt);

        if (asks.at(priceKey).empty()) {
            asks.erase(priceKey);
        }
    }

    orderIdLookup.erase(orderId);
}

std::vector<Order> getOrders(std::ifstream &inFile);

float formatPrice(Price price);

std::vector<std::string> parseTokens(const std::string &line, char delimiter);

