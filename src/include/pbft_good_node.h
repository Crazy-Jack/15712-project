/**
 * @file pbft_good_node.h
 * @author Abigale Kim
 * @brief Header file to define the PBFTGoodNode class, which is a
 * class that implements good nodes in the PBFT consensus protocol.
 */

#include "pbft_node.h"

#ifndef __PBFT_GOOD_NODE_H__
#define __PBFT_GOOD_NODE_H__

class PBFTGoodNode : public PBFTNode {
  public:
    PBFTGoodNode(bool faulty, uint64_t id, uint64_t num_nodes, uint64_t leader, uint64_t f) : PBFTNode(faulty, id, num_nodes, leader, f) {
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
    /** 
     * Implements command validation, aka the request, pre-prepare, prepare,
     * and commit stages. Returns true if all of these work.
     */
    bool CommandValidation(std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command);

    /**
     * @brief Implements view change. Protocol ends when a leader that
     * successfully sends NEWVIEW message that is validated by the
     * other nodes.
     */
    void ViewChange(std::vector<std::shared_ptr<PBFTNode>>& nodes);

    /** Checks for the existence of one pre-prepare message or that the timer has passed. */
    bool AllPrePrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** Checks for the existence of 3f prepare messages or that the timer has passed. */
    bool AllPrepareMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** Checks for the existence of 3f commit messages or that the timer has passed. */
    bool AllCommitMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** Checks for the existence of 3f view change messages or that the timer has passed. */
    bool AllViewChangeMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** Checks for the existence of one new view message or that the timer has passed. */
    bool AllNewViewMsgExist(std::chrono::time_point<std::chrono::steady_clock> &start);

    /** REQUEST STAGE */

    /** Method so that the leader node processes the request in the client. */
    void ReceiveRequestMsg(const std::string& command);

    /** PRE-PREPARE STAGE */

    /** 
     * Return the pre-prepare message to multicast to replicas. Only called in
     * the leader node.
    */
    PBFTMessage GeneratePrePrepareMsg();

    /** Receive the pre-prepare message. Called only in the replica nodes. */
    std::vector<PBFTMessage> ReceivePrePrepareMsg();
    
    /** If the pre-prepare message is validated, then replica nodes should
     * update their own state with the pre-prepare info.
    */
    void SetPrePrepareMsgState(const PBFTMessage& msg);

    /** PREPARE STAGE */

    /** Returns the prepare message to multicast to other nodes. */
    PBFTMessage GeneratePrepareMsg();

    /** Collects prepare messages from other nodes. */
    std::vector<PBFTMessage> ReceivePrepareMsg();

    /** COMMIT STAGE */

    /** Returns the commit message to multicast to other nodes. */
    PBFTMessage GenerateCommitMsg();

    /** Collects commit messages from other nodes. */
    std::vector<PBFTMessage> ReceiveCommitMsg();

    /** REPLY STAGE */

    /** Generates the response to reply to the client with. */
    std::string ReplyRequest();

    /** VIEW CHANGE + NEW VIEW */

    /** 
     * Returns the view change message that a replica who thinks leader
     * is bad should send to other nodes.
    */
    PBFTMessage GenerateViewChangeMsg();

    /** Collects the view changes messages received by the other nodes. */
    std::vector<PBFTMessage> ReceiveViewChangeMsg();

    /** 
     * Returns the new view message that should be sent to the other
     * nodes. Called by new leader only.
    */
    PBFTMessage GenerateNewViewMsg();

    /** 
     * Collects the new view messages received by the other nodes. 
     * Called in the new replicas only.
    */
    std::vector<PBFTMessage> ReceiveNewViewMsg(); // replica only
    void ClearQueue(uint64_t new_view_num);

    // Value the good nodes are keeping track of.
    int val_;

    /** Keep track of state sent by pre-prepare message locally. */
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
    static const uint64_t timeout{1000};
};

#endif // __PBFT_GOOD_NODE_H__