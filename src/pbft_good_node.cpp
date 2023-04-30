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
#include <chrono>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
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
bool PBFTGoodNode::AllPrePrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start) {
  // Assumes you have the queue lock.
  for (const auto& msg : queue_) {
    if (msg.type_ == PBFTMessageType::PREPREPARE) {
      return true;
    }
  }
  std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
  uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  return milliseconds > timeout;
}

/** Checks for the existence of 3f prepare messages. */
bool PBFTGoodNode::AllPrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start) {
  uint64_t count = 0;
  for (const auto& msg : queue_) {
    if (msg.type_ == PBFTMessageType::PREPARE) {
      count += 1;
    }
  }
  std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
  uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  return count == f_ * 3 || milliseconds > timeout;
}

/** Checks for the existence of 3f commit messages. */
bool PBFTGoodNode::AllCommitMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start) {
  uint64_t count = 0;
  for (const auto& msg : queue_) {
    if (msg.type_ == PBFTMessageType::COMMIT) {
      count += 1;
    }
  }
  std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
  uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  return count == f_ * 3 || milliseconds > timeout;
}

/** Checks for the existence of 2f view change messages. */
bool PBFTGoodNode::AllViewChangeMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start) {
  uint64_t count = 0;
  for (const auto& msg : queue_) {
    if (msg.type_ == PBFTMessageType::VIEWCHANGE) {
      count += 1;
    }
  }
  std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
  uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  return count == f_ * 2 || milliseconds > timeout;
}

/** Checks for the existence of one new view message. */
bool PBFTGoodNode::AllNewViewMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start) {
  // Assumes you have the queue lock.
  for (const auto& msg : queue_) {
    if (msg.type_ == PBFTMessageType::NEWVIEW) {
      return true;
    }
  }
  std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
  uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  return milliseconds > timeout;
}

