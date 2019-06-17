import socket

_UDP_HOST = '127.0.0.1'
_UDP_PORT = 5005

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((_UDP_HOST, _UDP_PORT))

while True:
    data = sock.recv(1024)
    if not data:
        break
    print(data)

sock.close()