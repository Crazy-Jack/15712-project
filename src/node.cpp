#include "node.h"

#include <cstdlib>

int Node::GetId() const {
    return id_;
}

std::string Node::ToStr() const {
    std::string faulty = faulty_ ? "true" : "false";
    return "Node " + std::to_string(id_) + ", faulty=" + faulty;
}

void Node::ReceiveMessage(Message&& message) {
    messages_[message.sender] = std::move(message);
}