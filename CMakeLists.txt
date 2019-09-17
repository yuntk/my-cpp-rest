cmake_minimum_required(VERSION 3.14)
project(codetest)

set(CMAKE_CXX_STANDARD 14)

LINK_LIBRARIES(boost_system crypto ssl cpprest)
add_executable(codetest main.cpp myrest.h)
