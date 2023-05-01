import socket
import logging
from threading import Timer, Thread
import time
import json
from Crypto.Hash import SHA256
from Crypto.Signature import DSS
from binascii import hexlify

HOST = '127.0.0.1'

def to_json(dat):
    return json.dumps(dat).encode() + b'\n'

class BroadcastNode:
    def __init__(self, i, n, delta, ports, m, priv_key, pub_keys, epoch, socks):
        self.i = i # 0 <= i < n indicates the index of the node
        self.n = n
        self.f = n // 3
        self.delta = delta
        self.ports = ports
        self.is_leader = i == 0
        self.m = m
        self.priv_key = priv_key
        self.pub_keys = pub_keys
        self.epoch = epoch
        self.I = None
        self.gathered = [None for _ in range(self.n)]

        self.phase = 0
        self.round = 0
        self.start_time = 0 # Updated when start is called
        # Networking, where
        # - ith socket is the listening socket
        # - other sockets are the client sockets
        self.socks = socks
        self.timer_thread = None
        self.listener_thread = None
        self.threads = [] # Keep track of threads sequentially
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    def log(self, m):
        elapsed = time.time() - self.start_time
        logging.info(f"[Node {self.i} at {elapsed:.5f}] {m}")

    def sign(self, m):
        h = SHA256.new(m.encode())
        signer = DSS.new(self.priv_key, 'fips-186-3')
        return hexlify(signer.sign(h)).decode()
    
    def count_non_none(self, a):
        res = 0
        for o in a:
            if o is not None:
                res += 1
        return res

    def schedule_advance(self):
        if self.round == 3:
            self.round = 0
            self.phase += 1
        else:
            self.round += 1
        self.send_messages()
        self.log(f"Advanced to phase {self.phase} round {self.round}.")
        self.timer_thread = Timer(self.delta, self.schedule_advance)
        self.timer_thread.start()

    def start(self):
        self.start_time = time.time()
        #Thread(target = self.listen).start()
        #self.connect()
        self.schedule_advance()

    def verify_message(self, v):
        if 'm' in v:
            return len(v['sigs']) <= self.n
        return True

    def on_message(self, v, ind):
        if 'm' not in v or 'sigs' not in v:
            return

        m = v['m']
        sigs = v['sigs']
        if len(sigs) == 2*self.f + 1:
            self.log(f'I from {ind}')
            self.I = (m, sigs)
        elif len(sigs) == 2 and self.round <= 2:
            self.log(f'Forward from {ind}')
            self.gathered[ind] = sigs[1]
            if self.count_non_none(self.gathered) >= 2 * self.f + 1 and self.gathered[0] is not None:
                self.log(b'DONE')
                for j in range(self.n):
                    if j != self.i:
                        sending_sigs = []
                        for s in self.gathered:
                            if s is not None:
                                sending_sigs.append(s)
                        sending_sigs = sending_sigs[:2 * self.f + 1]
                        self.socks[j].sendall(to_json({'epoch': self.epoch, 'm': m, 'sigs': sending_sigs}))
                self.shutdown(m)

        elif len(sigs) == 1 and self.round == 1:
            self.log(f'Leader from {ind}')
            if self.gathered[0] is None and ind == 0:
                self.gathered[0] = sigs[0]
                sig = self.sign(m)
                self.gathered[self.i] = sig
                for j in range(self.n):
                    if j != self.i:
                        self.socks[j].sendall(to_json({'epoch': self.epoch, 'm': m, 'sigs': [sigs[0], sig]}))
        else:
            self.log('KIND4')
            

        
    '''
    def react_to_messages(self, client, address):
        # TODO: change this so we can get arbitrary size
        size = 4096
        ind = -1
        while True:
            try:
                data = client.recv(size)
                data = json.loads(data)
                # print(data)
                if ind == -1:
                    ind = data['node']
                else:
                    self.react_phase0(data, ind)
                
            except:
                client.close()
                return False
    '''
    
    # This is called at the start of each round
    def send_messages(self):
        if self.phase == 0 and self.round == 1:
            if self.is_leader:
                sig = self.sign(self.m)
                self.gathered[0] = sig
                for j in range(self.n):
                    if j != self.i:
                        self.socks[j].sendall(to_json({'epoch': self.epoch, 'm': self.m, 'sigs': [sig]}))
                    

            


    def shutdown(self, m):
        self.log("SHUTDOWN")
        if self.i == 1:
            with open("myfile.txt", "a") as f:
                f.write(m + "\n")
        
        if self.timer_thread is not None:
            self.timer_thread.cancel()
        for thread in self.threads:
            thread.join()
        self.log("DONE")

