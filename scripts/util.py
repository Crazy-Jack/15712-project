import json
from Crypto.Cipher import AES

def to_json(dat):
    return json.dumps(dat).encode() + b'\n'

def get_leader(epoch, n, s):
    cipher = AES.new(s, AES.MODE_ECB)
    c = cipher.encrypt(epoch.to_bytes(16, 'big'))
    return int.from_bytes(c, 'big') % n