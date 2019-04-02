import aiohttp
from aiohttp import web
import asyncio
import zmq
from zmq.asyncio import Context


_WS_SLEEP_TIME = 0.5
_ZMQ_PAIR_SLEEP = 0.5

_ZMQ_URL = 'tcp://localhost:9999'

message_queue = asyncio.Queue()

context = Context.instance()


async def zmq_receiver():
    socket = context.socket(zmq.PAIR)
    socket.connect(_ZMQ_URL)
    while True:
        message = await socket.recv_string()
        await message_queue.put(message)
        await asyncio.sleep(_ZMQ_PAIR_SLEEP)


async def websocket_handler(request):

    ws = web.WebSocketResponse()
    await ws.prepare(request)

    while True:
        message = await message_queue.get()
        if message:
            await ws.send_str(message)
        else:
            await asyncio.sleep(_WS_SLEEP_TIME)


app = web.Application()
app.add_routes([web.get('/', websocket_handler)])


async def start_background_tasks(app):
    app['zmq_receiver'] = app.loop.create_task(zmq_receiver())

app.on_startup.append(start_background_tasks)

web.run_app(app)
