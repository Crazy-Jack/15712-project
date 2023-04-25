/**
 * @file basic_good_service.cpp
 * @author your name (you@domain.com)
 * @brief Processes command for basic service.
 */

#include "pbft_service.h"

#include <iostream>
#include <sstream>
#include <string>

void PBFTService::ProcessCommand(const std::string& command) {
  // Process client request
  ClientReq client_req;
  if (command.compare("g") == 0) {
    client_req.type_ = ClientReqType::PBFT_GET;
  } else {
    std::stringstream ss(command);  
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> commands(begin, end);
    client_req.type_ = ClientReqType::PBFT_SET;
    client_req.num_ = std::stoi(commands[1]);
  }

  
}