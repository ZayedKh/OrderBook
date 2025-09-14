#include "functions.h"

#include <fstream>
#include <iostream>
#include <sstream>


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
