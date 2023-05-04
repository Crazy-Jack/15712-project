/**
 * @file bb_node.cpp
 * @author Tianqin Li (tianqinl) and Abigale Kim (abigalek)
 * @brief TODO: fill in
 */
#include "lib.h"
#include "bb_node.h"

#include <sstream>
#include <iterator>

std::string BBMessageTypeToStr(BBMessageType type) {
  switch(type) {
    case BBMessageType::BB_REQUEST: {
      return "BB_REQUEST";
    }
    case BBMessageType::BB_PREPREPARE: {
      return "BB_PREPREPARE";
    }
    case BBMessageType::BB_PREPARE: {
      return "PREPARE";
    }
    case BBMessageType::BB_OUTPUT: {
      return "OUTPUT";
    }
  }
  return "";
}

BBMessageType StrToBBMessageType(const std::string& str) {
  if (str == "BB_REQUEST") {
    return BBMessageType::BB_REQUEST;
  }
  if (str == "BB_PREPREPARE") {
    return BBMessageType::BB_PREPREPARE;
  }
  if (str == "PREPARE") {
    return BBMessageType::BB_PREPARE;
  }
  if (str == "OUTPUT") {
    return BBMessageType::BB_OUTPUT;
  }
  
  throw std::runtime_error("Invalid argument: " + str);
}

void BBNode::SendNotifyAll() {
  queue_cond_var_.notify_all();
}

BBMessage StrToBBMessage(std::string str) {
  BBMessage bb_message;
  std::stringstream input_stream(str);
  while(input_stream.good()) {
    std::string substring;
    getline(input_stream, substring, ',');
    if (str_starts_with(substring, "Type: ")) {
      std::string type = substring.substr(6, substring.size() - 6);
      bb_message.type_ = StrToBBMessageType(type);
    } else if (str_starts_with(substring, " Sender: ")) {
      std::string sender_str = substring.substr(9, substring.size() - 9);
      uint64_t sender = static_cast<uint64_t>(std::stoi(sender_str));
      bb_message.sender_ = sender;
    } else if (str_starts_with(substring, " Data: ")) {
      std::string data = substring.substr(7, substring.size() - 7);
      bb_message.data_ = data;
      bb_message.data_hash_ = sha256(data);
    }
  }
  return bb_message;
}


void BBNode::SendMessage(BBMessage message) {
  queue_lock_.lock();
  queue_.push_back(message);
  log_.push_back(message);
  queue_lock_.unlock();

  queue_cond_var_.notify_all();
}

BBClientReq bb_process_client_req(const std::string& command) {
  BBClientReqType type;
  int num = 0;
  if (command.compare("g") == 0) {
    type = BBClientReqType::BB_GET;
  } else {
    std::stringstream ss(command);  
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> commands(begin, end);
    type = BBClientReqType::BB_SET;
    num = std::stoi(commands[1]);
  }
  return BBClientReq(type, num);
}
