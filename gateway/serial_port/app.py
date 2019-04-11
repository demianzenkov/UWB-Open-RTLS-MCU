import zmq
import asyncio
import logging
import json
import serial_asyncio
from zmq.asyncio import Context
from queue import Queue

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch_format = logging.Formatter('%(asctime)s : [%(name)s] : [%(levelname)s] : %(message)s')
ch.setFormatter(ch_format)

fh = logging.FileHandler(f'./{__name__}.log')
fh_format = logging.Formatter('%(asctime)s : [%(name)s] : [%(levelname)s] : %(message)s')
fh.setFormatter(fh_format)

logger.addHandler(ch)
logger.addHandler(fh)
logger.setLevel(logging.DEBUG)

_ZMQ_PAIR_SLEEP = 0.01
_ZMQ_URL = 'tcp://127.0.0.1:9999'
_SERIAL_PORT_ADDRESS = '/dev/tty.usbserial-A9A51FF3'
_BAUD_RATE = 115200

context = Context.instance()
socket = context.socket(zmq.PAIR)
socket.bind(_ZMQ_URL)

to_socket_server_queue = Queue()

async def zmq_receiver(loop):
    transport, protocol = None, None
    while True:
        message = await socket.recv_string()
        try:
            data = json.loads(message)
        except:
            logger.warning(f'Corrupted JSON: {message}')
            continue
        logger.info(f'Received command: {data}')

        command = data.get('command')
        payload = data.get('payload')

        if command not in ['open', 'close', 'send']:
            continue

        if transport is None and command != 'open':
            logger.warning('Closed serial port can only receive open command')
            continue
        
        if command == 'open' and transport is not None:
            logger.info('Omitting open command. Serial port is already opened')
            continue
        
        if command == 'open' and (transport is None or transport.is_closing()):
            logger.info('Opening serial port connection')
            try:
                transport, protocol = await loop.create_task(serial_asyncio.create_serial_connection(loop, Output, _SERIAL_PORT_ADDRESS, baudrate=_BAUD_RATE))
            except Exception as e:
                logger.exception(e)
            continue
        
        if command == 'close' and (transport is None or transport.is_closing()):
            logger.info('Serial port is already closed')
            continue
        
        if command == 'close' and not transport.is_closing():
            logger.info('Closing serial port connection')
            transport.close()
            transport, protocol = None, None
            continue
        
        if command == 'send':
            transport.write(payload.encode())
        
        await asyncio.sleep(_ZMQ_PAIR_SLEEP)

async def zmq_sender():
    while True:
        try:
            message = to_socket_server_queue.get(block=False)
        except:
            await asyncio.sleep(_ZMQ_PAIR_SLEEP)
            continue
        await socket.send_string(json.dumps(message))

class Output(asyncio.Protocol):
    
    def connection_made(self, transport):
        logger.info('Serial port connection opened')
        self.transport = transport
        
    def data_received(self, data):
        logger.info(f'Received message from serial port: {data}')
        to_socket_server_queue.put({'type': 'serial_port_data', 'data': data.decode()})

    def connection_lost(self, exc):
        logger.info(f'Serial port connection lost')
        to_socket_server_queue.put({'type': 'message', 'message': 'serial_port_connection_lost'})

    def pause_writing(self):
        pass

    def resume_writing(self):
        pass

async def main(loop):
    await asyncio.gather(zmq_sender(), zmq_receiver(loop))

loop = asyncio.get_event_loop()
loop.run_until_complete(main(loop))
