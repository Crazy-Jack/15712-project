
/**
 * @file pbft_good_node.cpp
 * @author Abigale Kim (abigalek)
 * @brief Implementation of good nodes for PBFT protocol.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "bb_node.h"
#include "bb_good_node.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <vector>

/** Checks for the existence of one pre-prepare message. */
bool BBGoodNode::AllPrePrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start) {
  // Assumes you have the queue lock.
  for (const auto& msg : queue_) {
    if (msg.type_ == BBMessageType::BB_PREPREPARE) {
      return true;
    }
  }
  std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
  uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  return milliseconds > timeout;
}


/** Checks for the existence of 3f prepare messages. */
bool BBGoodNode::AllPrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start) {
  uint64_t count = 0;
  for (const auto& msg : queue_) {
    if (msg.type_ == BBMessageType::BB_PREPARE) {
      count += 1;
    }
  }
  std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
  uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  return count == f_ * 2 || milliseconds > timeout;
}


/** Checks for the existence of star messages. */
bool BBGoodNode::CheckIstarExist(std::chrono::time_point<std::chrono::steady_clock> &start) {
  for (const auto& msg : queue_) {
    if (msg.type_ == BBMessageType::BB_OUTPUT) {
      return true;
    }
  }
  std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::steady_clock::now();
  uint64_t milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
  return milliseconds > timeout;
}


std::vector<BBMessage> BBGoodNode::ReceivePrePrepareMsg() {
  std::unique_lock lk(queue_lock_);
  std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
  queue_cond_var_.wait(lk, [&]{return AllPrePrepareMsgExist(start);});
  

  // adding the results of the preprepare messages
  std::vector<BBMessage> res;
  for (auto &elem : queue_) {
    if (elem.type_ == BBMessageType::BB_PREPREPARE) {
      res.push_back(elem);
    }
  }

  queue_.erase(std::remove_if(queue_.begin(), 
                              queue_.end(),
                              [](BBMessage &elem) { return elem.type_ == BBMessageType::BB_PREPREPARE; }),
               queue_.end());

  lk.unlock();
  return res;
}


std::vector<BBMessage> BBGoodNode::ReceivePrepareMsg()  {
  std::unique_lock lk(queue_lock_);
  std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
  queue_cond_var_.wait(lk, [&]{return AllPrepareMsgExist(start);});

  std::vector<BBMessage> res;

  // We collect pre-prepare messages at any stage because it indicates a faulty leader node/another
  // node is attempting to masquerade as leader.
  for (auto &elem : queue_) {
    if (elem.type_ == BBMessageType::BB_PREPREPARE || elem.type_ == BBMessageType::BB_PREPARE) {
      res.push_back(elem);
    }
  }

  queue_.erase(std::remove_if(queue_.begin(), 
                              queue_.end(),
                              [](BBMessage &elem) { return elem.type_ == BBMessageType::BB_PREPREPARE || elem.type_ == BBMessageType::BB_PREPARE; }),
               queue_.end());

  lk.unlock();
  return res;
}


std::vector<BBMessage> BBGoodNode::ReceiveIstarMsg()  {
  std::unique_lock lk(queue_lock_);
  std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
  queue_cond_var_.wait(lk, [&]{return CheckIstarExist(start);});

  std::vector<BBMessage> res;

  // We collect pre-prepare messages at any stage because it indicates a faulty leader node/another
  // node is attempting to masquerade as leader.
  for (auto &elem : queue_) {
    if (elem.type_ == BBMessageType::BB_PREPREPARE || elem.type_ == BBMessageType::BB_OUTPUT) {
      res.push_back(elem);
    }
  }

  queue_.erase(std::remove_if(queue_.begin(), 
                              queue_.end(),
                              [](BBMessage &elem) { return elem.type_ == BBMessageType::BB_PREPREPARE || elem.type_ == BBMessageType::BB_OUTPUT; }),
               queue_.end());

  lk.unlock();
  return res;
}

void BBGoodNode::SetPrePrepareMsgState(const BBMessage& msg) {
  local_message_ = msg.data_; // PrePrepare directly overwrite local_message_ from the leader 
  local_lead_id_ = msg.leader_id_;
}


void BBGoodNode::SetPrepareMsg(const BBMessage& msg, bool BOT) {
  if (BOT) {
    local_data_bot_ = true;
  }
}

