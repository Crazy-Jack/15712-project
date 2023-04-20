/**
 * @file node.h
 * @author Abi Kim (abigalek@)
 * @brief defines server nodes
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <string>


class Node {
    public: 
        Node(bool faulty, int id): id_(id), faulty_(faulty) {}
        int GetId() const;
        std::string ToStr() const;
    private:
        int id_;
        bool faulty_;
};