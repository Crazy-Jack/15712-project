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

    Message(int value, int sender_id) : value(value), sender_id(sender_id) {}

    static Message BOT() {
        return Message(-1, -1);
    }
};

class Node {
public:
    int id;
    int phase;
    int output_value;
    Message local_variable;
    std::vector<Node*> neighbors;

    Node(int id) : id(id), phase(0), output_value(-1), local_variable(Message::BOT()) {}

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

private:
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

void bft_consensus(std::vector<Node>& nodes) {
    int num_nodes = nodes.size();
    bool consensus_reached = false;

    nodes[0].local_variable = Message(42, 0); // The leader node initializes the message with value 42

    while (!consensus_reached) {
        // Increment phase for all nodes
        for (Node& node : nodes) {
            node.store_phase(node.phase + 1);
        }

        // Round 1
        for (Node& node : nodes) {
            node.send_message_to_all(node.local_variable);
        }

        // Round 2
        for (Node& node : nodes) {
            int non_bot_count = 0;
            for (const Message& msg : node.received_messages[node.id]) {
                if (msg.value != -1) {
                    non_bot_count++;
                }
            }
            if (non_bot_count >= 2 * f + 1) {
                node.local_variable = node.received_messages[node.id][0]; // Use the first non-bot message
                node.send_message_to_all(node.local_variable);
            } else {
                node.local_variable = Message::BOT();
            }
        }

        // Round 3
        for (Node& node : nodes) {
            int non_bot_count = 0;
            for (const Message& msg : node.received_messages[node.id]) {
                if (msg.value != -1) {
                    non_bot_count++;
                }
            }
            if (non_bot_count >= 2 * f + 1) {
                node.local_variable = node.received_messages[node.id][0]; // Use the first non-bot message
                node.output_value = node.local_variable.value;
            } else {
                node.local_variable = Message::BOT();
            }
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
