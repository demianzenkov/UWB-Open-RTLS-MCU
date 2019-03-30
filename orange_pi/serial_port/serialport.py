import asyncio
import serial_asyncio


async def main(loop):
    reader, writer = await serial_asyncio.open_serial_connection(url='/dev/tty.usbserial-A9A51FF3', baudrate=115200)
    print('Reader, writer created')
    messages = [b'foo\n', b'bar\n', b'baz\n', b'qux\n']
    # sent = send(writer, messages)
    received = receive(reader)
    await asyncio.wait({received})


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
loop.run_until_complete(main(loop))
loop.close()