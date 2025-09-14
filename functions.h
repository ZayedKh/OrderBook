#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <vector>
#include "TradeRequest.h"


std::vector<Order> getOrders(std::ifstream &inFile);

float formatPrice(Price price);

std::vector<std::string> parseTokens(const std::string &line, char delimiter);

#endif
