
/**
 * @file bb_good_node.h
 * @author Abigale Kim
 * @brief BB good node
 */

#include "bb_node.h"

#ifndef __BB_GOOD_NODE_H__
#define __BB_GOOD_NODE_H__

class BBGoodNode : public BBNode {
  public:
    BBGoodNode(bool faulty, uint64_t id, uint64_t num_nodes, uint64_t leader, uint64_t f) : BBNode(faulty, id, num_nodes, leader, f) {
      type_ = BBNodeType::BB_GOOD_NODE;
      if (leader == id) {
        leader_ = true;
      }
      local_lead_id_ = id;
    }

    void ExecuteCommand(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command, std::promise<std::string>&& val) override;

    inline uint64_t GetF() { return f_; }

  private:
    bool CommandValidation(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command);

    void ViewChange(std::vector<std::shared_ptr<BBNode>>& nodes);

    /** Checks for the existence of one pre-prepare message. */
    bool AllPrePrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** Checks for the existence of 2f prepare messages. */
    bool AllPrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);


    /** Checks for the existence of one new view message. */
    bool CheckIstarExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** REQUEST STAGE */
    void ReceiveRequestMsg(const std::string& command);

    /** PRE-PREPARE STAGE */

    std::vector<BBMessage> ReceivePrePrepareMsg();
    std::vector<BBMessage> ReceivePrepareMsg();
    std::vector<BBMessage> ReceiveIstarMsg();

    void SetPrePrepareMsgState(const BBMessage& msg);
    void SetPrepareMsg(const BBMessage& msg, bool BOT);
    void SetStarMsg(const BBMessage& msg);

    BBMessage GenerateProposalMessage();
    BBMessage GenerateIstarMessage(bool BOT);
    BBMessage GeneratePrepareMsg(bool BOT);

    bool CommandValidationPhase0(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command);
    void ReceiveStarMessages(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command);
    bool CommandValidationPhaseK_R1(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command);
    bool CommandValidationPhaseK_R2(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command);
    bool CommandValidationPhaseK_R3(std::vector<std::shared_ptr<BBNode>>& nodes, std::string command);
    void Initialize();
    
    std::string ReplyRequest();
   
    // Value the good nodes are keeping track of:
    int val_; // the one value the nodes are updating

    // Keeps track of internal message state
    std::string local_message_;


    std::string local_state_data_{""}; // data that contains
    
    bool local_data_bot_{true}; // bool indicate if the local data is bot

    // Whether the node is a leader or not
    bool leader_{false};
    uint64_t phase_k_{0};

    // Static member to account for global timer (in milliseconds)
    static const uint64_t timeout{1000};
};

#endif // __BB_GOOD_NODE_H__

