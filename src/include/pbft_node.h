/**
 * @file pbft_nodes.h
 * @author Abigale Kim
 * @brief Node classes used in the PBFT protocol.
 */

#include "lib.h"
#include "node.h"

#include <condition_variable>
#include <future>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifndef __PBFT_NODE_H__
#define __PBFT_NODE_H__

enum ClientReqType : char {
  PBFT_GET,
  PBFT_SET
};

struct ClientReq {
  ClientReqType type_;
  int num_; // if set, then set this here.

  ClientReq(ClientReqType type, int num) : type_(type), num_(num) {}
};

enum PBFTMessageType : char {
  REQUEST,
  PREPREPARE,
  PREPARE,
  COMMIT,
  VIEWCHANGE,
  NEWVIEW
};

/** Helper functions */
std::string PBFTMessageTypeToStr(PBFTMessageType type);
PBFTMessageType StrToPBFTMessageType(const std::string& str);
ClientReq process_client_req(const std::string& command);

struct PBFTMessage {
  PBFTMessageType type_;
  uint64_t sender_; // sender of the node
  uint64_t view_number_; // leader
  uint64_t sequence_number_; // request number
  std::string data_;
  std::string data_hash_;
  std::vector<std::string> view_change_msgs_;

  PBFTMessage() {}

  PBFTMessage(PBFTMessageType type, 
  uint64_t sender, 
  uint64_t view_number, 
  uint64_t sequence_number, 
  std::string data) : type_(type)
  , sender_(sender)
  , view_number_(view_number)
  , sequence_number_(sequence_number)
  , data_(data) {
    data_hash_ = sha256(data);
  }

  std::string ToStr() const {
    return "Type: " + PBFTMessageTypeToStr(type_) + ", Sender: " 
    + std::to_string(sender_) + ", View Num: " + std::to_string(view_number_)
    + ", Sequence Number: " + std::to_string(sequence_number_) + ", Data: " + data_;
  }
};

PBFTMessage StrToPBFTMessage(std::string str);

enum PBFTNodeType : char {
  GOOD_NODE,
  WRONG_NODE,
  NON_RESPONSIVE_NODE
};

// Virtual class
class PBFTNode : public Node {
  public:
    PBFTNode(bool faulty, uint64_t id, uint64_t num_nodes, uint64_t leader, uint64_t f) 
    : Node(faulty, id, num_nodes)
    , view_number_(leader)
    ,f_(f) {}

    void SendMessage(PBFTMessage message);
    void SendNotifyAll();
    virtual void ExecuteCommand(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val) = 0;
  protected:
    std::list<PBFTMessage> queue_;
    std::mutex queue_lock_;
    std::condition_variable queue_cond_var_;
    uint64_t view_number_; // who it thinks the leader is
    std::vector<PBFTMessage> log_; // logs all the messages locally
    uint64_t f_; // number of faulty nodes
    PBFTNodeType type_; // Type of PBFT Node
};

#endif // __PBFT_NODE_H__