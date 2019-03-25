import aiohttp

SERVER_HOST = ''
SERVER_PORT = ''

async with session.ws_connect(f'http://{SERVER_HOST}:{SERVER_HOST}/ws') as ws:
    async for msg in ws:
        if msg.type == aiohttp.WSMsgType.TEXT:
            print(msg.data)
        elif msg.type == aiohttp.WSMsgType.ERROR:
            break