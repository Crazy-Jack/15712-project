/**
 * @file basic_good_service.cpp
 * @author your name (you@domain.com)
 * @brief Processes command for basic service.
 */

#include "basic_good_service.h"

#include <iostream>

void BasicGoodService::ProcessCommand(const std::string& command) {
  std::cout << "Command is: " << command << std::endl;
  BroadcastAll();
  RunRoundProcess();
}