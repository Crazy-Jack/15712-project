import json
from Crypto.Hash import SHA256
from Crypto.Signature import DSS
from Crypto.Cipher import AES
from binascii import hexlify

def sign(priv_key, m):
    h = SHA256.new(m.encode())
    signer = DSS.new(priv_key, 'fips-186-3')
    return hexlify(signer.sign(h)).decode()

def to_json(dat):
    return json.dumps(dat).encode() + b'\n'

def get_leader(epoch, n, s):
    cipher = AES.new(s, AES.MODE_ECB)
    c = cipher.encrypt(epoch.to_bytes(16, 'big'))
    return int.from_bytes(c, 'big') % n