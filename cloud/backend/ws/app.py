import socket
import aiohttp
import asyncio
import logging
from aiohttp import web
import json

logger = logging.getLogger('ws')
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch_format = logging.Formatter('%(asctime)s : [%(name)s] : [%(levelname)s] : %(message)s')
ch.setFormatter(ch_format)

logger.addHandler(ch)
logger.setLevel(logging.DEBUG)


async def websocket_handler(request):

    ws = web.WebSocketResponse()
    await ws.prepare(request)

    app['data']['ws'] = ws

    async for msg in ws:
        if msg.type == aiohttp.WSMsgType.TEXT:
            logger.info(f'Received message: {msg.data}')
        elif msg.type == aiohttp.WSMsgType.ERROR:
            logger.info(f'Websocket connection closed with exception {ws.exception()}')

    logger.info('Websocket connection closed')

    return ws

app = web.Application()
app['data'] = {}
app.add_routes([web.get('/ws', websocket_handler)])


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 5004))
sock.setblocking(False)


async def listen_socket():
    while True:
        await asyncio.sleep(0.1)
        try:
            data = sock.recv(4096)
            data = data.decode()
            data = json.loads(data)
            logger.info(f'Websocket received {data}')
            await app['data'].get('ws').send_json(data)
        except BlockingIOError:
            pass
        except AttributeError as e:
            logger.exception(e)
            logger.info('Browser probably lost ws connection. Reload page.')
        except Exception as e:
            logger.exception(e)


asyncio.get_event_loop().create_task(listen_socket())

web.run_app(app, host='0.0.0.0', port=8080)
