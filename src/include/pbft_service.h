#ifndef __PBFT_SERVICE_H__
#define __PBFT_SERVICE_H__

#include "service.h"
#include "pbft_node.h"
#include "pbft_good_node.h"
#include "pbft_wrong_node.h"
#include "pbft_not_responsive_node.h"

#include <memory>
#include <random>
#include <variant>

class PBFTService : public Service {
  public:
  /**
   * @brief Create the PBFT service object
   * 
   * @param num_faulty_nodes f, in the protocol.
   * @param byzantine_mode 0 (no Byzantine nodes), 1 (not responsive nodes), 2 (wrong nodes)
   * @param reliable whether the good nodes in the service can die (unused)
   */
    PBFTService(uint64_t num_faulty_nodes, uint64_t byzantine_mode, bool reliable) : f_(num_faulty_nodes) {
      uint64_t total_nodes = 3 * num_faulty_nodes + 1;
      uint64_t primary_node = 0;
      
      if (byzantine_mode == 0) {
        int timestamp = 0;
        for (uint64_t i = 0; i < total_nodes; ++i) {
          nodes_.emplace_back(std::make_shared<PBFTGoodNode>(reliable, timestamp, total_nodes, primary_node, num_faulty_nodes));
          timestamp += 1;
        }
      } else if (byzantine_mode == 1) {
        int timestamp = 0;
        for (uint64_t i = 0; i < num_faulty_nodes; ++i) {
          nodes_.emplace_back(std::make_shared<PBFTNotResponsiveNode>(reliable, 0, total_nodes, primary_node, num_faulty_nodes));
          timestamp += 1;
        }

        for (uint64_t i = num_faulty_nodes; i < total_nodes; ++i) {
          nodes_.emplace_back(std::make_shared<PBFTGoodNode>(reliable, timestamp, total_nodes, primary_node, num_faulty_nodes));
          timestamp += 1;
        }
      } else if (byzantine_mode == 2) {
        int timestamp = 0;
        for (uint64_t i = 0; i < num_faulty_nodes; ++i) {
          nodes_.emplace_back(std::make_shared<PBFTWrongNode>(reliable, 0, total_nodes, primary_node, num_faulty_nodes));
          timestamp += 1;
        }

        for (uint64_t i = num_faulty_nodes; i < total_nodes; ++i) {
          nodes_.emplace_back(std::make_shared<PBFTGoodNode>(reliable, timestamp, total_nodes, primary_node, num_faulty_nodes));
          timestamp += 1;
        }
      }
    }

    void ProcessCommand(const std::string& command) override;

  private:
    uint64_t f_;
    std::vector<std::shared_ptr<PBFTNode>> nodes_;
};

#endif // __PBFT_SERVICE_H__