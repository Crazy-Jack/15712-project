import socket
import logging
from threading import Timer, Thread
import time
import json
from Crypto.Hash import SHA256
from Crypto.Signature import DSS
from binascii import hexlify

from bb import BroadcastNode
from util import to_json

HOST = '127.0.0.1'


class OSMRNode:
    def __init__(self, i, n, delta, kappa, listen_ports, connect_ports, priv_key, pub_keys):
        self.i = i # 0 <= i < n indicates the index of the node
        self.n = n
        self.f = n // 3
        self.delta = delta
        self.kappa = kappa
        self.listen_ports = listen_ports
        self.connect_ports = connect_ports
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
        self.listen_threads = [None for _ in range(self.n)]
        self.listen_socks = [None for _ in range(self.n)]
        self.connect_socks = [None for _ in range(self.n)]
        self.client_sock = None
        self.listen_sock = None

    def log(self, m):
        elapsed = time.time() - self.start_time
        logging.info(f"[Node {self.i} at {elapsed:.5f}] {m}")

    def schedule_advance(self):
        self.epoch += 1
        if self.pointer < len(self.queue):
            m = self.queue[self.pointer]
            self.pointer += 1
        else:
            m = "bot"
        self.bbs.append(BroadcastNode(self.i, self.n, self.delta, self.connect_socks, m, self.priv_key, self.pub_keys, self.epoch))
        self.bbs[self.epoch].start()
        self.log(f"Advanced to epoch {self.epoch}.")
        self.timer_thread = Timer(self.kappa, self.schedule_advance)
        self.timer_thread.start()

    def start(self):
        self.start_time = time.time()
        if self.i == 0:
            Thread(target=self.listen_client).start()
        self.listen()
        self.connect()
        self.schedule_advance()

    def listen1(self, j):
        port = self.listen_ports[j]
        self.listen_socks[j] = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock = self.listen_socks[j]
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind((HOST, port))
        sock.listen()
        while True:
            client, address = sock.accept()
            client.settimeout(90)
            self.log(f"Starting client {address}")
            thread = Thread(target=self.react_to_messages, args=(client, j))
            self.threads.append(thread)
            thread.start()

    def listen(self):
        for j in range(self.n):
            if j != self.i:
                thread = Thread(target=self.listen1, args=(j,))
                self.listen_threads[j] = thread
                thread.start()

    def react_to_messages(self, client, j):
        # TODO: change this so we can get arbitrary size
        size = 2048
        queue = b''
        while True:
            messages = queue.split(b'\n')
            queue = messages[-1]
            for data in messages[:-1]:
                data = json.loads(data)
                epoch = data['epoch']
                self.bbs[epoch].on_message(data, j)

            try:
                data = client.recv(size)
                queue += data                
            except:
                client.close()
                return False

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
            except:
                client.close()
                return False

    # Connect to the other guys
    def connect(self):
        for j in range(self.n):
            if j != self.i:
                port = self.connect_ports[j]
                while True:
                    try:
                        self.connect_socks[j] = socket.socket()
                        self.connect_socks[j].connect((HOST, port))
                        break
                    except Exception as e:
                        pass
    
        
    def shutdown(self):
        self.log("SHUTDOWN")
        if self.timer_thread is not None:
            self.timer_thread.cancel()
        for thread in self.threads:
            thread.join()
        self.log("DONE")

