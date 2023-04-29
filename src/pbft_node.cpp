#include "pbft_node.h"

std::string PBFTMessageTypeToStr(PBFTMessageType type) {
  switch(type) {
    case PBFTMessageType::REQUEST: {
      return "REQUEST";
    }
    case PBFTMessageType::PREPREPARE: {
      return "PREPREPARE";
    }
    case PBFTMessageType::PREPARE: {
      return "PREPARE";
    }
    case PBFTMessageType::COMMIT: {
      return "COMMIT";
    }
  }

  return "";
}

void PBFTNode::SendMessage(PBFTMessage message) {
  queue_lock_.lock();
  queue_.push_back(message);
  log_.push_back(message);
  queue_lock_.unlock();

  queue_cond_var_.notify_all();
}