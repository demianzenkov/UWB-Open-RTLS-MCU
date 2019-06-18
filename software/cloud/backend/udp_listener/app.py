import socket
import time

_UDP_HOST = '127.0.0.1'
_UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((_UDP_HOST, _UDP_PORT))
sock.setblocking(False)


def listen_udp():
    reqs = 0
    t1 = time.time()
    while True:
        t2 = time.time()
        if t2 - t1 >= 1:
            print(f'Reqs/sec: {reqs}')
            t1 = t2
            reqs = 0
        data = None
        try:
            data = sock.recv(4096)
            reqs += 1
        except socket.timeout as e:
            pass
        except socket.error as e:
            pass
        else:
            pass
    sock.close()


if __name__ == '__main__':
    listen_udp()
