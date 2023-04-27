/**
 * @file pbft_nodes.h
 * @author Abigale Kim
 * @brief Node classes used in the PBFT protocol.
 */

#include "lib.h"
#include "node.h"

#include <condition_variable>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifndef __PBFT_NODES_H__
#define __PBFT_NODES_H__

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
  COMMIT
};

/** Helper functions */
std::string PBFTMessageTypeToStr(PBFTMessageType type);

struct PBFTMessage {
  PBFTMessageType type_;
  uint64_t sender_; // sender of the node
  uint64_t view_number_; // leader
  uint64_t sequence_number_; // request number
  std::string data_;
  std::string data_hash_;
  // other fields?

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

// Virtual class
class PBFTNode : public Node {
  public:
    PBFTNode(bool faulty, uint64_t id, uint64_t num_nodes, uint64_t leader, uint64_t f) 
    : Node(faulty, id, num_nodes)
    , view_number_(leader)
    , f_(f) {}

    void SendMessage(PBFTMessage message);
    inline uint64_t GetViewNumber() { return view_number_; }
    inline uint64_t GetF() { return f_; }
    
    /** REQUEST STAGE */
    virtual void ReceiveRequestMsg(const std::string& command) = 0; // leader only

    /** PRE-PREPARE STAGE */
    virtual PBFTMessage GeneratePrePrepareMsg() = 0; // leader only
    virtual std::vector<PBFTMessage> ReceivePrePrepareMsg() = 0;
    virtual void SetPrePrepareMsgState(const PBFTMessage& msg) = 0;

    /** PREPARE STAGE */
    virtual PBFTMessage GeneratePrepareMsg() = 0;
    virtual std::vector<PBFTMessage> ReceivePrepareMsg()= 0;

    /** COMMIT STAGE */
    virtual PBFTMessage GenerateCommitMsg() = 0;
    virtual std::vector<PBFTMessage> ReceiveCommitMsg() = 0;

    /** REPLY */
    virtual std::string ReplyRequest() = 0;
  
  protected:
    /** Checks for the existence of one pre-prepare message. */
    bool AllPrePrepareMsgExist();

    /** Checks for the existence of 2f prepare messages. */
    bool AllPrepareMsgExist();

    /** Checks for the existence of 2f commit messages. */
    bool AllCommitMsgExist();

    std::list<PBFTMessage> queue_;
    std::mutex queue_lock_;
    std::condition_variable queue_cond_var_;
    uint64_t view_number_; // who it thinks the leader is
    std::vector<PBFTMessage> log_; // logs all the messages locally
    int val_; // the one value the nodes are updating
    uint64_t f_; // number of nodes

    // Keeps track of state sent by pre-prepare message locally
    std::string local_message_;
    uint64_t local_sequence_num_{0};
    uint64_t local_view_number_{0};
};

class PBFTGoodNode : public PBFTNode {
  public:
    PBFTGoodNode(bool faulty, uint64_t id, uint64_t num_nodes, uint64_t leader, uint64_t f) : PBFTNode(faulty, id, num_nodes, leader, f) {}

    /** REQUEST STAGE */
    void ReceiveRequestMsg(const std::string& command) override;

    /** PRE-PREPARE STAGE */
    PBFTMessage GeneratePrePrepareMsg() override; // leader only
    std::vector<PBFTMessage> ReceivePrePrepareMsg() override;
    void SetPrePrepareMsgState(const PBFTMessage& msg) override;

    /** PREPARE STAGE */
    PBFTMessage GeneratePrepareMsg() override;
    std::vector<PBFTMessage> ReceivePrepareMsg() override;

    /** COMMIT STAGE */
    PBFTMessage GenerateCommitMsg() override;
    std::vector<PBFTMessage> ReceiveCommitMsg() override;

    /** REPLY */
    std::string ReplyRequest() override;
};

#endif // __PBFT_NODES_H__