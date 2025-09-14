#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <vector>
#include <sstream>
#include <iterator>

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
    void addOrder(const Order &order);

    void removeOrder(OrderId orderId);

    std::map<Price, std::list<Order> > getBids() { return bids; }
    std::map<Price, std::list<Order> > getAsks() { return asks; }

private:
    std::map<Price, std::list<Order> > bids;
    std::map<Price, std::list<Order> > asks;

    // Lookup table to find orders by their ID, for efficient removal O(1) compared to O(n)
    std::unordered_map<OrderId, std::list<Order>::iterator> orderIdLookup;
};

void OrderBook::addOrder(const Order &order) {
    const int priceKey = static_cast<int>(order.price);
    if (order.side == Side::Buy) {
        bids[priceKey].push_back(order);
        const auto it = std::prev(bids[priceKey].end());
        orderIdLookup[order.orderId] = it;
    } else if (order.side == Side::Sell) {
        asks[priceKey].push_back(order);
        const auto it = std::prev(asks[priceKey].end());
        orderIdLookup[order.orderId] = it;;
    } else {
        std::cerr << "Invalid side, needs to be either Buy or Sell" << std::endl;
    }
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

std::vector<std::string> parseTokens(const std::string &line, char delimiter);

int main() {
    OrderBook orderBook;
    std::ifstream inputFile("../data.txt");
    std::vector<Order> orders = getOrders(inputFile);

    for (Order o: orders) {
        std::cout << o.orderId << " " << o.quantity << " " << o.price << " " << (o.side == Side::Buy ? "Buy" : "Sell")
                << std::endl;
        orderBook.addOrder(o);
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
