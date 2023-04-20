/**
 * @file node.h
 * @author Abi Kim (abigalek@)
 * @brief defines server nodes. virtual class.
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef __NODE_H__
#define __NODE_H__

#include <string>

struct Message {
    uint64_t sender;
    std::string msg_;
    // other fields
};

class Node {
    public: 
        Node(bool faulty, uint64_t id): id_(id), faulty_(faulty) {}
        int GetId() const;
        std::string ToStr() const;

        // virtual methods for sending and receiving messages

        // Generates message to send to peer peer.
        virtual Message GenerateMessagePeer(uint64_t peer, uint64_t round_num);

        // Generates message to send to all nodes.
        virtual Message GenerateMessageAll(uint64_t round_num);

        void ReceiveMessage(Message&& message);
        virtual void ProcessMessages(uint64_t round_num);
    private:
        uint64_t id_;
        bool faulty_;
        std::vector<Message> messages_; // messages from the other nodes
};

#endif // __NODE_H__