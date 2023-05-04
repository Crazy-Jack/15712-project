


#include "bb_node.h"
#include "lib.h"

#include <sstream>
#include <iterator>




ClientReq process_client_req(const std::string& command) {
  ClientReqType type;
  int num = 0;
  if (command.compare("g") == 0) {
    type = ClientReqType::BB_GET;
  } else {
    std::stringstream ss(command);  
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> commands(begin, end);
    type = ClientReqType::BB_SET;
    num = std::stoi(commands[1]);
  }
  return ClientReq(type, num);
}

std::string BBMessageTypeToStr(BBMessageType type) {
  switch(type) {
    case BBMessageType::REQUEST: {
      return "REQUEST";
    }
    case BBMessageType::PREPREPARE: {
      return "PREPREPARE";
    }
    case BBMessageType::PREPARE: {
      return "PREPARE";
    }
    case BBMessageType::OUTPUT: {
      return "OUTPUT";
    }
  }
  return "";
}

BBMessageType StrToBBMessageType(const std::string& str) {
  if (str == "REQUEST") {
    return BBMessageType::REQUEST;
  }
  if (str == "PREPREPARE") {
    return BBMessageType::PREPREPARE;
  }
  if (str == "PREPARE") {
    return BBMessageType::PREPARE;
  }
  if (str == "OUTPUT") {
    return BBMessageType::OUTPUT;
  }
  
  throw std::runtime_error("Invalid argument: " + str);
}

void BBNode::SendNotifyAll() {
  queue_cond_var_.notify_all();
}

bool str_starts_with(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
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

ClientReq process_client_req(const std::string& command) {
  ClientReqType type;
  int num = 0;
  if (command.compare("g") == 0) {
    type = ClientReqType::BB_GET;
  } else {
    std::stringstream ss(command);  
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> commands(begin, end);
    type = ClientReqType::BB_SET;
    num = std::stoi(commands[1]);
  }
  return ClientReq(type, num);
}
