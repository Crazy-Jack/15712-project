#!/usr/bin/env python3
import threading
import logging
import argparse
from random import randbytes
from Crypto.PublicKey import ECC

from osmr import OSMRNode

logging.basicConfig(level = logging.INFO)

def start_node(i, priv_key, pub_keys):
    # i: id of node
    # n: number of nodes
    # delta: network delay bound
    # PORTS: the ports of the nodes
    # i == 0: whether or not the node is the leader

    listen_ports = PORTS[i]
    connect_ports = [PORTS[j][i] for j in range(n)]
    
    OSMRNode(i, n, delta, kappa, listen_ports, connect_ports, priv_key, pub_keys).start()
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Spawn nodes for the protocol')
    parser.add_argument('-f', metavar='FAULTY', type=int, default=1,
        help='number of faulty nodes')
    parser.add_argument('-d', '--delta', type=float, default=1,
        help='network delay bound in seconds')
    parser.add_argument('-k', '--kappa', type=float, default=2,
        help='epoch time in seconds')
    args = parser.parse_args()

    # Parameters for the protocol
    f = args.f
    n = 3 * f + 1
    delta = args.delta
    kappa = args.kappa
    mu = delta / kappa
    ############################
    # PORTS matrix (n x n):
    # - Suppose position (i,j) contains port p [0-index].
    # - This means node i listens to port p and node j connects to port p.
    ############################
    PORTS = [range(15712 + n * i, 15712 + n * (i + 1)) for i in range(n)]

    priv_keys = [ECC.generate(curve='P-256') for _ in range(n)]
    pub_keys = [priv_keys[i].public_key() for i in range(n)]

    for i in range(n):
        threading.Thread(target=start_node, args = (i, priv_keys[i], pub_keys)).start()

