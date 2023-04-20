#include "service.h"

void Service::BroadcastAll() {
    // Generate and send all the messages per round
    std::vector<Message> messages_to_send; 
    for (const auto &node :nodes_) {
        messages_to_send.emplace_back(node->GenerateMessageAll(round_num_));
    }

    for (uint64_t i = 0; i < nodes_.size(); ++i) {
        for (uint64_t j = 0; j < nodes_.size(); ++j) {
            if (i == j) continue;

            // Sending message from i to j.
            nodes_[j]->ReceiveMessage(std::move(messages_to_send[i]));
        }
    }
}
void Service::RunRoundProcess() {
    for (const auto& node: nodes_) {
        node->ProcessMessages(round_num_);
    }
}