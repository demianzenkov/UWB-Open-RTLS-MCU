import zmq
import asyncio
import logging
import json
import serial_asyncio
from zmq.asyncio import Context

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

_ZMQ_PAIR_SLEEP = 0.5
_ZMQ_URL = 'tcp://127.0.0.1:9999'
_SERIAL_PORT_ADDRESS = '/dev/tty.usbserial-A9A51FF3'
_BAUD_RATE = 115200

context = Context.instance()
socket = context.socket(zmq.PAIR)
socket.bind(_ZMQ_URL)

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

        assert command in ['open', 'close', 'send']

        if transport is None and command != 'open':
            logger.warning('Closed serial port can only receive open command')
            continue
        
        if command == 'open' and transport is not None:
            logger.info('Omitting open command. Serial port is already opened')
            continue
        
        if command == 'open' and (transport is None or transport.is_closing()):
            logger.info('Opening serial port connection')
            transport, protocol = await loop.create_task(serial_asyncio.create_serial_connection(loop, Output, _SERIAL_PORT_ADDRESS, baudrate=_BAUD_RATE))
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

async def zmq_sender(message: str):
    await socket.send_string(message)

class Output(asyncio.Protocol):
    
    def connection_made(self, transport):
        logger.info('Serial port connection opened')
        self.transport = transport
        
    def data_received(self, data):
        logger.info(f'Received message from serial port: {data}')

    def connection_lost(self, exc):
        logger.info(f'Serial port connection lost')

    def pause_writing(self):
        pass

    def resume_writing(self):
        pass

loop = asyncio.get_event_loop()
loop.run_until_complete(zmq_receiver(loop))