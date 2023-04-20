#include <iostream>

#include "basic_good_node.h"

// Generates message to send to peer peer.
Message BasicGoodNode::GenerateMessagePeer(uint64_t peer, uint64_t round_num) {
  return {id_, "Good node"};
}

// Generates message to send to all nodes.
Message BasicGoodNode::GenerateMessageAll(uint64_t round_num) {
  return {id_, "Good node"};
}
void BasicGoodNode::ProcessMessages(uint64_t round_num) {
  std::cout << "Messages received from nodes in round " << round_num << std::endl;
  for (const auto& message: messages_) {
    std::cout << message.ToStr() << std::endl;
  }
}