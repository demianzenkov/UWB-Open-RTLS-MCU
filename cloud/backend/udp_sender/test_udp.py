import socket
import time

UDP_IP = "192.168.1.100"
UDP_PORT = 30001
MESSAGE = "Hello, World!!!"


print("UDP target IP:", UDP_IP)
print("UDP target port:", UDP_PORT)
print("message:", MESSAGE)
i=0
while True:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
    sock.sendto(bytes(MESSAGE, "utf-8"), (UDP_IP, UDP_PORT))
    print("udp_sent: ", i)
    i=i+1
    time.sleep(5)

