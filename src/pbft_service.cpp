/**
 * @file pbft_service.cpp
 * @author your name (you@domain.com)
 * @brief pbft service
 */

#include "pbft_service.h"

#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

void ProcessCommandNode(int i, std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val) {
  nodes[i]->ExecuteCommand(nodes, command, std::move(val));
}

void PBFTService::ProcessCommand(const std::string& command) {
  // Empty string = no value
  std::vector<std::promise<std::string>> return_promises(nodes_.size());
  std::vector<std::future<std::string>> return_futures;
  std::vector<std::thread> threads;

  for (uint64_t i = 0; i < nodes_.size(); ++i) {
    return_futures.emplace_back(return_promises[i].get_future());
    threads.push_back(std::thread(ProcessCommandNode, i, std::ref(nodes_), command, std::move(return_promises[i])));
  }

  for (auto &t : threads) {
    t.join();
  }

  std::unordered_map<std::string, uint64_t> count_vals;
  for (auto& future: return_futures) {
    std::string val = future.get();
    if (count_vals.find(val) == count_vals.end()) {
      count_vals.insert(std::make_pair(val, 1));
    } else {
      count_vals[val] += 1;
    }
  }

  for (const auto& elem : count_vals) {
    if (elem.second > 2 * f_ ) {
      std::cout << elem.first << std::endl;
    }
  }
}