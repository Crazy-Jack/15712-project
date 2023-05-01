#!/usr/bin/env python3
import threading
import json
import logging
from random import randbytes
from Crypto.PublicKey import ECC

from osmr import OSMRNode

logging.basicConfig(level = logging.INFO)

f = 1 # n = 3f + 1
n = 3 * f + 1
delta = 1 # in s
kappa = 3 # in s
mu = 1 # delta = mu * kappa

PORTS = range(15712, 15712 + n) 

def to_json(dat):
    return json.dumps(dat).encode() + b'\n'

class Server(object):

    def listenToClient(self, client, address):
        BroadcastNode(i, n, delta).start()
        '''
        size = 1024
        carnival = Carnival(address)
        client.send(carnival.menu())
        while True:
            try:
                data = client.recv(size)
                if data:
                    res = carnival.interact(json.loads(data))
                    if 'help' in res:
                        client.send(res['help'].encode())
                    else:
                        client.send(to_json(res))
                    if 'error' in res:
                        raise Exception("There's an error")
                else:
                    raise Exception('Client disconnected')
            except:
                client.close()
                return False
        '''

def start_node(i, priv_key, pub_keys):
    # i: id of node
    # n: number of nodes
    # delta: network delay bound
    # PORTS: the ports of the nodes
    # i == 0: whether or not the node is the leader
    # b'ADB': the message to be agreed on

    #BroadcastNode(i, n, delta, PORTS, 'ADB' if i == 0 else None, priv_key, pub_keys).start()

    OSMRNode(i, n, delta, kappa, PORTS, priv_key, pub_keys).start()
    
if __name__ == '__main__':
    priv_keys = [ECC.generate(curve='P-256') for _ in range(n)]
    pub_keys = [priv_keys[i].public_key() for i in range(n)]
    for i in range(n):
        threading.Thread(target=start_node, args = (i, priv_keys[i], pub_keys)).start()

