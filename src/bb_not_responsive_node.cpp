/**
 * @file bb_not_responsive_node.cpp
 * @author Abigale Kim
 * @brief Implementation of not responsive node.
 * 
 */

#include "bb_not_responsive_node.h"

void BBNotResponsiveNode::ExecuteCommand(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command, std::promise<std::string>&& val) {
  (void)nodes;
  (void)command;
  val.set_value("NULL");
}