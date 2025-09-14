#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <vector>
#include <sstream>
#include <iterator>
#include <ranges>

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

int main() {
    OrderBook orderBook;
    std::ifstream inputFile("../data.txt");
    std::vector<Order> orders = getOrders(inputFile);

    for (auto &order: orders) {
        orderBook.addOrder(order);
    }

    // In main()
    for (auto &order: orders) {
        for (std::vector<TradeRequest> trades = orderBook.addOrder(order); const auto &trade: trades) {
            std::cout << "TRADE: " << trade.quantity << " @ " << formatPrice(trade.price) << std::endl;
        }
    }
}

std::vector<Order> getOrders(std::ifstream &inFile) {
    if (!inFile.is_open()) {
        throw std::domain_error("Cannot parse data");
    }
    std::string line;
    std::vector<Order> orders;

    // We receive data in format: orderId,quantity,price,side (e.g., 1,100,10.5,Buy)
    while (std::getline(inFile, line)) {
        std::vector<std::string> tokens = parseTokens(line, ',');
        if (tokens.size() != 4) {
            std::cerr << "Skipping invalid line: " << line << std::endl;
            continue;
        }
        orders.emplace_back(Order{
                std::stoll(tokens[0]),
                static_cast<Quantity>(stoul(tokens[1])),
                static_cast<Price>(std::stod(tokens[2]) * 100), // Convert to integer representation for full accuracy
                tokens[3] == "Buy" ? Side::Buy : Side::Sell,
            }
        );
    }

    return orders;
}

// Utility function to split a string by a given delimiter
std::vector<std::string> parseTokens(const std::string &line, const char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream stream(line);

    while (getline(stream, token, delimiter)) tokens.push_back(token);

    return tokens;
}

float formatPrice(const Price price) {
    const float formattedPrice = static_cast<float>(price) / 100;
    return formattedPrice;
}
