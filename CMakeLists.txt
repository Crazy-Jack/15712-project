cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(Byzantine-Simulator DESCRIPTION "15-712 course project")

add_executable(bft-sim 
  src/simulator.cpp 
  src/service.cpp 
  src/basic_good_service.cpp 
  src/node.cpp 
  src/basic_good_node.cpp)
  
target_include_directories(bft-sim PUBLIC ${CMAKE_SOURCE_DIR}/src/include)