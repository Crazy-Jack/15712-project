#ifndef __BB_SERVICE_H__
#define __BB_SERVICE_H__

#include "service.h"
#include "bb_node.h"
#include "bb_good_node.h"
#include "bb_not_responsive_node.h"

#include <memory>
#include <random>
#include <variant>

class BBService : public Service {
  public:
  /**
   * @brief Create the BB service object
   * 
   * @param num_faulty_nodes f, in the protocol.
   * @param byzantine_mode 0 (no Byzantine nodes), 1 (not responsive nodes), 2 (wrong nodes)
   * @param reliable whether the good nodes in the service can die (unused)
   */
    BBService(uint64_t num_faulty_nodes, uint64_t byzantine_mode, bool reliable, uint64_t timeout) : f_(num_faulty_nodes) {
      uint64_t total_nodes = 3 * num_faulty_nodes + 1;
      // , primary_node_(primary_node_)
      uint64_t primary_node_ = 0;

      if (byzantine_mode == 0) {
        int timestamp = 0;
        for (uint64_t i = 0; i < total_nodes; ++i) {
          nodes_.emplace_back(std::make_shared<BBGoodNode>(reliable, timestamp, total_nodes, primary_node_, num_faulty_nodes, timeout));
          timestamp += 1;
        }
      } else if (byzantine_mode == 1) {
        int timestamp = 0;
        for (uint64_t i = 0; i < num_faulty_nodes; ++i) {
          nodes_.emplace_back(std::make_shared<BBNotResponsiveNode>(reliable, 0, total_nodes, primary_node_, num_faulty_nodes));
          timestamp += 1;
        }

        for (uint64_t i = num_faulty_nodes; i < total_nodes; ++i) {
          nodes_.emplace_back(std::make_shared<BBGoodNode>(reliable, timestamp, total_nodes, primary_node_, num_faulty_nodes, timeout));
          timestamp += 1;
        }
      }
    }

    void ProcessCommand(const std::string& command) override;

  private:
    uint64_t f_;
    std::vector<std::shared_ptr<BBNode>> nodes_;
};

#endif // __BB_SERVICE_H__