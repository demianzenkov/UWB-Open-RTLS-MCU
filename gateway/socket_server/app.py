import aiohttp
from aiohttp import web
import asyncio
import zmq
from zmq.asyncio import Context, Poller

message_queue = asyncio.Queue()

ctx = Context.instance()
zmq_url = 'tcp://127.0.0.1:5555'


async def dummy_ws():
    while True:
        await message_queue.put('>>>>>>')
        await asyncio.sleep(2.0)


async def zmq_receiver():
    """receive messages with polling"""
    pull = ctx.socket(zmq.PULL)
    pull.connect(zmq_url)
    poller = Poller()
    poller.register(pull, zmq.POLLIN)
    while True:
        events = await poller.poll()
        if pull in dict(events):
            msg = await pull.recv_multipart()
            print('zmq_rx: ', msg)


async def zmq_sender():
    push = ctx.socket(zmq.PUSH)
    # push.bind(zmq_url)
    while True:
        await push.send_multipart(["hello from socket server".encode('ascii')])
        await asyncio.sleep(2.5)
        print("zmq sent")


async def websocket_handler(request):

    ws = web.WebSocketResponse()
    await ws.prepare(request)
    async for msg in ws:
        print(msg.data)

    while True:
        message = await message_queue.get()
        if message:
            await ws.send_str(message)
        else:
            await asyncio.sleep(0.5)


app = web.Application()
app.add_routes([web.get('/ws', websocket_handler)])


async def start_background_tasks(app):
    app['dummy_ws'] = app.loop.create_task(dummy_ws())
    app['zmq_receiver'] = app.loop.create_task(zmq_receiver())
    app['zmq_sender'] = app.loop.create_task(zmq_sender())

app.on_startup.append(start_background_tasks)

web.run_app(app)
