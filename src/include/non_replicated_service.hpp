/**
 * @file non_replicated_service.hpp
 * @author Abigale Kim (abigalek)
 * @brief Basic, unreplicated service.
 */

#include "service.h"

#include <iostream>
#include <sstream>
#include <string>

#ifndef __NON_REPLICATED_SERVICE_H__
#define __NON_REPLICATED_SERVICE_H__

class NonReplicatedService : public Service {
  public:
    NonReplicatedService() {}

    void ProcessCommand(const std::string& command) override;

  private:
    int val_;
};

void NonReplicatedService::ProcessCommand(const std::string& command) {
  if (command.compare("g") == 0) {
    std::cout << val_ << std::endl;
  } else {
    std::stringstream ss(command);  
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> commands(begin, end);
    val_ = std::stoi(commands[1]);
    std::cout << "SET " << val_ << std::endl;
  }
}

#endif // __NON_REPLICATED_SERVICE_H__