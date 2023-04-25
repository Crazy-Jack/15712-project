#ifndef __PBFT_SERVICE_H__
#define __PBFT_SERVICE_H__

#include "service.h"
#include "pbft_nodes.h"

#include <memory>
#include <random>
#include <variant>

class PBFTService : public Service {
  public:
    PBFTService(uint64_t num_faulty_nodes, uint64_t byzantine_mode, bool reliable) {
      int timestamp = 0;
      uint64_t total_nodes = 3 * num_faulty_nodes + 1;
      for (uint64_t i = 0; i < num_faulty_nodes; ++i) {
        nodes_.emplace_back(std::make_shared<PBFTByzantineNode>(reliable, timestamp, total_nodes));
        timestamp += 1;
      }

      for (uint64_t i = num_faulty_nodes; i < total_nodes; ++i) {
        nodes_.emplace_back(std::make_shared<PBFTGoodNode>(reliable, timestamp, total_nodes));
        timestamp += 1;
      }

      // Pick leader node
      std::default_random_engine generator;
      std::uniform_int_distribution<uint64_t> distribution(0,total_nodes - 1);
      primary_node_ = distribution(generator);
    }

    void ProcessCommand(const std::string& command) override;

  private:
    uint64_t primary_node_; // index of primary node
    std::vector<std::shared_ptr<PBFTNode>> nodes_;
    /*
      Round number: 
      0 for request
      1 for pre-prepare
      2 for prepare
      3 for commit
      4 for reply
    */
};

#endif // __PBFT_SERVICE_H__