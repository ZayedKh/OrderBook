cmake_minimum_required(VERSION 3.31)
project(OrderBook)

set(CMAKE_CXX_STANDARD 20)

add_executable(OrderBook main.cpp OrderBook.h OrderBook.cpp TradeRequest.h functions.cpp functions.h)
