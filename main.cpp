#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <ranges>
#include "TradeRequest.h"
#include "OrderBook.h"
#include "functions.h"


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
