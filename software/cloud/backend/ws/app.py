import socket
import aiohttp
import asyncio
from aiohttp import web


async def websocket_handler(request):

    ws = web.WebSocketResponse()
    await ws.prepare(request)

    app['data']['ws'] = ws

    # await ws.send_str('{}')

    async for msg in ws:
        if msg.type == aiohttp.WSMsgType.TEXT:
            print(f'received some {msg.data}')
        elif msg.type == aiohttp.WSMsgType.ERROR:
            print('ws connection closed with exception %s' %
                  ws.exception())

    print('websocket connection closed')

    return ws

app = web.Application()
app['data'] = {}
app.add_routes([web.get('/ws', websocket_handler)])


sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(('0.0.0.0', 5004))
sock.setblocking(0)


async def t():
    while True:
        await asyncio.sleep(0.1)
        try:
            data = sock.recv(5000)
            print('sock', data)
        except:
            pass
        else:
            await app['data'].get('ws').send_str(data.decode())

asyncio.get_event_loop().create_task(t())

web.run_app(app, host='0.0.0.0', port=8080)
