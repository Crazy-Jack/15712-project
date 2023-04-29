#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <functional>

class Message {
    // Your message class implementation
};

class Signature {
    // Your signature class implementation
};

class Node {
    // Your node class implementation
};

int f; // number of faulty nodes
std::vector<Node> P; // set of nodes

std::map<int, std::vector<Message>> received_messages;

bool has_2f_plus_1_values(const std::vector<Message>& messages, bool non_bot) {
    int count = 0;
    for (const Message& msg : messages) {
        if ((msg.value != Message::BOT) == non_bot) {
            ++count;
        }
        if (count >= 2 * f + 1) {
            return true;
        }
    }
    return false;
}

Message arbitrary_non_bot_value(const std::vector<Message>& messages) {
    for (const Message& msg : messages) {
        if (msg.value != Message::BOT) {
            return msg;
        }
    }
    return Message::BOT;
}

void bft_consensus() {
    // Assuming that leader ω is at index 0 in the P vector
    Node leader = P[0];

    // Phase 0
    leader.send_message_to_all(Message::M);

    for (int phase = 0; true; ++phase) {
        for (Node& alpha : P) {
            alpha.store_phase(phase);
            std::vector<Message> messages = received_messages[alpha.id];

            // Round 1
            alpha.send_message_to_all(alpha.local_variable);

            if (has_2f_plus_1_values(messages, false)) {
                alpha.local_variable = Message::BOT;
            } else if (has_2f_plus_1_values(messages, true)) {
                alpha.local_variable = arbitrary_non_bot_value(messages);
            } else {
                alpha.local_variable = Message::BOT;
            }

            // Round 2
            alpha.send_message_to_all(alpha.local_variable);

            if (has_2f_plus_1_values(messages, false)) {
                alpha.local_variable = Message::BOT;
            } else if (has_2f_plus_1_values(messages, true)) {
                alpha.local_variable = arbitrary_non_bot_value(messages);
                alpha.send_message_to_all(alpha.local_variable);
                alpha.output(alpha.local_variable.value);
            } else {
                alpha.local_variable = arbitrary_non_bot_value(messages);
            }

            // Round 3
            Message message = Message(alpha.local_variable, Signature(alpha.id, phase));
            alpha.send_message_to_all(message);

            if (has_2f_plus_1_values(messages, false)) {
                alpha.local_variable = Message::BOT;
            } else if (has_2f_plus_1_values(messages, true)) {
                alpha.local_variable = arbitrary_non_bot_value(messages);
            } else {
                // Assuming lsb and H are implemented as utility functions
                int b = lsb(min_alpha_H(phase));
                if (b == 0) {
                    alpha.local_variable = Message::BOT;
                } else {
                    alpha.local_variable = arbitrary_non_bot_value(messages);
                }
            }
        }
    }
}