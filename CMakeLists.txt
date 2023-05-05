cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
project(Byzantine-Simulator DESCRIPTION "15-712 course project")

add_executable(bft-sim 
  src/simulator.cpp 
  src/service.cpp 
  src/include/non_replicated_service.hpp
  src/node.cpp 
  src/pbft_service.cpp
  src/pbft_node.cpp
  src/pbft_good_node.cpp
  src/pbft_wrong_node.cpp
  src/pbft_not_responsive_node.cpp
  src/bb_service.cpp 
  src/bb_node.cpp
  src/bb_good_node.cpp  
  src/bb_not_responsive_node.cpp
  src/lib.cpp)
  
target_include_directories(bft-sim PUBLIC ${CMAKE_SOURCE_DIR}/src/include)

# Importing OpenSSL
find_package(OpenSSL REQUIRED)
target_link_libraries(bft-sim OpenSSL::Crypto)
