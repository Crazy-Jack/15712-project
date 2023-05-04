/**
 * @file basic_good_service.h
 * @author Abigale Kim (abigalek)
 * @brief Basic good service implementation. Not used in the simulator. Here as an example
 * on how to implement a basic service.
 */

#ifndef __BASIC_GOOD_SERVICE_H__
#define __BASIC_GOOD_SERVICE_H__

#include "service.h"
#include "basic_good_node.h"

#include <memory>

class BasicGoodService : public Service {
  public:
    BasicGoodService(uint64_t num_faulty_nodes, uint64_t byzantine_mode, bool reliable) {
      int timestamp = 0;
      uint64_t total_nodes = 3 * num_faulty_nodes + 1;
      for (uint64_t i = 0; i < total_nodes; ++i) {
        nodes_.emplace_back(std::make_shared<BasicGoodNode>(false, timestamp, total_nodes));
        timestamp += 1;
      }
    }

    void ProcessCommand(const std::string& command) override;

  private:
    std::vector<std::shared_ptr<BasicGoodNode>> nodes_;
};

#endif // __BASIC_GOOD_SERVICE_H__