bool PBFTGoodNode::CommandValidation(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command) {
  // Pre-prepare stage
  std::cout << "in command validation\n";
  if (leader_) {
    // Has client request (from simulation)
    ReceiveRequestMsg(command);
    std::cout << "leader received request\n";

    PBFTMessage pre_prepare_msg = GeneratePrePrepareMsg();
    for (uint64_t j = 0; j < nodes.size(); ++j) {
      if (GetId() == j) continue;
      else {
        nodes[j]->SendMessage(pre_prepare_msg);
      }
    }

    std::cout << "leader sent pre-prepare msgs\n";
  } else {
    std::vector<PBFTMessage> pre_prepare_msgs = ReceivePrePrepareMsg();
    std::cout << "replica received pre-prepare msgs\n";
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
      std::cout << "replica passed pre-prepare msgs validation\n";
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
  std::cout << "node sent prepare msgs\n";

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

  std::cout << "node passed prepare msg validation\n";

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

// Simplified view change protocol. 
void PBFTGoodNode::ViewChange(std::vector<std::shared_ptr<PBFTNode>>& nodes) {
  uint64_t new_view_number;
  assert(!leader_);
  while (true) {
    PBFTMessage view_change_msg = GenerateViewChangeMsg();
    uint64_t leader_iteration = view_change_msg.view_number_;
    for (uint64_t j = 0; j < nodes.size(); ++j) {
      if (j == GetId()) {
        continue;
      } else {
      nodes[j]->SendMessage(view_change_msg);
      }
    }

    if (id_ == leader_iteration) {
      std::vector<PBFTMessage> view_change_msgs = ReceiveViewChangeMsg();
      uint64_t count = 0;
      for (auto &msg : view_change_msgs) {
        if (msg.view_number_ == view_change_msg.view_number_
        || msg.sequence_number_ == view_change_msg.sequence_number_) {
          count += 1;
        }
      }

      if (count < 2 * f_) {
        continue;
      }

      PBFTMessage new_view_msg = GenerateNewViewMsg();
      for (uint64_t j = 0; j < nodes.size(); ++j) {
        if (j == GetId()) {
          continue;
        } else {
        nodes[j]->SendMessage(new_view_msg);
        }
      }
      break;
    } else {
      std::vector<PBFTMessage> new_view_msgs = ReceiveNewViewMsg();
      if (new_view_msgs.size() != 1) {
        continue;
      }

      PBFTMessage& new_view_msg = new_view_msgs[0];
      if (new_view_msg.view_number_ != leader_iteration
      || new_view_msg.sequence_number_ != view_change_msg.sequence_number_) {
        continue;
      }

      std::vector<PBFTMessage> vc_msg_from_leader;
      for (auto &str : new_view_msg.view_change_msgs_) {
        vc_msg_from_leader.push_back(StrToPBFTMessage(str));
      }
      std::vector<PBFTMessage> view_change_msgs = ReceiveViewChangeMsg();
      if (view_change_msgs.size() != vc_msg_from_leader.size()
      || view_change_msgs.size() != 2 * f_) {
        continue;
      }

      std::unordered_map<uint64_t, uint64_t> sender_to_count_map;
      for (auto &msg : vc_msg_from_leader) {
        if (msg.view_number_ == leader_iteration 
         && msg.sequence_number_ == view_change_msg.sequence_number_
         && msg.type_ == PBFTMessageType::VIEWCHANGE) {
          sender_to_count_map.insert(std::make_pair(msg.sender_, 1));
        }
      }

      for (auto &msg : view_change_msgs) {
        if (msg.view_number_ == leader_iteration 
         && msg.sequence_number_ == view_change_msg.sequence_number_
         && msg.type_ == PBFTMessageType::VIEWCHANGE) {
          if (sender_to_count_map.find(msg.sender_) == sender_to_count_map.end()) {
            sender_to_count_map.insert(std::make_pair(msg.sender_, 1));
          } else {
            sender_to_count_map[msg.sender_] += 1;
          }
        }
      }

      bool valid_map = true;
      for (auto &elem : sender_to_count_map) {
        if ((elem.first == id_ || elem.first == leader_iteration) && elem.second != 1) {
          valid_map = false;
        } else if (elem.second != 2) {
          valid_map = false;
        }
      }

      if (!valid_map) {
        continue;
      }

      break;
    }
  }

  // Update local state.
  view_number_ = new_view_number;
  ClearQueue(view_number_);
}

void PBFTGoodNode::ExecuteCommand(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val) {
  std::cout << "in execute command\n";
  while (!CommandValidation(nodes, command)) {
    ViewChange(nodes);
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
  std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
  queue_cond_var_.wait(lk, [&]{return AllPrePrepareMsgExist(start);});

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
  std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
  queue_cond_var_.wait(lk, [&]{return AllPrepareMsgExist(start);});

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
  std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
  queue_cond_var_.wait(lk, [&]{return AllCommitMsgExist(start);});

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
  valid_sequence_num_ = local_sequence_num_;

  ClientReq req = process_client_req(local_message_);
  if (req.type_ == ClientReqType::PBFT_GET) {
    return std::to_string(val_);
  }

  val_ = req.num_;
  return "SET " + std::to_string(req.num_);
}

/** ***********************
 * VIEW CHANGE/NEW VIEW STAGE
 **************************/
PBFTMessage PBFTGoodNode::GenerateViewChangeMsg() {
  uint64_t total_nodes = (3 * f_) + 1;
  return PBFTMessage(PBFTMessageType::VIEWCHANGE, id_, (view_number_ + 1) % total_nodes, valid_sequence_num_, "");
}

std::vector<PBFTMessage> PBFTGoodNode::ReceiveViewChangeMsg() {
  std::unique_lock lk(queue_lock_);
  std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
  queue_cond_var_.wait(lk, [&]{return AllViewChangeMsgExist(start);});

  std::vector<PBFTMessage> res;

  // We collect pre-prepare messages at any stage because it indicates a faulty leader node/another
  // node is attempting to masquerade as leader.
  for (auto &elem : queue_) {
    if (elem.type_ == PBFTMessageType::VIEWCHANGE) {
      res.push_back(elem);
      view_change_msgs_.push_back(elem);
    }
  }

  queue_.erase(std::remove_if(queue_.begin(), 
                              queue_.end(),
                              [](PBFTMessage &elem) { return elem.type_ == PBFTMessageType::VIEWCHANGE; }),
               queue_.end());

  lk.unlock();
  return res;
}

PBFTMessage PBFTGoodNode::GenerateNewViewMsg() {
  PBFTMessage msg = PBFTMessage(PBFTMessageType::NEWVIEW, id_, id_, valid_sequence_num_, "");
  std::vector<std::string> msgs_as_str;
  for (auto & vc_msg : view_change_msgs_) {
    msg.view_change_msgs_.push_back(vc_msg.ToStr());
  }
  return msg;
}

std::vector<PBFTMessage> PBFTGoodNode::ReceiveNewViewMsg() {
  std::unique_lock lk(queue_lock_);
  std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
  queue_cond_var_.wait(lk, [&]{return AllNewViewMsgExist(start);});

  std::vector<PBFTMessage> res;
  for (auto &elem : queue_) {
    if (elem.type_ == PBFTMessageType::NEWVIEW) {
      res.push_back(elem);
    }
  }

  queue_.erase(std::remove_if(queue_.begin(), 
                              queue_.end(),
                              [](PBFTMessage &elem) { return elem.type_ == PBFTMessageType::NEWVIEW; }),
               queue_.end());

  lk.unlock();
  return res;
}

void PBFTGoodNode::ClearQueue(uint64_t new_view_num) {
  std::unique_lock lk(queue_lock_);
  queue_.erase(std::remove_if(queue_.begin(), 
                              queue_.end(),
                              [&](PBFTMessage &elem) { return elem.view_number_ != new_view_num; }),
               queue_.end());
  lk.unlock();
  view_change_msgs_.clear();
}