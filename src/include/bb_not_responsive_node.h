/**
 * @file bb_not_responsive_node.h
 * @author Abigale Kim
 * @brief Node that doesn't do anything.
 * 
 */

#include "bb_node.h"

#ifndef __BB_NOT_RESPONSIVE_NODE_H__
#define __BB_NOT_RESPONSIVE_NODE_H__

class BBNotResponsiveNode : public BBNode {
  public:
    BBNotResponsiveNode(
      bool faulty, 
      uint64_t id, 
      uint64_t num_nodes, 
      uint64_t leader, 
      uint64_t f) 
    : BBNode(faulty, id, num_nodes, leader, f) {}

    void ExecuteCommand(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command, std::promise<std::string>&& val) override;
};

#endif // __BB_NOT_RESPONSIVE_NODE_H__