/**
 * @file pbft_good_node.cpp
 * @author Abigale Kim (abigalek)
 * @brief Implementation of good nodes for PBFT protocol.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "pbft_node.h"
#include "pbft_good_node.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>

/** ***********************
 * MISC. HELPER FUNCTIONS
 **************************/
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

/** Checks for the existence of one pre-prepare message. */
bool PBFTGoodNode::AllPrePrepareMsgExist() {
  // Assumes you have the queue lock.
  for (const auto& msg : queue_) {
    if (msg.type_ == PBFTMessageType::PREPREPARE) {
      return true;
    }
  }
  return false;
}

/** Checks for the existence of 3f prepare messages. */
bool PBFTGoodNode::AllPrepareMsgExist() {
  uint64_t count = 0;
  for (const auto& msg : queue_) {
    if (msg.type_ == PBFTMessageType::PREPARE) {
      count += 1;
    }
  }
  return count == f_ * 3;
}

/** Checks for the existence of 3f commit messages. */
bool PBFTGoodNode::AllCommitMsgExist() {
  uint64_t count = 0;
  for (const auto& msg : queue_) {
    if (msg.type_ == PBFTMessageType::COMMIT) {
      count += 1;
    }
  }
  return count == f_ * 3;
}

bool PBFTGoodNode::CommandValidation(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command) {
  // Pre-prepare stage
  if (leader_) {
    // Has client request (from simulation)
    ReceiveRequestMsg(command);

    PBFTMessage pre_prepare_msg = GeneratePrePrepareMsg();
    for (uint64_t j = 0; j < nodes.size(); ++j) {
      if (GetId() == j) continue;
      else {
        nodes[j]->SendMessage(pre_prepare_msg);
      }
    }
  } else {
    std::vector<PBFTMessage> pre_prepare_msgs = ReceivePrePrepareMsg();
      if (pre_prepare_msgs.size() != 1) {
        return false;
      }

      // TODO: faulty node threshold
      PBFTMessage &pre_prepare_msg = pre_prepare_msgs[0];
      if (pre_prepare_msg.type_ != PBFTMessageType::PREPREPARE
      || pre_prepare_msg.data_hash_ != sha256(pre_prepare_msg.data_)
      || pre_prepare_msg.sender_ != pre_prepare_msg.view_number_) {
        return false;
      }

      SetPrePrepareMsgState(pre_prepare_msg);
  }

  // Prepare stage
  PBFTMessage prepare_msg = GeneratePrepareMsg();
  for (uint64_t j = 0; j < nodes.size(); ++j) {
    if (j == GetId()) {
      continue;
    } else {
      nodes[j]->SendMessage(prepare_msg);
    }
  }

  std::vector<PBFTMessage> prepare_msgs = ReceivePrepareMsg();
  uint64_t valid_prepare_msg_count = 0;
  for (const auto& msg : prepare_msgs) {
    // view number, sequence number, hash
    // checks signatures (hash, here), replica number = current view, 2f prepare
    // messages match w the pre-prepare message
    if (msg.type_ == PBFTMessageType::PREPARE
     && msg.data_hash_ == prepare_msg.data_hash_
     && msg.view_number_ == prepare_msg.view_number_
     && msg.sequence_number_ == prepare_msg.sequence_number_) {
      valid_prepare_msg_count += 1;
    } else if (msg.type_ == PBFTMessageType::PREPREPARE) {
      return false;
    }
  }

  if (valid_prepare_msg_count < f_ * 2) {
    return false;
  }

  // Commit stage
  // Send out commit messages
  PBFTMessage commit_msg = GenerateCommitMsg();
  for (uint64_t j = 0; j < nodes.size(); ++j) {
    if (j == GetId()) {
      continue;
    } else {
      nodes[j]->SendMessage(commit_msg);
    }
  }

  std::vector<PBFTMessage> commit_msgs = ReceiveCommitMsg();
  for (const auto &msg : commit_msgs) {
    if (msg.type_ == PBFTMessageType::PREPREPARE) {
      return false;
    }
  }

  return commit_msgs.size() >= 2 * f_;
}

void PBFTGoodNode::ExecuteCommand(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val) {
  while (!CommandValidation(nodes, command)) {
    // TODO: view change
  }

  val.set_value(ReplyRequest());
}

