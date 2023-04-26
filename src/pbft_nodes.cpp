/**
 * @file pbft_nodes.cpp
 * @author your name (you@domain.com)
 * @brief stuff
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "pbft_nodes.h"

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>

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
  bool PBFTNode::AllPrePrepareMsgExist() {
    // Assumes you have the queue lock.
    for (const auto& msg : queue_) {
      if (msg.type_ == PBFTMessageType::PREPREPARE) {
        return true;
      }
    }
    return false;
  }

  /** Checks for the existence of 3f prepare messages. */
  bool PBFTNode::AllPrepareMsgExist() {
    uint64_t count = 0;
    for (const auto& msg : queue_) {
      if (msg.type_ == PBFTMessageType::PREPARE) {
        count += 1;
      }
    }
    return count == f_ * 3;
  }

  /** Checks for the existence of 3f commit messages. */
  bool PBFTNode::AllCommitMsgExist() {
    uint64_t count = 0;
    for (const auto& msg : queue_) {
      if (msg.type_ == PBFTMessageType::COMMIT) {
        count += 1;
      }
    }
    return count == f_ * 3;
  }

void PBFTNode::SendMessage(PBFTMessage message) {
  queue_lock_.lock();
  queue_.push_back(message);
  queue_lock_.unlock();

  queue_cond_var_.notify_all();
}

/** REQUEST STAGE */
void PBFTGoodNode::ReceiveRequestMsg(const std::string& command) {
  local_message_ = command;
}

/** PRE-PREPARE STAGE */
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

/** PREPARE STAGE */
PBFTMessage PBFTGoodNode::GeneratePrepareMsg()  {
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

/** COMMIT STAGE */
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

/** REPLY */
std::string PBFTGoodNode::ReplyRequest()  {
  ClientReq req = process_client_req(local_message_);
  if (req.type_ == ClientReqType::PBFT_GET) {
    return std::to_string(val_);
  }

  val_ = req.num_;
  return "SET " + std::to_string(req.num_);
}