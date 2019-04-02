# helper only to test zmq

import zmq
import random
import sys
import time
import uuid

port = "9999"
context = zmq.Context()
socket = context.socket(zmq.PAIR)
socket.bind("tcp://*:%s" % port)

while True:
    socket.send_string(str(uuid.uuid4()))
    time.sleep(1)
