/**
 * @file pbft_node.cpp
 * @author Abigale Kim (abigalek)
 * @brief Additional PBFT-specific helper methods, along
 * with implementations of methods defined for the PBFTNode class.
 * 
 */

#include "pbft_node.h"
#include "lib.h"

#include <sstream>
#include <iterator>

ClientReq process_client_req(const std::string& command) {
  ClientReqType type;
  int num = 0;
  if (command.compare("g") == 0) {
    type = ClientReqType::PBFT_GET;
  } else {
    std::stringstream ss(command);  
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> commands(begin, end);
    type = ClientReqType::PBFT_SET;
    num = std::stoi(commands[1]);
  }
  return ClientReq(type, num);
}

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
    case PBFTMessageType::VIEWCHANGE: {
      return "VIEWCHANGE";
    }
    case PBFTMessageType::NEWVIEW: {
      return "NEWVIEW";
    }
  }

  return "";
}

PBFTMessageType StrToPBFTMessageType(const std::string& str) {
  if (str == "REQUEST") {
    return PBFTMessageType::REQUEST;
  }
  if (str == "PREPREPARE") {
    return PBFTMessageType::PREPREPARE;
  }
  if (str == "PREPARE") {
    return PBFTMessageType::PREPARE;
  }
  if (str == "COMMIT") {
    return PBFTMessageType::COMMIT;
  }
  if (str == "VIEWCHANGE") {
    return PBFTMessageType::VIEWCHANGE;
  }
  if (str == "NEWVIEW") {
    return PBFTMessageType::NEWVIEW;
  }

  throw std::runtime_error("Invalid argument: " + str);
}

void PBFTNode::SendMessage(PBFTMessage message) {
  queue_lock_.lock();
  queue_.push_back(message);
  log_.push_back(message);
  queue_lock_.unlock();

  queue_cond_var_.notify_all();
}

void PBFTNode::SendNotifyAll() {
  queue_cond_var_.notify_all();
}

PBFTMessage StrToPBFTMessage(std::string str) {
  PBFTMessage pbft_message;
  std::stringstream input_stream(str);
  while(input_stream.good()) {
    std::string substring;
    getline(input_stream, substring, ',');
    if (str_starts_with(substring, "Type: ")) {
      std::string type = substring.substr(6, substring.size() - 6);
      pbft_message.type_ = StrToPBFTMessageType(type);
    } else if (str_starts_with(substring, " Sender: ")) {
      std::string sender_str = substring.substr(9, substring.size() - 9);
      uint64_t sender = static_cast<uint64_t>(std::stoi(sender_str));
      pbft_message.sender_ = sender;
    } else if (str_starts_with(substring, " View Num: ")) {
      std::string view_num_str = substring.substr(11, substring.size() - 11);
      uint64_t view_number = static_cast<uint64_t>(std::stoi(view_num_str));
      pbft_message.view_number_ = view_number;
    } else if (str_starts_with(substring, " Sequence Number: ")) {
      std::string seq_num_str = substring.substr(18, substring.size() - 18);
      uint64_t seq_num = static_cast<uint64_t>(std::stoi(seq_num_str));
      pbft_message.sequence_number_ = seq_num;
    } else if (str_starts_with(substring, " Data: ")) {
      std::string data = substring.substr(7, substring.size() - 7);
      pbft_message.data_ = data;
      pbft_message.data_hash_ = sha256(data);
    }
  }
  return pbft_message;
}