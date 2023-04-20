/**
 * @file basic_good_node.h
 * @author your name (you@domain.com)
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

class BasicGoodNode : public Node {
  public:
    BasicGoodNode(bool faulty, uint64_t id) : Node(faulty, id) {}

    // Generates message to send to peer peer.
    Message GenerateMessagePeer(uint64_t peer, uint64_t round_num) override;

        // Generates message to send to all nodes.
    Message GenerateMessageAll(uint64_t round_num) override;
    void ProcessMessages(uint64_t round_num) override;
};

#endif //__BASIC_GOOD_NODE_H__