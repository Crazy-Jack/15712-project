/**
 * @file byzantine_nodes.h
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2023-04-20
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "node.h"

#include <condition_variable>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <thread>

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

struct PBFTMessage {
  PBFTMessageType type_;
  uint64_t sender_; // sender of the node
  uint64_t view_number_; // leader
  uint64_t sequence_number_; // request number
  std::string data_;
  int data_hash_;
  // other fields?
};

// Virtual class
class PBFTNode : public Node {
  public:
    PBFTNode(bool faulty, uint64_t id, uint64_t num_nodes) : Node(faulty, id, num_nodes) {}

    void SendMessage(PBFTMessage message);
    
    /** REQUEST STAGE */
    virtual PBFTMessage ReceiveRequestMsg(); // leader only

    /** PRE-PREPARE STAGE */
    virtual PBFTMessage GeneratePrePrepareMsg(); // leader only
    virtual std::vector<PBFTMessage> ReceivePrePrepareMsg();

    /** PREPARE STAGE */
    virtual PBFTMessage GeneratePrepareMsg();
    virtual std::vector<PBFTMessage> ReceivePrepareMsg();

    /** COMMIT STAGE */
    virtual PBFTMessage GenerateCommitMsg();
    virtual std::vector<PBFTMessage> ReceiveCommitMsg();

    /** REPLY */
    virtual std::string ReplyRequest();
  
  protected:
    std::list<PBFTMessage> queue_;
    std::mutex queue_lock_;
    std::condition_variable queue_cond_var_;
    uint64_t view_number_; // who it thinks the leader is
};

class PBFTGoodNode : public PBFTNode {
  public:
    PBFTGoodNode(bool faulty, uint64_t id, uint64_t num_nodes) : PBFTNode(faulty, id, num_nodes) {}

    /** REQUEST STAGE */
    PBFTMessage ReceiveRequestMsg() override;
};

class PBFTByzantineNode : public PBFTNode {
  public:
    PBFTByzantineNode(bool faulty, uint64_t id, uint64_t num_nodes) : PBFTNode(faulty, id, num_nodes) {}

    /** REQUEST STAGE */
    PBFTMessage ReceiveRequestMsg() override;
};

#endif // __PBFT_NODES_H__