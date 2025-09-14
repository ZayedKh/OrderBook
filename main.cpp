#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <vector>
#include <sstream>

using Price = double;
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
    void addOrder(const Order &order) {
        int priceKey = static_cast<int>(order.price * 100); // Convert price to integer key (e.g., in cents)
        if (order.side == Side::Buy) {
            bids[priceKey].push_back(order);
        } else if (order.side == Side::Sell) {
            asks[priceKey].push_back(order);
        }
    };

    void removeOrder(OrderId orderId);

private:
    std::map<int, std::list<Order> > bids;
    std::map<int, std::list<Order> > asks;
};

OrderBook orderBook;

std::vector<Order> getOrders(std::ifstream &inFile);

std::vector<std::string> parseTokens(const std::string &line, char delimiter);

int main() {
    std::ifstream inputFile("../data.txt");

    std::vector<Order> orders = getOrders(inputFile);

    for (Order o: orders)
        std::cout << o.orderId << " " << o.quantity << " " << o.price << " " << (o.side == Side::Buy ? "Buy" : "Sell")
                <<
                std::endl;
}

std::vector<Order> getOrders(std::ifstream &inFile) {
    if (!inFile.is_open()) {
        throw std::domain_error("Cannot parse data");
    }
    std::string line;
    std::vector<Order> orders;


    // We receive data in format: orderId,side,quantity,price (e.g., 1,B,100,10.5)
    while (std::getline(inFile, line)) {
        std::vector<std::string> tokens = parseTokens(line, ',');
        orders.emplace_back(Order{
                std::stoll(tokens[0]),
                static_cast<Quantity>(stoul(tokens[1])),
                std::stod(tokens[2]),
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
