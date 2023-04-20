/**
 * @file service.h
 * @author Abigale Kim
 * @brief service virtual class
 */

#ifndef __SERVICE_H__
#define __SERVICE_H__

#include <memory>
#include <utility>
#include <vector>

#include "node.h"

class Service {
    public: 
        Service() {}

        // Helper methods to send messages to all nodes or to process all the returning messages.
        void BroadcastAll();
        void RunRoundProcess();

        // Process a command!
        virtual void ProcessCommand(const std::string& command) = 0;
    protected:
        std::vector<std::shared_ptr<Node>> nodes_;
        uint64_t round_num_{0};
};

#endif //  __SERVICE_H__