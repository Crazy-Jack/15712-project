/**
 * @file pbft_good_node.h
 * @author Abigale Kim
 * @brief PBFT good node
 */

#include "pbft_node.h"

#ifndef __PBFT_GOOD_NODE_H__
#define __PBFT_GOOD_NODE_H__

class PBFTGoodNode : public PBFTNode {
  public:
    PBFTGoodNode(bool faulty, uint64_t id, uint64_t num_nodes, uint64_t leader, uint64_t f) : PBFTNode(faulty, id, num_nodes, leader, f) {
      type_ = PBFTNodeType::GOOD_NODE;
    }

    void ExecuteCommand(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val) override;

    inline uint64_t GetViewNumber() { return view_number_; }
    inline uint64_t GetF() { return f_; }

  private:
    bool CommandValidation(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command);

    /** Checks for the existence of one pre-prepare message. */
    bool AllPrePrepareMsgExist();

    /** Checks for the existence of 2f prepare messages. */
    bool AllPrepareMsgExist();

    /** Checks for the existence of 2f commit messages. */
    bool AllCommitMsgExist();

    /** REQUEST STAGE */
    void ReceiveRequestMsg(const std::string& command);

    /** PRE-PREPARE STAGE */
    PBFTMessage GeneratePrePrepareMsg(); // leader only
    std::vector<PBFTMessage> ReceivePrePrepareMsg();
    void SetPrePrepareMsgState(const PBFTMessage& msg);

    /** PREPARE STAGE */
    PBFTMessage GeneratePrepareMsg();
    std::vector<PBFTMessage> ReceivePrepareMsg();

    /** COMMIT STAGE */
    PBFTMessage GenerateCommitMsg();
    std::vector<PBFTMessage> ReceiveCommitMsg();

    /** REPLY */
    std::string ReplyRequest();

    // Value the good nodes are keeping track of:
    int val_; // the one value the nodes are updating

    // Keeps track of state sent by pre-prepare message locally
    std::string local_message_;
    uint64_t local_sequence_num_{0};
    uint64_t local_view_number_{0};

    // Whether the node is a leader or not
    bool leader_{false};
};

#endif // __PBFT_GOOD_NODE_H__