void BBGoodNode::SetStarMsg(const BBMessage& msg) {
  local_node_output_status_[msg.sender_] = true;
  local_node_output_data_[msg.sender_] = msg.data_;

  if (msg.BOT) {
    local_node_output_bot_[msg.sender_] = true;
    local_data_bot_ = true;
  } 

  // TODO: Should you log the SubsetSuccessNodesIDs? not right now
}



BBMessage BBGoodNode::GenerateProposalMessage() {
    // TODO: generate proposal messages from leader
    // type: BBMessage::PREPREPAPARE 
    //   local_view_number_ = GetId();
    //   return BBMessage(BBMessageType::PREPREPARE, GetId(), GetId(), local_sequence_num_, local_message_);
  return BBMessage(BBMessageType::BB_PREPREPARE, GetId(), GetId(), local_message_, false, phase_k_);

}



BBMessage BBGoodNode::GenerateIstarMessage(bool BOT) {
    // TODO: generate I* messages
    return BBMessage(BBMessageType::BB_OUTPUT, GetId(), local_lead_id_, local_message_, BOT, phase_k_);
}



BBMessage BBGoodNode::GeneratePrepareMsg(bool BOT) {
    return BBMessage(BBMessageType::BB_PREPARE, GetId(), local_lead_id_, local_message_, BOT, phase_k_);
}




// BB should also main a status of its neighbors and the neighbors outputs
// if the neighbor ever outputs, it stop sending them messages

// phase 0 execution
bool BBGoodNode::CommandValidationPhase0(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command) {
    // return true if there is an output, otherwise return false
    phase_k_ = 0;
    // round 0
    BBMessage prepare_msg;
    if (leader_) {
        // leader send the message <m>{w} to all nodes
        // Has client request (from simulation)
        ReceiveRequestMsg(command);

        prepare_msg = GenerateProposalMessage();

        // send message to all nodes excepts the leader
        for (uint64_t j = 0; j < nodes.size(); ++j) {
            if (GetId() == j) continue;
            else {
                nodes[j]->SendMessage(prepare_msg);
            }
        }

    } else {
        // check the received preprepare message from the leader
        std::vector<BBMessage> pre_prepare_msgs = ReceivePrePrepareMsg();
        if (pre_prepare_msgs.empty()) return false;
        // verify the correctness of the preprepare
        BBMessage right_preprepare_msg;
        for (auto& msg : pre_prepare_msgs) {
            if (msg.type_ == BBMessageType::BB_PREPREPARE
            && msg.data_hash_ == sha256(msg.data_)
            && msg.sender_ == msg.leader_id_) {
                right_preprepare_msg = msg;
                break;
            }
        }
        SetPrePrepareMsgState(right_preprepare_msg);
    

        // round 1: (for non leader node only)
        // prepare message <m>{w, alpha} to be sent
        prepare_msg = GeneratePrepareMsg(false);

        // send message to all nodes excepts the alpha
        for (uint64_t j = 0; j < nodes.size(); ++j) {
            if (GetId() == j) continue;
            else {
                nodes[j]->SendMessage(prepare_msg);
            }
        }
    }

    // round 2: 
    // For any α ∈ P, on receiving at least
    // 2f + 1 distinct valid signatures for some m from
    // S ⊆ P, sets I := ⟨m⟩S , sends I* to everyone, and
    // outputs m.

    std::vector<BBMessage> prepare_msgs = ReceivePrepareMsg();
    uint64_t valid_prepare_msg_count = 0;
    // log the subset S node number

    // TODO: distinct number needs to be checked here for preventing bad nodes send fake data multiple times
    for (const auto& msg : prepare_msgs) {
        // view number, sequence number, hash
        // checks signatures (hash, here), replica number = current view, 2f prepare
        // messages match w the pre-prepare message
        if (msg.type_ == BBMessageType::BB_PREPARE
        && msg.data_hash_ == prepare_msg.data_hash_
        && msg.leader_id_ == prepare_msg.leader_id_) {
            valid_prepare_msg_count += 1;
        }
    }

    // TODO: check the SubsetSuccessNodesIDs has distinct 2f+1 values

    // Not enough prepare messages
    if (valid_prepare_msg_count < f_ * 2) {
        return false; // output condition not met
    }

    // Otherwise, set local variable state to <m>{S}
    // SetPrePrepareMsgStateStar(msg, SubsetSuccessNodesIDs);
    // generate message star message
    BBMessage prepare_istar_msg = GenerateIstarMessage(false);

    SetStarMsg(prepare_istar_msg);

    // send I * to everyone, broadcast
    for (uint64_t j = 0; j < nodes.size(); ++j) {
      if (!local_node_output_status_[j]) {
        nodes[j]->SendMessage(prepare_istar_msg);
      }
    }
    return true;
}

