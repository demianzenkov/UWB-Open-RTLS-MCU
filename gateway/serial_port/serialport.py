import sys
from enum import Enum
import time
import asyncio
import serial_asyncio
import zmq
from zmq.asyncio import Context, Poller

sys.path.append('../dwm_api')
from dwm_api import DWM1001


dwm = DWM1001()
# dwm.dwm_init()
# print(dwm.dwm_upd_rate.DWM_UPD_RATE_MAX.value)

ctx = Context.instance()
zmq_url = 'tcp://127.0.0.1:5555'


async def zmq_receiver():
    """receive messages with polling"""
    pull = ctx.socket(zmq.PULL)
    pull.connect(zmq_url)
    poller = Poller()
    poller.register(pull, zmq.POLLIN)
    while True:
        events = await poller.poll()
        if pull in dict(events):
            # print("recving", events)
            msg = await pull.recv_multipart()
            await zmq_parser(msg)


async def zmq_parser(msg):
    print('zmq_rx: ', msg)


async def zmq_tx(push, msg):
    print("zmq_tx: ", msg)
    await push.send_multipart(msg)


async def zmq_sender():
    """send a message every second"""
    tic = time.time()
    push = ctx.socket(zmq.PUSH)
    push.bind(zmq_url)
    while True:
        await zmq_tx(push, [str(time.time() - tic).encode('ascii')])
        await asyncio.sleep(1)


class SerialFactory(asyncio.Protocol):
    def __init__(self):
        self.buf = bytes()
        self.msgs_recvd = 0
        self.transport = 0

    def connection_made(self, transport):
        self.transport = transport
        if self.transport.serial.isOpen():
            print('Reader connection created')

    def data_received(self, data):
        self.buf += data
        print('uart_rx: ', data)
        # if b'\n' in self.buf:
        #     lines = self.buf.split(b'\n')
        #     self.buf = lines[-1]  # whatever was left over
        #     for line in lines[:-1]:
        #         print(f'Reader received: {line.decode()}')
        #         self.msgs_recvd += 1
        # if self.msgs_recvd == 5:
        #     self.send(b'5 strings accepted')

    def connection_lost(self, exc):
        print('Reader closed')

    def send(self, message):
        # for b in message:
            # self.transport.serial.write(bytes([b]))
        self.transport.serial.write(message)
        print('serial_tx: ', message)


async def zmq_routine():
    await asyncio.wait([zmq_receiver(), zmq_sender()])


def main():
    loop = asyncio.get_event_loop()
    serial_routine = serial_asyncio.create_serial_connection(loop, SerialFactory, 'reader', baudrate=115200)
    # reader, writer = await serial_asyncio.open_serial_connection(url='COM18', baudrate=115200)

    loop.create_task(zmq_routine())
    loop.create_task(serial_routine)
    loop.run_forever()
    # loop.run_until_complete(main())
    loop.close()


main()
