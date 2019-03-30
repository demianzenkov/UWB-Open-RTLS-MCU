import time
import asyncio
import serial_asyncio
import zmq
from zmq.asyncio import Context, Poller


url = 'tcp://127.0.0.1:5555'
ctx = Context.instance()
# queue = asyncio.Queue()


async def receiver():
    """receive messages with polling"""
    pull = ctx.socket(zmq.PULL)
    pull.connect(url)
    poller = Poller()
    poller.register(pull, zmq.POLLIN)
    while True:
        events = await poller.poll()
        if pull in dict(events):
            # print("recving", events)
            msg = await pull.recv_multipart()
            print('recvd', msg)


async def sender():
    """send a message every second"""
    tic = time.time()
    push = ctx.socket(zmq.PUSH)
    push.bind(url)
    while True:
        print("sending")
        await push.send_multipart([str(time.time() - tic).encode('ascii')])
        await asyncio.sleep(1)



async def main():
    # reader, writer = await serial_asyncio.open_serial_connection(url='/dev/tty.usbserial-A9A51FF3', baudrate=115200)
    reader, writer = await serial_asyncio.open_serial_connection(url='reader', baudrate=115200)
    print('Reader, writer created')
    messages = [b'foo\n', b'bar\n', b'baz\n', b'qux\n']
    # sent = send(writer, messages)
    received = receive(reader)
    await asyncio.wait([received,
                        # ping(),
                        receiver(),
                        sender()])


async def send(w, msgs):
    for msg in msgs:
        w.write(msg)
        print(f'sent: {msg.decode().rstrip()}')
        await asyncio.sleep(0.5)
    print('Done sending')


async def receive(r):
    while True:
        msg = await r.readline()
        if msg.rstrip() == b'DONE':
            print('Done receiving')
            break
        print(f'received: {msg.rstrip().decode()}')


loop = asyncio.get_event_loop()
loop.run_until_complete(main())
loop.close()

