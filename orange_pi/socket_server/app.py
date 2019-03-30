import aiohttp
from aiohttp import web
import asyncio
import zmq
import zmq.asyncio

message_queue = asyncio.Queue()


async def pzm():
    # todo: zmq listener
    while True:
        await message_queue.put('>>>>>>')
        await asyncio.sleep(2.0)


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
