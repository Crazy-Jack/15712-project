#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <functional>
#include <random>

const int f = 2; // number of faulty nodes
const int NUM_NODES = 2 * f + 1;


class Message {
public:
    int value;
    int sender_id;
    bool abort;


    Message(int value, int sender_id, bool abort) : value(value), sender_id(sender_id), abort(abort) {}

    static Message BOT() {
        return Message(-1, -1, false);
    }

    static Message BOT_Abort() {
        return Message(-1, -1, true);
    }


};

class Node {
public:
    int id;
    int phase;
    int output_value;
    bool abort;
    Message local_variable;
    std::vector<Node*> neighbors;

    Node(int id) : id(id), phase(0), output_value(-1), abort(false), local_variable(Message::BOT()) {}

    void store_phase(int new_phase) {
        phase = new_phase;
    }

    void send_message_to_all(const Message& message) {
        for (Node* neighbor : neighbors) {
            if (neighbor->id != id) {
                neighbor->receive_message(message);
            }
        }
    }

    void receive_message(const Message& message) {
        received_messages[message.sender_id].push_back(message);
    }

    void add_neighbor(Node* node) {
        neighbors.push_back(node);
    }

    bool has_output() const {
        return output_value != -1;
    }


    std::map<int, std::vector<Message>> received_messages;
};

std::vector<Node> create_nodes() {
    std::vector<Node> nodes;
    for (int i = 0; i < NUM_NODES; ++i) {
        nodes.emplace_back(i);
    }
    for (int i = 0; i < NUM_NODES; ++i) {
        for (int j = 0; j < NUM_NODES; ++j) {
            if (i != j) {
                nodes[i].add_neighbor(&nodes[j]);
            }
        }
    }
    return nodes;
}


bool round_1(Node& alpha, int f) {
    alpha.send_message_to_all(alpha.local_variable);

    int bot_count = 0;
    int non_bot_count = 0;

    for (const Message& msg : alpha.received_messages[alpha.id]) {
        if (msg.value == -1) {
            bot_count++;
        } else {
            non_bot_count++;
        }
    }

    if (bot_count >= 2 * f + 1) {
        // TODO: output Abort 
        alpha.local_variable = Message::BOT();
        alpha.send_message_to_all(alpha.local_variable);
        alpha.output_value = -1; // Output ⊥
        return false;
    } else if (non_bot_count >= 2 * f + 1) {
        // Find and set the first non-bot message
        for (const Message& msg : alpha.received_messages[alpha.id]) {
            if (msg.value != -1) {
                alpha.local_variable = msg;
                break;
            }
        }
        return true;
    } else {
        alpha.local_variable = Message::BOT();
        return true;
    }
}


void bft_consensus(std::vector<Node>& nodes) {
    int num_nodes = nodes.size();
    bool consensus_reached = false;

    nodes[0].local_variable = Message(42, 0); // The leader node initializes the message with value 42

    while (!consensus_reached) {
        // Increment phase for all nodes
        for (Node& node : nodes) {
            node.store_phase(node.phase + 1);
        }
        std::cout << "Phase: " << nodes[0].phase << std::endl;

        // Round 1
        std::cout << "Round 1" << std::endl;
        for (Node& node : nodes) {
            round_1(node, f);
            
        }
        

        // Print the local variables of each node after Round 1
        for (const Node& node : nodes) {
            std::cout << "Node " << node.id << " local variable: " << node.local_variable.value << std::endl;
        }


        // Round 2
        std::cout << "Round 2" << std::endl;
        for (Node& node : nodes) {
            node.send_message_to_all(node.local_variable);

            int non_bot_count = 0;
            for (const Message& msg : node.received_messages[node.id]) {
                if (msg.value != -1) {
                    non_bot_count++;
                }
            }

            if (non_bot_count >= 2 * f + 1) {
                // Find and set the first non-bot message
                for (const Message& msg : node.received_messages[node.id]) {
                    if (msg.value != -1) {
                        node.local_variable = msg;
                        break;
                    }
                }
                node.send_message_to_all(node.local_variable);
                node.output_value = node.local_variable.value;
            } else {
                node.local_variable = Message::BOT();
            }
        }

        // Print the local variables of each node after Round 2
        for (const Node& node : nodes) {
            std::cout << "Node " << node.id << " local variable: " << node.local_variable.value << std::endl;
        }

        // Round 3
        std::cout << "Round 3" << std::endl;
        for (Node& node : nodes) {
            node.send_message_to_all(node.local_variable);

            int non_bot_count = 0;
            for (const Message& msg : node.received_messages[node.id]) {
                if (msg.value != -1) {
                    non_bot_count++;
                }
            }

            if (non_bot_count >= 2 * f + 1) {
                // Find and set the first non-bot message
                for (const Message& msg : node.received_messages[node.id]) {
                    if (msg.value != -1) {
                        node.local_variable = msg;
                        break;
                    }
                }
            } else {
                node.local_variable = Message::BOT();
            }
        }

        // Print the local variables of each node after Round 3
        for (const Node& node : nodes) {
            std::cout << "Node " << node.id << " local variable: " << node.local_variable.value << std::endl;
        }


        // Check if all nodes have reached a consensus
        consensus_reached = true;
        for (const Node& node : nodes) {
            if (!node.has_output()) {
                consensus_reached = false;
                break;
            }
        }
    }
}



int main() {
    std::vector<Node> nodes = create_nodes();

    // Run the consensus algorithm
    bft_consensus(nodes);

    // Print the output values of each node
    for (const Node& node : nodes) {
        std::cout << "Node " << node.id << " output: " << node.output_value << std::endl;
    }

    return 0;
}
