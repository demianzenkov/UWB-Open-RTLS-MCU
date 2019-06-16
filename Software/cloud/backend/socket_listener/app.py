import asyncio
import aiohttp

SERVER_HOST = '172.17.0.1'
SERVER_PORT = '3001'

async def main():
    async with aiohttp.ClientSession() as session:
        async with session.ws_connect(f'ws://localhost:3001/socket.io/\?transport=websocket') as ws:
            async for msg in ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    print(msg.data)
                elif msg.type == aiohttp.WSMsgType.ERROR:
                    break

loop = asyncio.get_event_loop()
loop.run_until_complete(main())
