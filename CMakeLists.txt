cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Byzantine-Simulator DESCRIPTION "15-712 course project")

add_executable(bft-sim 
  src/simulator.cpp 
  src/service.cpp 
  src/include/non_replicated_service.hpp
  src/node.cpp 
  src/pbft_service.cpp
  src/pbft_nodes.cpp)
  
target_include_directories(bft-sim PUBLIC ${CMAKE_SOURCE_DIR}/src/include)