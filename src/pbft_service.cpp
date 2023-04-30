/**
 * @file pbft_service.cpp
 * @author your name (you@domain.com)
 * @brief pbft service
 */

#include "pbft_service.h"

#include <atomic>
#include <chrono>
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

void ProcessCommandNode(int i, std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val, std::atomic_uint64_t& cnt) {
  nodes[i]->ExecuteCommand(nodes, command, std::move(val));
  cnt += 1;
}

void SendNotifyAll(uint64_t total_nodes, std::vector<std::shared_ptr<PBFTNode>>& nodes, std::atomic_uint64_t& cnt) {
  while (cnt < total_nodes) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    for (auto &node : nodes) {
      node->SendNotifyAll();
    }
  }
}

void PBFTService::ProcessCommand(const std::string& command) {
  // Empty string = no value
  std::vector<std::promise<std::string>> return_promises(nodes_.size());
  std::vector<std::future<std::string>> return_futures;
  std::vector<std::thread> threads;
  std::atomic_uint64_t acnt = 0;

  for (uint64_t i = 0; i < nodes_.size(); ++i) {
    return_futures.emplace_back(return_promises[i].get_future());
    threads.push_back(std::thread(ProcessCommandNode, i, std::ref(nodes_), command, std::move(return_promises[i]), std::ref(acnt)));
  }

  threads.emplace_back(std::thread(SendNotifyAll, nodes_.size(), std::ref(nodes_), std::ref(acnt)));

  for (auto &t : threads) {
    t.join();
  }

  std::unordered_map<std::string, uint64_t> count_vals;
  for (auto& future: return_futures) {
    if (future.valid()) {
      std::string val = future.get();
      if (count_vals.find(val) == count_vals.end()) {
        count_vals.insert(std::make_pair(val, 1));
      } else {
        count_vals[val] += 1;
      }
    }
  }

  for (const auto& elem : count_vals) {
    if (elem.second > 2 * f_ ) {
      std::cout << elem.first << std::endl;
    }
  }
}