import aiohttp
from aiohttp import web
import asyncio
import zmq
from zmq.asyncio import Context, Poller

message_queue = asyncio.Queue()

zmq_url = 'tcp://127.0.0.1:5555'

async def pzm():
    # todo: zmq listener
    ctx = Context.instance()
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

        await message_queue.put('>>>>>>')
        await asyncio.sleep(2.0)


async def zmq_parser(msg):
    print('zmq_rx: ', msg)

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
    app['zmq_listener'] = app.loop.create_task(pzm())

app.on_startup.append(start_background_tasks)

web.run_app(app)
