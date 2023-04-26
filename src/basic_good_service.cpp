/**
 * @file basic_good_service.cpp
 * @author Abigale Kim (abigalek)
 * @brief Processes command for basic service.
 */

#include "basic_good_service.h"

#include <iostream>

void BasicGoodService::ProcessCommand(const std::string& command) {
  std::cout << "Command is: " << command << std::endl;
  // Generate and send all the messages per round
  std::vector<BasicGoodMessage> messages_to_send; 
  for (const auto &node :nodes_) {
      messages_to_send.emplace_back(node->GenerateMessageAll(round_num_));
  }

  for (uint64_t i = 0; i < nodes_.size(); ++i) {
      for (uint64_t j = 0; j < nodes_.size(); ++j) {
          if (i == j) continue;

          // Sending message from i to j.
          nodes_[j]->ReceiveMessage(messages_to_send[i]);
      }
  }
  
  for (const auto& node: nodes_) {
    node->ProcessMessages(round_num_);
  }
}