/** ***********************
 * REQUEST STAGE
 **************************/
void PBFTGoodNode::ReceiveRequestMsg(const std::string& command) {
  local_message_ = command;
}

/** ***********************
 * PRE-PREPARE STAGE
 **************************/
PBFTMessage PBFTGoodNode::GeneratePrePrepareMsg() {
  local_sequence_num_ += 1;
  local_view_number_ = GetId();
  return PBFTMessage(PBFTMessageType::PREPREPARE, GetId(), GetId(), local_sequence_num_, local_message_);
}

std::vector<PBFTMessage> PBFTGoodNode::ReceivePrePrepareMsg() {
  std::unique_lock lk(queue_lock_);
  queue_cond_var_.wait(lk, [&]{return AllPrePrepareMsgExist();});

  std::vector<PBFTMessage> res;
  for (auto &elem : queue_) {
    if (elem.type_ == PBFTMessageType::PREPREPARE) {
      res.push_back(elem);
    }
  }

  queue_.erase(std::remove_if(queue_.begin(), 
                              queue_.end(),
                              [](PBFTMessage &elem) { return elem.type_ == PBFTMessageType::PREPREPARE; }),
               queue_.end());

  lk.unlock();
  return res;
}

void PBFTGoodNode::SetPrePrepareMsgState(const PBFTMessage& msg) {
  local_message_ = msg.data_;
  local_view_number_ = msg.view_number_;
  local_sequence_num_ = msg.sequence_number_;
}

/** ***********************
 * PREPARE STAGE
 **************************/
PBFTMessage PBFTGoodNode::GeneratePrepareMsg() {
  return PBFTMessage(PBFTMessageType::PREPARE, GetId(), local_view_number_, local_sequence_num_, local_message_);
}

std::vector<PBFTMessage> PBFTGoodNode::ReceivePrepareMsg()  {
  std::unique_lock lk(queue_lock_);
  queue_cond_var_.wait(lk, [&]{return AllPrepareMsgExist();});

  std::vector<PBFTMessage> res;

  // We collect pre-prepare messages at any stage because it indicates a faulty leader node/another
  // node is attempting to masquerade as leader.
  for (auto &elem : queue_) {
    if (elem.type_ == PBFTMessageType::PREPREPARE || elem.type_ == PBFTMessageType::PREPARE) {
      res.push_back(elem);
    }
  }

  queue_.erase(std::remove_if(queue_.begin(), 
                              queue_.end(),
                              [](PBFTMessage &elem) { return elem.type_ == PBFTMessageType::PREPREPARE || elem.type_ == PBFTMessageType::PREPARE; }),
               queue_.end());

  lk.unlock();
  return res;
}

/** ***********************
 * COMMIT STAGE
 **************************/
PBFTMessage PBFTGoodNode::GenerateCommitMsg()  {
  return PBFTMessage(PBFTMessageType::COMMIT, GetId(), local_view_number_, local_sequence_num_, local_message_);
}

std::vector<PBFTMessage> PBFTGoodNode::ReceiveCommitMsg()  {
  std::unique_lock lk(queue_lock_);
  queue_cond_var_.wait(lk, [&]{return AllCommitMsgExist();});

  std::vector<PBFTMessage> res;

  // We collect pre-prepare messages at any stage because it indicates a faulty leader node/another
  // node is attempting to masquerade as leader.
  for (auto &elem : queue_) {
    if (elem.type_ == PBFTMessageType::PREPREPARE || elem.type_ == PBFTMessageType::COMMIT) {
      res.push_back(elem);
    }
  }

  queue_.erase(std::remove_if(queue_.begin(), 
                              queue_.end(),
                              [](PBFTMessage &elem) { return elem.type_ == PBFTMessageType::PREPREPARE || elem.type_ == PBFTMessageType::COMMIT; }),
               queue_.end());

  lk.unlock();
  return res;
}


/** ***********************
 * REPLY STAGE
 **************************/
std::string PBFTGoodNode::ReplyRequest()  {
  ClientReq req = process_client_req(local_message_);
  if (req.type_ == ClientReqType::PBFT_GET) {
    return std::to_string(val_);
  }

  val_ = req.num_;
  return "SET " + std::to_string(req.num_);
}