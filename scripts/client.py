import socket
import json
import time
HOST = '127.0.0.1'
PORT = 15213

def to_json(dat):
    return json.dumps(dat).encode() + b'\n'

with open('../traces/trace100.txt', 'r') as f:
    ls = f.readlines()
    sock = socket.socket()
    sock.connect((HOST, PORT))
    for i in range(len(ls)):
        sock.sendall(to_json({'m': ls[i].rstrip()}))
        #time.sleep(0.1)
