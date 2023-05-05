/**
 * @file bb_nodes.h
 * @author Tianqin
 * @brief Node classes used in the BB protocol.
 */

#include "lib.h"
#include "node.h"

#include <condition_variable>
#include <future>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifndef __BB_NODE_H__
#define __BB_NODE_H__

enum BBClientReqType : char {
  BB_GET,
  BB_SET
};

struct BBClientReq {
  BBClientReqType type_;
  int num_; // if set, then set this here.

  BBClientReq(BBClientReqType type, int num) : type_(type), num_(num) {}
};


enum BBMessageType : char {
  // TBD what types of BB message outthere,
  // there should be ⟨m⟩{ω,α} 
  BB_REQUEST,
  BB_PREPREPARE, // prepare
  BB_PREPARE, // prepare 
  BB_OUTPUT, // message type that suggest the sender had made up its mind
};


/** Helper functions */
std::string BBMessageTypeToStr(BBMessageType type);
BBMessageType StrToBBMessageType(const std::string& str);
BBClientReq bb_process_client_req(const std::string& command);

struct BBMessage {
  BBMessageType type_;
  uint64_t sender_; // sender of the node
  uint64_t leader_id_; // leader
  std::string data_;
  std::string data_hash_;
  bool BOT;
  uint64_t phase_k_;

  BBMessage() {}

  BBMessage(BBMessageType type, 
  uint64_t sender, uint64_t leader_id_,
  std::string data, bool bot, uint64_t phase_k_) : type_(type)
  , sender_(sender)
  , data_(data), leader_id_(leader_id_), BOT(bot)
  , phase_k_(phase_k_) {
    data_hash_ = sha256(data);
  }

  std::string ToStr() const {
    return "Type: " + BBMessageTypeToStr(type_) + ", Sender: "
    + std::to_string(sender_) + ", Leader Num: " + std::to_string(leader_id_)
    + ", Data: " + data_ + ", BOT: " + (BOT ? "true" : "false");
  }
};

BBMessage StrToBBMessage(std::string str);

enum BBNodeType : char {
  BB_GOOD_NODE,
  BB_WRONG_NODE,
  BB_NON_RESPONSIVE_NODE
};

// Virtual class
class BBNode : public Node {
  public:
    BBNode(bool faulty, uint64_t id, uint64_t num_nodes, uint64_t leader, uint64_t f) 
    : Node(faulty, id, num_nodes)
    , local_lead_id_(leader)
    , num_nodes(num_nodes)
    ,f_(f) {}

    void SendMessage(BBMessage message);
    void SendNotifyAll();
    virtual void ExecuteCommand(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command, std::promise<std::string>&& val) = 0;
  protected:
    std::list<BBMessage> queue_;
    std::mutex queue_lock_;
    std::condition_variable queue_cond_var_;
    std::vector<BBMessage> log_; // logs all the messages locally
    uint64_t f_; // number of faulty nodes
    uint64_t local_lead_id_; // store leader id locally
    BBNodeType type_; // Type of BB Node
    uint64_t num_nodes; 
    
    // init
    std::map<uint64_t, bool> local_node_output_status_; // bool for each node in the neighborhood, indicating whether it outputed or not
    for (uint64_t i = 0; i < num_nodes; ++i) {
      local_node_output_status_.insert({i, false});
    }
    
    std::map<uint64_t, std::string> local_node_output_data_; // outputted values of each node
    std::string new_init_message = "";
    for (uint64_t i = 0; i < num_nodes; ++i) {
      local_node_output_data_.insert({i, new_init_message});
    }

    std::map<uint64_t, bool> local_node_output_bot_; // bool for each node in the neighborhood, indicating whether it output bot or not
    for (uint64_t i = 0; i < num_nodes; ++i) {
      local_node_output_bot_.insert({i, false});
    }

};

#endif // __BB_NODE_H__