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

#ifndef __PBFT_NODES_H__
#define __PBFT_NODES_H__

enum ClientReqType : char {
  PBFT_GET,
  PBFT_SET
};

struct ClientReq {
  ClientReqType type_;
  int num_; // if set, then set this here.
};

enum PBFTMessageType : char {
  REQUEST,
  PREPREPARE,
  PREPARE,
  COMMIT
};

struct PBFTMessage {
  PBFTMessageType type_;
  uint64_t view_number_; // leader
  uint64_t sequence_number_; // request number
  // other fields?
};

// Virtual class
class PBFTNode : public Node {
  public:
    PBFTNode(bool faulty, uint64_t id, uint64_t num_nodes) : Node(faulty, id, num_nodes) {}
    
    /** REQUEST STAGE */
    virtual void ReceiveRequestMsg(PBFTMessage& msg);

    /** PRE-PREPARE STAGE */
    virtual PBFTMessage GeneratePrePrepareMsg();
    virtual void ReceivePrePrepareMsg(PBFTMessage& msg);

    /** PREPARE STAGE */
    virtual PBFTMessage GeneratePrepareMsg();
    virtual void ReceivePrepareMsg(PBFTMessage& msg);

    /** COMMIT STAGE */
    virtual PBFTMessage GenerateCommitMsg();
    virtual void ReceiveCommitMsg(PBFTMessage& msg);
};

class PBFTGoodNode : public PBFTNode {
  public:
    PBFTGoodNode(bool faulty, uint64_t id, uint64_t num_nodes) : PBFTNode(faulty, id, num_nodes) {}

    /** REQUEST STAGE */
    void ReceiveRequestMsg(PBFTMessage& msg) override;

    /** PRE-PREPARE STAGE */
    PBFTMessage GeneratePrePrepareMsg() override;
    void ReceivePrePrepareMsg(PBFTMessage& msg) override;

    /** PREPARE STAGE */
    PBFTMessage GeneratePrepareMsg() override;
    void ReceivePrepareMsg(PBFTMessage& msg) override;

    /** COMMIT STAGE */
    PBFTMessage GenerateCommitMsg() override;
    void ReceiveCommitMsg(PBFTMessage& msg) override;
};

class PBFTByzantineNode : public PBFTNode {
  public:
    PBFTByzantineNode(bool faulty, uint64_t id, uint64_t num_nodes) : PBFTNode(faulty, id, num_nodes) {}

    /** REQUEST STAGE */
    void ReceiveRequestMsg(PBFTMessage& msg) override;

    /** PRE-PREPARE STAGE */
    PBFTMessage GeneratePrePrepareMsg() override;
    void ReceivePrePrepareMsg(PBFTMessage& msg) override;

    /** PREPARE STAGE */
    PBFTMessage GeneratePrepareMsg() override;
    void ReceivePrepareMsg(PBFTMessage& msg) override;

    /** COMMIT STAGE */
    PBFTMessage GenerateCommitMsg() override;
    void ReceiveCommitMsg(PBFTMessage& msg) override;
};

#endif // __PBFT_NODES_H__