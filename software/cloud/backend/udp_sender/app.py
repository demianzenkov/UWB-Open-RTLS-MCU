import socket
import json
from random import randint
from time import sleep
import time

UDP_PORT = 5005
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.connect(('127.0.0.1', UDP_PORT))

reqs = 0
t1 = time.time()
while True:
    t2 = time.time()
    if t2 - t1 >= 1:
        print(f'Reqs/sec: {reqs}')
        t1 = t2
        reqs = 0
    sock.send('1'.encode())
    reqs += 1
