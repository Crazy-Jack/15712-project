/**
 * @file basic_good_node.h
 * @author Abigale Kim (abigalek)
 * @brief basic good node
 * @version 0.1
 * @date 2023-04-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __BASIC_GOOD_NODE_H__
#define __BASIC_GOOD_NODE_H__

#include "node.h"

struct BasicGoodMessage {
    uint64_t sender_;
    std::string msg_;

    BasicGoodMessage(){}
    BasicGoodMessage(uint64_t sender, std::string&& msg) : sender_(sender), msg_(std::move(msg)) {}
    // other fields

    std::string ToStr() const {
      return "From Node " + std::to_string(sender_) + ": " + msg_;
    }
};

class BasicGoodNode : public Node {
  public:
    BasicGoodNode(bool faulty, uint64_t id, uint64_t num_nodes) : Node(faulty, id, num_nodes) {
      messages_.resize(num_nodes);
    }

    void ReceiveMessage(BasicGoodMessage message);

    // Generates message to send to peer peer.
    BasicGoodMessage GenerateMessagePeer(uint64_t peer, uint64_t round_num);

        // Generates message to send to all nodes.
    BasicGoodMessage GenerateMessageAll(uint64_t round_num);
    void ProcessMessages(uint64_t round_num);

  private:
    std::vector<BasicGoodMessage> messages_; // messages from the other nodes
};

#endif //__BASIC_GOOD_NODE_H__