// phase 0 execution
void BBGoodNode::ReceiveStarMessages(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command) {
    // return true if there is an output, otherwise return false

    // round3: on receiving J*, set I = J*
    std::vector<BBMessage> prepare_msgs = ReceiveIstarMsg();
    for (const auto& msg : prepare_msgs) {
        // view number, sequence number, hash
        // checks signatures (hash, here), replica number = current view, 2f prepare
        // messages match w the pre-prepare message
        if (msg.type_ == BBMessageType::BB_OUTPUT) {
            // store J to I
            SetStarMsg(msg);
        }
    }

}


bool BBGoodNode::CommandValidationPhaseK_R1(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command) {
    // return true if there is an output, otherwise return false
    phase_k_ += 1;
    // round 1: For any α ∈ P, at start of round, sends
    // local variable I (could be ⊥) to every β ∈ P
    // (including the case β = α). 
    
    BBMessage broadcast_msg = GeneratePrepareMsg(local_data_bot_);

    // send I * to everyone, broadcast
    for (uint64_t j = 0; j < nodes.size(); ++j) {
      if (!local_node_output_status_[j]) {
        nodes[j]->SendMessage(broadcast_msg);
      }
    }

    std::vector<BBMessage> prepare_msgs = ReceivePrepareMsg();
    uint64_t valid_prepare_msg_bot_count = 0;
    uint64_t valid_prepare_msg_non_bot_count = 0;
    BBMessage botmessage;
    BBMessage nonbotmessage;

    // TODO: distinct number needs to be checked here for preventing bad nodes send fake data multiple times
    for (const auto& msg : prepare_msgs) {
        // view number, sequence number, hash
        // checks signatures (hash, here), replica number = current view, 2f prepare
        // messages match w the pre-prepare message
        if (msg.type_ == BBMessageType::BB_PREPARE
        && msg.data_hash_ == broadcast_msg.data_hash_
        && msg.leader_id_ == broadcast_msg.leader_id_) {

          if (msg.BOT) {
            valid_prepare_msg_bot_count += 1;
            botmessage = msg;
          } else {
            valid_prepare_msg_non_bot_count += 1;
            nonbotmessage = msg;
          }
        }
    }

    if (valid_prepare_msg_bot_count > 2 * f_ + 1) {
      // set local message to bot
      SetPrepareMsg(botmessage, true);
      // sends ⊥ * to everyone else and outputs ⊥
      // prepare bot messages
      BBMessage boardcast_bot_star_msg = GenerateIstarMessage(true);
      SetStarMsg(boardcast_bot_star_msg);

      // send I * to everyone, broadcast
      for (uint64_t j = 0; j < nodes.size(); ++j) {
        if (!local_node_output_status_[j]) {
          nodes[j]->SendMessage(boardcast_bot_star_msg);
        }
      }
      // wait to receive the star messages
      ReceiveStarMessages(nodes, command);

    } else if (valid_prepare_msg_non_bot_count > 2 * f_ + 1) {
      // set local with non bot message
      SetPrepareMsg(nonbotmessage, false);
      return false;

    } else {
      // set local variable to be bot
      SetPrepareMsg(botmessage, true);
      return false;

    }

    return true;
}


