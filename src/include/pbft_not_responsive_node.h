/**
 * @file pbft_not_responsive_node.h
 * @author Abigale Kim
 * @brief Node that doesn't do anything.
 * 
 */

#include "pbft_node.h"

#ifndef __PBFT_NOT_RESPONSIVE_NODE_H__
#define __PBFT_NOT_RESPONSIVE_NODE_H__

class PBFTNotResponsiveNode : public PBFTNode {
  public:
    PBFTNotResponsiveNode(
      bool faulty, 
      uint64_t id, 
      uint64_t num_nodes, 
      uint64_t leader, 
      uint64_t f) 
    : PBFTNode(faulty, id, num_nodes, leader, f) {}

    void ExecuteCommand(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val) override;
};

#endif // __PBFT_NOT_RESPONSIVE_NODE_H__