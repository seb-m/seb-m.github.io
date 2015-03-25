#!/usr/bin/env python
import base64
import hashlib
import urllib
import struct
import sys
# Local import
import md5

def md5_b64encode(data):
    return base64.urlsafe_b64encode(hashlib.md5(data).digest()).strip('=')

def md5_b64decode(b64hash):
    return base64.urlsafe_b64decode(b64hash + '=' * (len(b64hash) % 4))

def md5_padding(data_len):
    data_len = data_len << 3
    index = (data_len >> 3) & 0x3fL
    if index < 56:
        pad_len = 56 - index
    else:
        pad_len = 120 - index
    padding = '\x80' + '\x00' * 63
    bits = ''.join([chr((data_len >> (i * 8)) & 0xff) for i in range(8)])
    return padding[:pad_len] + bits

def md5_set_state(md5obj, state, data_len):
    A, B, C, D = struct.unpack("<IIII", state)
    md5obj.A = A
    md5obj.B = B
    md5obj.C = C
    md5obj.D = D
    md5obj.count[0] = data_len << 3

def md5_extend(host, uri, base_hash, expire):
    new_expire = '1658275195'  # 2022
    base_new_uri = uri + expire
    base_new_uri_len = len(base_new_uri)
    hash_raw = md5_b64decode(base_hash)
    for i in range(1, 32):
        new_uri = base_new_uri + md5_padding(i + base_new_uri_len)
        req = host + urllib.quote(new_uri)
        extension = md5.MD5Type()
        md5_set_state(extension, hash_raw, (i >> 6) + 64)
        extension.update(new_expire)
        req += '?st=' + base64.urlsafe_b64encode(extension.digest()).strip('=')
        req += '&e=' + new_expire
        # print(req)
        urlobj = urllib.urlopen(req)
        if urlobj.getcode() == 200:
            return urlobj.read()

if __name__ == '__main__':
    generate = False
    host = 'http://cc.dbzteam.org:9000'
    uri = '/p/eve/restricted.txt'
    if generate:
        # Branch used to generate the challenge
        key = 'rr43nn-gMTS_paYH'
        expire = '1295613171'
        base_hash = md5_b64encode(key + uri + expire)
        print('base_hash: %s' % base_hash)
    else:
        # Branch used to break the challenge
        base_hash = sys.argv[1]
        expire = sys.argv[2]
    ret = md5_extend(host, uri, base_hash, expire)
    print(ret)
