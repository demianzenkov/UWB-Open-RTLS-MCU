import sys

import asyncio
import serial_asyncio
import zmq
from zmq.asyncio import Context

sys.path.append('../dwm_api')
from dwm_api import DWM1001

dwm = DWM1001()
dwm.dwm_init()
# print(dwm.dwm_upd_rate['DWM_UPD_RATE_MAX'])

_ZMQ_PAIR_SLEEP = 0.5
_ZMQ_URL = 'tcp://127.0.0.1:9999'


context = Context.instance()
socket = context.socket(zmq.PAIR)
socket.bind(_ZMQ_URL)

reader = None
writer = None
portInitialized = False
reader_task = None


async def serial_openport(device, **kwargs):
    global reader, writer, portInitialized
    if portInitialized is False:
        try:
            portInitialized = True
            reader, writer = await serial_asyncio.open_serial_connection(url=device, **kwargs)
            loop = asyncio.get_event_loop()
            global reader_task
            reader_task = loop.create_task(serial_read())
            # loop.create_task(serial_checkconnection())
            print('Port opened!')
        except:
            print('Port cant be opened')


async def serial_closeport():
    global reader, writer, reader_task, portInitialized
    if portInitialized is True and writer.transport.serial.is_open:
        #TODO: Cancel read task!
        reader_task.cancel()
        writer.transport.serial.close()
        # reader.transport.serial.close()
        portInitialized = False
        print('Port closed!')

async def serial_checkconnection():
    while True:
        if reader_task:
            global reader, writer, reader_task
            if writer.transport.serial.closed:
                print('Serial connection lost!')
                await reader_task.cancel()
                reader_task, reader, writer = None
            await asyncio.sleep(3)


async def zmq_receiver():
    while True:
        message = await socket.recv_string()
        print("zmq_rx:", message)
        if 'serial_openport' in message:
            await serial_openport('COM18', baudrate=115200)
        elif 'serial_closeport' in message:
            await serial_closeport()
        await asyncio.sleep(_ZMQ_PAIR_SLEEP)


async def zmq_sender(message: str):
    await socket.send_string(message)


async def serial_read():
    while True:
        if reader._transport.serial.is_open:
            msg = await reader.readuntil(b'\n')
            print(f'received: {msg.rstrip().decode()}')


async def alive():
    while True:
        print('ping')
        await asyncio.sleep(3)


def main():
    loop = asyncio.get_event_loop()
    # loop.create_task(alive())
    loop.create_task(zmq_receiver())
    loop.run_forever()
    loop.close()


main()
