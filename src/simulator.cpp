#include "node.h"

#include <iostream>
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  int c;
  uint64_t faulty_nodes = 0;
  std::string tracefile;
  uint64_t byzantine_mode = 0;
  bool reliable = true;

  while ((c = getopt(argc, argv, "f:t:b:r:")) != -1) {
    switch (c) {
      case 'f': {
        faulty_nodes = static_cast<uint64_t>(atoi(optarg));
      } break;
      case 't': {
        tracefile = std::string(optarg);
      } break;
      case 'b': {
        byzantine_mode = static_cast<uint64_t>(atoi(optarg));
      } break;
      case 'r': {
        if (strlen(optarg) == 5 && strncmp("false", optarg, 5)) {
          reliable = false;
        }
      } break;
      default: {
        std::cout << "Got unknown argument " << c << std::endl;
      }
    }
  }

  std::vector<Node> nodes;
  int timestamp = 0;
  for (uint64_t i = 0; i < faulty_nodes; ++i) {
    nodes.emplace_back(true, timestamp);
    timestamp += 1;
  }

  uint64_t total_nodes = 3 * faulty_nodes + 1;
  for (uint64_t i = faulty_nodes; i < total_nodes; ++i) {
    nodes.emplace_back(false, timestamp);
    timestamp += 1;
  }

  for (const auto& node : nodes) {
    std::cout << node.ToStr() << std::endl;
  }
  
  return 0;
}