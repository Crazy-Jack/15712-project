#include "node.h"
#include "service.h"
#include "non_replicated_service.hpp"
#include "pbft_service.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>

/*
Mode: 
0: non replicated service
1: service using PBFT
2: service using our algorithm
*/

int main(int argc, char** argv) {
  int c;
  uint64_t faulty_nodes = 0;
  std::string tracefile;
  uint64_t byzantine_mode = 0;
  bool reliable = true;
  uint64_t mode = 0;
  std::shared_ptr<Service> service;

  while ((c = getopt(argc, argv, "f:t:b:r:m:")) != -1) {
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
      case 'm': {
        mode = static_cast<uint64_t>(atoi(optarg));
      } break;
      default: {
        std::cout << "Got unknown argument " << c << std::endl;
      }
    }
  }

  if (mode == 0) {
    service = std::make_shared<NonReplicatedService>();
  } else if (mode == 1) {
    service = std::make_shared<PBFTService>(faulty_nodes, byzantine_mode, true);
  }

  // Parse the tracefile. 
  std::string command;
  std::ifstream file;
  file.open(tracefile, std::ios::in);
  while (std::getline(file, command)) {
    service->ProcessCommand(command);
  }

  return 0;
}