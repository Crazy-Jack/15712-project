/**
 * @file service.h
 * @author Abigale Kim
 * @brief Service virtual class that exposes the ProcessCommand method.
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

        // Process a command!
        virtual void ProcessCommand(const std::string& command) = 0;
    protected:
        uint64_t round_num_{0};
};

#endif //  __SERVICE_H__