/**
 * @file pbft_wrong_node.h
 * @author Abigale Kim
 * @brief PBFT wrong node.
 * 
 * Byzantine behavior spec. As leader, it will send half GET, half SET 1 requests to replicas.
 * As a replica, it will convert GET to SET 2 AND SET n to SET n+1. 
 * Also it doesn't perform validation or view change.
 */

#include "pbft_node.h"

#ifndef __PBFT_WRONG_NODE_H__
#define __PBFT_WRONG_NODE_H__

class PBFTWrongNode : public PBFTNode {
  public:
    PBFTWrongNode(bool faulty, uint64_t id, uint64_t num_nodes, uint64_t leader, uint64_t f) : PBFTNode(faulty, id, num_nodes, leader, f) {
      type_ = PBFTNodeType::GOOD_NODE;
      if (leader == id) {
        leader_ = true;
        view_number_ = id;
      }
    }

    void ExecuteCommand(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val) override;

    inline uint64_t GetViewNumber() { return view_number_; }
    inline uint64_t GetF() { return f_; }

  private:
    bool CommandValidation(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command);

    void ViewChange(std::vector<std::shared_ptr<PBFTNode>>& nodes);

    /** Checks for the existence of one pre-prepare message. */
    bool AllPrePrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** Checks for the existence of 2f prepare messages. */
    bool AllPrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** Checks for the existence of 2f commit messages. */
    bool AllCommitMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** Checks for the existence of 3f view change messages. */
    bool AllViewChangeMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** Checks for the existence of one new view message. */
    bool AllNewViewMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

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

    /** VIEW CHANGE + NEW VIEW */
    PBFTMessage GenerateViewChangeMsg(); // replica only
    std::vector<PBFTMessage> ReceiveViewChangeMsg(); // leader only
    PBFTMessage GenerateNewViewMsg(); // leader only 
    std::vector<PBFTMessage> ReceiveNewViewMsg(); // replica only
    void ClearQueue(uint64_t new_view_num);

    // Value the good nodes are keeping track of:
    int val_; // the one value the nodes are updating

    // Keeps track of state sent by pre-prepare message locally
    std::string local_message_;
    uint64_t local_sequence_num_{0};
    uint64_t local_view_number_{0};

    // Whether the node is a leader or not
    bool leader_{false};

    // Keeps track of last successful sequence number (for view change).
    uint64_t valid_sequence_num_{0};
    // Keeps track of valid view change messages (for view change).
    std::vector<PBFTMessage> view_change_msgs_;

    // Static member to account for global timer (in milliseconds)
    static const uint64_t timeout{500};
};

#endif // __PBFT_WRONG_NODE_H__