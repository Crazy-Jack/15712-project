import socket
import logging
from threading import Timer, Thread
import time
import json
from Crypto.Hash import SHA256
from Crypto.Signature import DSS
from binascii import hexlify
from bb import BroadcastNode

HOST = '127.0.0.1'

def to_json(dat):
    return json.dumps(dat).encode() + b'\n'

class OSMRNode:
    def __init__(self, i, n, delta, kappa, ports, priv_key, pub_keys):
        self.i = i # 0 <= i < n indicates the index of the node
        self.n = n
        self.f = n // 3
        self.delta = delta
        self.kappa = kappa
        self.ports = ports
        self.is_leader = i == 0
        self.priv_key = priv_key
        self.pub_keys = pub_keys
        self.queue = []
        self.pointer = 0
        self.threads = []
        self.agreed_vals = []

        self.bbs = []
        self.epoch = -1
        self.start_time = 0 # Updated when start is called

        # socks
        # Networking, where
        # - ith socket is the None
        # - other sockets are sockets for other nodes
        self.socks = [None for _ in range(self.n)]
        self.client_sock = None
        self.listen_sock = None

    def log(self, m):
        elapsed = time.time() - self.start_time
        logging.info(f"[Node {self.i} at {elapsed:.5f}] {m}")

    def schedule_advance(self):
        self.epoch += 1
        if self.pointer < len(self.queue):
            print("-------------------\n HAHA\n ------------")
            m = self.queue[self.pointer]
            self.pointer += 1
        else:
            m = "bot"
        self.bbs.append(BroadcastNode(self.i, self.n, self.delta, self.ports, m, self.priv_key, self.pub_keys, self.epoch, self.socks))
        self.bbs[self.epoch].start()
        self.log(f"Advanced to epoch {self.epoch}.")
        self.timer_thread = Timer(self.kappa, self.schedule_advance)
        self.timer_thread.start()

    def start(self):
        print('1')
        self.start_time = time.time()
        if self.i == 0:
            Thread(target=self.listen_client).start()
        print('2')
        Thread(target=self.listen).start()
        print('3')
        self.connect()
        print('4')
        self.schedule_advance()

    def listen(self):
        self.listen_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock = self.listen_sock
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((HOST, self.ports[self.i]))
        sock.listen()
        while True:
            client, address = sock.accept()
            client.settimeout(90)
            self.log(f"Starting client {address}")
            thread = Thread(target = self.react_to_messages, args = (client, address))
            self.threads.append(thread)
            thread.start()

    def listen_client(self):
        self.client_sock  = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock = self.client_sock
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((HOST, 15213))
        sock.listen()
        while True:
            client, address = sock.accept()
            client.settimeout(90)
            self.log(f"Starting actual client {address}")
            thread = Thread(target = self.react_to_requests, args = (client, address))
            self.threads.append(thread)
            thread.start()

    def react_to_requests(self, client, address):
        size = 2048
        while True:
            try:
                data = client.recv(size)
                data = json.loads(data)
                self.queue.append(data['m'])
                
                '''
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
                '''
            except:
                client.close()
                return False

    # Connect to the other guys
    def connect(self):
        for j in range(self.n):
            if j != self.i:
                port = self.ports[j]
                while True:
                    try:
                        self.socks[j] = socket.socket()
                        self.socks[j].connect((HOST, port))
                        self.socks[j].sendall(to_json({'epoch': self.epoch, 'node': self.i, 'sigs': None}))
                        break
                    except Exception as e:
                        pass
    
    def react_to_messages(self, client, address):
        # TODO: change this so we can get arbitrary size
        size = 2048
        ind = -1
        while True:
            try:
                data = client.recv(size)
                data = json.loads(data)
                # print(data)
                if ind == -1:
                    ind = data['node']
                else:
                    epoch = data['epoch']
                    self.bbs[epoch].on_message(data, ind)
                
                '''
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
                '''
            except:
                client.close()
                return False

        
    def shutdown(self):
        self.log("SHUTDOWN")
        if self.timer_thread is not None:
            self.timer_thread.cancel()
        for j in range(self.n):
            self.socks[j].close()
        for thread in self.threads:
            thread.join()
        self.log("DONE")

