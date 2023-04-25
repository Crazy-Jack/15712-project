/**
 * @file basic_good_service.cpp
 * @author your name (you@domain.com)
 * @brief Processes command for basic service.
 */

#include "pbft_service.h"

#include <condition_variable>
#include <optional>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
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
   || ) {
    return false;
  }

  // Send out prepare message
  PBFTMessage pre_prepare_msg = nodes[i]->GeneratePrepareMsg();
  for (uint64_t j = 0; j < nodes.size(); ++j) {
    if (j == nodes[i]->GetId()) {
      continue;
    } else {
      nodes[j]->SendMessage(pre_prepare_msg);
    }
  }

  std::vector<PBFTMessage> prepare_messages = nodes[i]->ReceivePrepareMsg();
  for (const auto& msg : prepare_messages) {
    // checks signatures (hash, here), replica number = current view, 2f prepare
    // messages match w the pre-prepare message

    if (false) {
    }
  }
}

std::optional<std::string> ProcessCommandReplica(int i, std::vector<std::shared_ptr<PBFTNode>>& nodes) {
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
    return std::make_optional<std::string>(nodes[i]->ReplyRequest());
  }

  return std::nullopt;
}

void PBFTService::ProcessCommand(const std::string& command) {
  // Process client request
  



  //create n -1 threads that execute this
	{
    // index into nodes
		// receives pre-prepare message
		// sends out a prepare message
		// processes prepare messages from other nodes
		// checks signatures (hash, here), replica number = current view, 2f prepare
		// messages match w the pre-prepare message
		// if this is true, then send out commit message
		// if accumulate 2f commit messages then send reply to client
	}
  
	// leader thread 
  {
		// receives client request
		// multicasts pre-prepare message to the nodes
		// sends out a prepare message
		// processes prepare messages from other nodes
		// checks signatures (hash, here), replica number = current view, 2f prepare
		// messages match w the pre-prepare message
		// if this is true, then send out commit message
		// if accumulate 2f commit messages then send reply to client
	}


}