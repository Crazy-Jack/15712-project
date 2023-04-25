#include "node.h"

#include <cstdlib>

uint64_t Node::GetId() const {
    return id_;
}

std::string Node::ToStr() const {
    std::string faulty = faulty_ ? "true" : "false";
    return "Node " + std::to_string(id_) + ", faulty=" + faulty;
}