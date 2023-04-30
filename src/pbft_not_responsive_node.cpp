/**
 * @file pbft_not_responsive_node.cpp
 * @author Abigale Kim
 * @brief Implementation of not responsive node.
 * 
 */

#include "pbft_not_responsive_node.h"

void PBFTNotResponsiveNode::ExecuteCommand(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val) {
  (void)nodes;
  (void)command;
  val.set_value("NULL");
}