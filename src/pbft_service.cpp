/**
 * @file pbft_service.cpp
 * @author Abigale Kim (abigalek)
 * @brief Implementation of PBFT service class. The PBFT service class exports one 
 * public method, ProcessCommand. 
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

/**
 * @brief This method executes ExecuteCommand for node with index i in the
 * array nodes. Count is used to indicate the number of threads that are
 * finished executing.
 */
void ProcessCommandNode(int i, std::vector<std::shared_ptr<PBFTNode>>& nodes, std::string command, std::promise<std::string>&& val, std::atomic_uint64_t& cnt) {
  nodes[i]->ExecuteCommand(nodes, command, std::move(val));
  cnt += 1;
}

/**
 * @brief Sends cv.notify_all so that nodes that don't receive messages (due to
 * a bad leader) do not hang. Ends execution when all nodes are done returning
 * results. Since we are running a simulator, we should ensure that our
 * simulator finishes running.
 */
void SendNotifyAll(uint64_t total_nodes, std::vector<std::shared_ptr<PBFTNode>>& nodes, std::atomic_uint64_t& cnt) {
  while (cnt < total_nodes) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    for (auto &node : nodes) {
      node->SendNotifyAll();
    }
  }
}

/**
 * @brief Implements ProcessCommand. This method takes in a command and then
 * runs the consensus protocol to agree on this command with the nodes in its class.
 */
void PBFTService::ProcessCommand(const std::string& command) {
  // We collect results by storing promises, which contain futures. Each promise
  // is guaranteed to be filled in by individual nodes. In particular, we
  // maintain an invariant that the promise in return_promises[i] is filled
  // by node[i], and we can extract that value after the node is finished
  // executing by accessing return_futures[i].get().
  std::vector<std::promise<std::string>> return_promises(nodes_.size());
  std::vector<std::future<std::string>> return_futures;
  std::vector<std::thread> threads;

  // We store an atomic count of how many threads have finished so that our
  // notify_all thread can finish after all threads are done executing.
  std::atomic_uint64_t acnt = 0;

  for (uint64_t i = 0; i < nodes_.size(); ++i) {
    return_futures.emplace_back(return_promises[i].get_future());
    threads.push_back(std::thread(ProcessCommandNode, i, std::ref(nodes_), command, std::move(return_promises[i]), std::ref(acnt)));
  }

  threads.emplace_back(std::thread(SendNotifyAll, nodes_.size(), std::ref(nodes_), std::ref(acnt)));

  for (auto &t : threads) {
    t.join();
  }

  // Processing the end results.
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

  // If there's a result that 2f + 1 nodes agree on, we output it.
  for (const auto& elem : count_vals) {
    if (elem.second > 2 * f_ ) {
      std::cout << elem.first << std::endl;
    }
  }
}