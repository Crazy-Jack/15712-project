/**
 * @file pbft_nodes.cpp
 * @author your name (you@domain.com)
 * @brief stuff
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "pbft_nodes.h"

#include <mutex>
#include <condition_variable>

ClientReq process_client_req(const std::string& command) {
  ClientReqType type;
  int num = 0;
  if (command.compare("g") == 0) {
    type = ClientReqType::PBFT_GET;
  } else {
    std::stringstream ss(command);  
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> commands(begin, end);
    type = ClientReqType::PBFT_SET;
    num = std::stoi(commands[1]);
  }
  return ClientReq(type, num);
}

void PBFTNode::SendMessage(PBFTMessage message) {
  queue_lock_.lock();
  queue_.push_back(message);
  queue_lock_.unlock();
}

/** GOOD NODES */
PBFTMessage PBFTGoodNode::ReceiveRequestMsg() {
  std::unique_lock<std::mutex> lck(queue_lock_);
  while () {
    queue_cond_var_.wait(queue_lock_);
  }
}

/** BAD NODES */
PBFTMessage PBFTByzantineNode::ReceiveRequestMsg() {

}