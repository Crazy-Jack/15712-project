import socket
import time
import json
from threading import Thread

HOST = '127.0.0.1'
port = 15213

def react_to_messages(client, address):
    size = 2048
    queue = b''
    started = False
    start_time = None
    count = 0
    while True:
        messages = queue.split(b'\n')
        queue = messages[-1]
        for data in messages[:-1]:
            data = json.loads(data)
            m = data['m']
            with open("myfile.txt", "a") as f:
                f.write(m + "\n")
            count += 1
            if count == 100:
                print(time.time() - start_time)
        try:
            data = client.recv(size)
            if not started:
                started = True
                start_time = time.time()
            queue += data                
        except:
            client.close()
            return False

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((HOST, port))
sock.listen()
client, address = sock.accept()
client.settimeout(90)
print(f"Starting client {address}")
thread = Thread(target=react_to_messages, args=(client, address))
thread.start()