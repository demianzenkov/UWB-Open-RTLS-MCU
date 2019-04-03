import aiohttp
from aiohttp import web
import asyncio
import zmq
import logging
import sys
from zmq.asyncio import Context

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

formatter = logging.Formatter('%(asctime)s : [%(levelname)s] : %(message)s')
handler = logging.StreamHandler(sys.stdout)

handler.setFormatter(formatter)
logger.addHandler(handler)


_WS_SLEEP_TIME = 0.5
_ZMQ_PAIR_SLEEP = 0.5

_ZMQ_URL = 'tcp://localhost:9999'

context = Context.instance()
socket = context.socket(zmq.PAIR)
socket.connect(_ZMQ_URL)

async def zmq_send(message: str):
    await socket.send_string(message)

async def zmq_receiver():
    while True:
        client_ws = app['clients'].get('cloud_ws')
        message = await socket.recv_string()
        logger.info(f'Received from ZMQ Pair: {message}')
        if client_ws is not None:
            await client_ws.send_str(message)
            logger.info(f'Message {message} sent via websocket to cloud')
        await asyncio.sleep(_ZMQ_PAIR_SLEEP)


async def websocket_handler(request):
    secret = request.query.get('secret')
    if not secret:
        return web.Response(status=403)

    ws = web.WebSocketResponse()
    await ws.prepare(request)
    app['clients']['cloud_ws'] = ws
    async for message in ws:
        logger.info(f'Socket received message from cloud: {message.data}')
        # await zmq_send(message.data)
        logger.info(f'Socket sent message to zmq: {message.data}')
    await ws.close()
    return ws


app = web.Application()
app['clients'] = {}
app.add_routes([web.get('/', websocket_handler)])


async def start_background_tasks(app):
    app['zmq_receiver'] = app.loop.create_task(zmq_receiver())

app.on_startup.append(start_background_tasks)

web.run_app(app)
