from aiohttp import web
import asyncio
import asyncpg
import json

pg_conn = None


async def connect():
    global pg_conn
    pg_conn = await asyncpg.connect(user='root', password='root',
                                    database='core', host='db')


async def handle(request):
    anchors = await get_anchors()

    anchors_ = []

    for anchor in anchors:
        anchor_ = {}
        anchor = dict(anchor)
        for k, v in anchor.items():
            if type(v) is str:
                v = v.strip(' ')
            anchor_[k] = v
        anchors_.append(anchor_)

    resp = web.Response(text=json.dumps(anchors_))
    resp.headers['Access-Control-Allow-Origin'] = 'http://localhost:3000'

    return resp


async def get_anchors():
    values = await pg_conn.fetch('''SELECT * FROM anchor;''')
    return values


loop = asyncio.get_event_loop()
loop.run_until_complete(connect())

app = web.Application()
app.add_routes([web.get('/anchors', handle),
                web.get('/tags', handle)])

if __name__ == '__main__':
    web.run_app(app, host='0.0.0.0', port=8000)
