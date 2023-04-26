/**
 * @file pbft_service.cpp
 * @author your name (you@domain.com)
 * @brief pbft service
 */

#include "pbft_service.h"

#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

bool ReplicaValidation(int i, std::vector<std::shared_ptr<PBFTNode>>& nodes) {
  // Get pre-prepare message
  std::vector<PBFTMessage> pre_prepare_msgs = nodes[i]->ReceivePrePrepareMsg();

  // Validate pre-prepare message
  if (pre_prepare_msgs.size() != 1) {
    return false;
  }

  // TODO: faulty node threshold
  PBFTMessage &pre_prepare_msg = pre_prepare_msgs[0];
  if (pre_prepare_msg.type_ != PBFTMessageType::PREPREPARE
   || pre_prepare_msg.data_hash_ != std::hash<std::string>{}(pre_prepare_msg.data_)
   || pre_prepare_msg.sender_ != pre_prepare_msg.view_number_
   || pre_prepare_msg.view_number_ != nodes[i]->GetViewNumber()) {
    return false;
  }

  // Send out prepare message
  PBFTMessage prepare_msg = nodes[i]->GeneratePrepareMsg();
  for (uint64_t j = 0; j < nodes.size(); ++j) {
    if (j == nodes[i]->GetId()) {
      continue;
    } else {
      nodes[j]->SendMessage(prepare_msg);
    }
  }

  std::vector<PBFTMessage> prepare_messages = nodes[i]->ReceivePrepareMsg();
  for (const auto& msg : prepare_messages) {
    // view number, sequence number, hash
    // checks signatures (hash, here), replica number = current view, 2f prepare
    // messages match w the pre-prepare message
    if (msg.data_hash_ != prepare_msg.data_hash_
     || msg.view_number_ != prepare_msg.view_number_
     || msg.sequence_number_ != prepare_msg.sequence_number_) {
      return false;
    }
  }

  return true;
}

void ProcessCommandReplica(int i, std::vector<std::shared_ptr<PBFTNode>>& nodes, std::promise<std::string>&& val) {
  while (!ReplicaValidation(i, nodes)) {
    /// TODO: view change
  }

  // Send out commit messages
  PBFTMessage commit_msg = nodes[i]->GenerateCommitMsg();
  for (uint64_t j = 0; j < nodes.size(); ++j) {
    if (j == nodes[i]->GetId()) {
      continue;
    } else {
      nodes[j]->SendMessage(commit_msg);
    }
  }

  std::vector<PBFTMessage> commit_msgs = nodes[i]->ReceiveCommitMsg();
  uint64_t f = (nodes.size() - 1)/3;
  if (commit_msgs.size() >= 2 * f) {
    val.set_value(nodes[i]->ReplyRequest());
    return;
  }

  val.set_value("");
}

void ProcessCommandLeader(int i, std::vector<std::shared_ptr<PBFTNode>>& nodes, const std::string& command, std::promise<std::string>&& val) {
  // Has client request (from simulation)
  nodes[i]->ReceiveRequestMsg(command);

  PBFTMessage pre_prepare_msg = nodes[i]->GeneratePrePrepareMsg();
  for (uint64_t j = 0; j < nodes.size(); ++j) {
    nodes[j]->SendMessage(pre_prepare_msg);
  }

  ProcessCommandReplica(i, nodes, std::move(val));
}

// SET VAL or VAL should be return value
void PBFTService::ProcessCommand(const std::string& command) {
  // Empty string = no value
  std::vector<std::promise<std::string>> return_promises(nodes_.size());
  std::vector<std::future<std::string>> return_futures;
  std::vector<std::thread> threads;

  for (uint64_t i = 0; i < nodes.size(); ++i) {
    return_futures.emplace_back(return_promises[i].get_future());
    if (i == primary_node_) {
      threads.emplace_back(ProcessCommandLeader, i, nodes_, command, std::move(return_promises[i]));
    } else {
      threads.emplace_back(ProcessCommandReplica, i, nodes_, std::move(return_promises[i]));
    }
  }

  // Join all threads
  for (auto& thread : threads) {
    thread.join();
  }

  std::unordered_map<std::string, uint64_t> count_vals;
  for (auto& future: return_futures) {
    std::string val = future.get();
    if (count_vals.find(val) == count_vals.end()) {
      count_vals.insert(std::make_pair(val, 1));
    } else {
      count_vals[val] += 1;
    }
  }

  for (const auto& elem : count_vals) {
    if (elem.second > 2 * f_ ) {
      std::cout << elem.first << std::endl;
    }
  }
}