/**
 * @file node.h
 * @author Abigale Kim (abigalek)
 * @brief Virtual class that defines server nodes. Only really includes
 * basic info.
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