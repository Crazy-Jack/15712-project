cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Byzantine-Simulator DESCRIPTION "15-712 course project")

add_executable(bft-sim src/simulator.cpp src/node.cpp)