bool BBGoodNode::CommandValidationPhaseK_R2(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command) {
  BBMessage broadcast_msg = GeneratePrepareMsg(local_data_bot_);

  // send I * to everyone, broadcast
  for (uint64_t j = 0; j < nodes.size(); ++j) {
    if (!local_node_output_status_[j]) {
      nodes[j]->SendMessage(broadcast_msg);
    }
  }

  std::vector<BBMessage> prepare_msgs = ReceivePrepareMsg();
  uint64_t valid_prepare_msg_bot_count = 0;
  uint64_t valid_prepare_msg_non_bot_count = 0;
  BBMessage botmessage;
  BBMessage nonbotmessage;

  // TODO: distinct number needs to be checked here for preventing bad nodes send fake data multiple times
  for (const auto& msg : prepare_msgs) {
      // view number, sequence number, hash
      // checks signatures (hash, here), replica number = current view, 2f prepare
      // messages match w the pre-prepare message
      if (msg.type_ == BBMessageType::BB_PREPARE
      && msg.data_hash_ == broadcast_msg.data_hash_
      && msg.leader_id_ == broadcast_msg.leader_id_) {

        if (msg.BOT) {
          valid_prepare_msg_bot_count += 1;
          botmessage = msg;
        } else {
          valid_prepare_msg_non_bot_count += 1;
          nonbotmessage = msg;
        }
      }
  }

  if (valid_prepare_msg_bot_count > 2 * f_ + 1) {
      // set local message to bot
      SetPrepareMsg(botmessage, true);
      return false;

  } else if (valid_prepare_msg_non_bot_count > 2 * f_ + 1) {
      // set local with non bot message
      SetPrepareMsg(nonbotmessage, false);
      // generate star message
      BBMessage boardcast_non_bot_star_msg = GenerateIstarMessage(false);

      SetStarMsg(boardcast_non_bot_star_msg);

      // send I * to everyone, broadcast
      for (uint64_t j = 0; j < nodes.size(); ++j) {
        if (!local_node_output_status_[j]) {
          nodes[j]->SendMessage(boardcast_non_bot_star_msg);
        }
      }
      // wait to receive the star messages
      ReceiveStarMessages(nodes, command);
    } else {
      SetPrepareMsg(nonbotmessage, false);
      return false;
    }

    return true;
}


bool BBGoodNode::CommandValidationPhaseK_R3(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command) {
  // TODO: 
  BBMessage broadcast_msg = GeneratePrepareMsg(local_data_bot_);

  // send I * to everyone, broadcast
  for (uint64_t j = 0; j < nodes.size(); ++j) {
    if (!local_node_output_status_[j]) {
      nodes[j]->SendMessage(broadcast_msg);
    }
  }

  std::vector<BBMessage> prepare_msgs = ReceivePrepareMsg();
  uint64_t valid_prepare_msg_bot_count = 0;
  uint64_t valid_prepare_msg_non_bot_count = 0;
  BBMessage botmessage;
  BBMessage nonbotmessage;
  uint64_t min_k = 999999999999999999;

  // TODO: distinct number needs to be checked here for preventing bad nodes send fake data multiple times
  for (const auto& msg : prepare_msgs) {
    
    if (msg.type_ == BBMessageType::BB_PREPARE
      && msg.data_hash_ == broadcast_msg.data_hash_
      && msg.leader_id_ == broadcast_msg.leader_id_) {
        // get min k 

        if (msg.phase_k_ < min_k) {
          min_k = msg.phase_k_;
        }
        if (msg.BOT) {
          valid_prepare_msg_bot_count += 1;
          botmessage = msg;
        } else {
          valid_prepare_msg_non_bot_count += 1;
          nonbotmessage = msg;
        }
    }
    }

    if (valid_prepare_msg_bot_count > 2 * f_ + 1) {
      // set local message to bot
      SetPrepareMsg(botmessage, true);
      
    } else if (valid_prepare_msg_non_bot_count > 2 * f_ + 1) {
      // set local message to bot
      SetPrepareMsg(nonbotmessage, false);
      
    } else {
      // b := lsb(minα H(⟨k⟩{α}))
      uint8_t b = lsb_sha256(std::to_string(min_k));
      if (!b) {
        SetPrepareMsg(botmessage, true);
      } else {
        SetPrepareMsg(nonbotmessage, false);
      }
    }
  return false;
}


/** ***********************
 * REQUEST STAGE
 **************************/
void BBGoodNode::ReceiveRequestMsg(const std::string& command) {
  local_message_ = command;
}

/** ***********************
 * REPLY STAGE
 **************************/
std::string BBGoodNode::ReplyRequest()  {
  BBClientReq req = bb_process_client_req(local_message_);
  if (req.type_ == BBClientReqType::BB_GET) {
    return std::to_string(val_);
  }
  // TBD
  val_ = req.num_;
  return "SET " + std::to_string(req.num_);
  // return "SET 5";
}




void BBGoodNode::ExecuteCommand(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command, std::promise<std::string>&& val) {

  // phase 0
  bool outputted = false;
  outputted = CommandValidationPhase0(nodes, command);
  ReceiveStarMessages(nodes, command);

  // // phase k
  while (!outputted) {
    outputted = CommandValidationPhaseK_R1(nodes, command);
    if (outputted) {break;}
    outputted = CommandValidationPhaseK_R2(nodes, command);
    if (outputted) {break;}
    outputted = CommandValidationPhaseK_R3(nodes, command);
    if (outputted) {break;}
  }

  // return value
  val.set_value(ReplyRequest());
}

