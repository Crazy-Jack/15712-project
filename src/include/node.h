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

class Node {
    public: 
        Node(bool faulty, uint64_t id, uint64_t num_nodes): id_(id), faulty_(faulty) {}
        uint64_t GetId() const;
        std::string ToStr() const;
    protected:
        uint64_t id_;
        bool faulty_;
};

#endif // __NODE_H__