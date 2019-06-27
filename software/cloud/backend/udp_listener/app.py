import socket
import time
import threading
import psycopg2
import logging

_UDP_HOST = '0.0.0.0'
_UDP_PORT = 5005

pg_conn = psycopg2.connect(host="db", database="core", user="root", password="root")

logger = logging.getLogger('udp_listener')
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch_format = logging.Formatter('%(asctime)s : [%(name)s] : [%(levelname)s] : %(message)s')
ch.setFormatter(ch_format)

logger.addHandler(ch)
logger.setLevel(logging.DEBUG)

ws_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ws_sock.connect(('ws', 5004))


def listen_udp(sock):
    reqs = 0
    t1 = time.time()
    while True:
        t2 = time.time()
        if t2 - t1 >= 1:
            # logger.info(f'Socket reqs/sec: {reqs}')
            t1 = t2
            reqs = 0
        data = None
        try:
            data = sock.recv(5000)
            logger.info(f'Socket {sock.getsockname()} received: {data}')
            ws_sock.send(data)
            reqs += 1
        except socket.timeout as e:
            pass
        except socket.error as e:
            pass
        else:
            pass
    sock.close()


def get_anchors():
    cur = pg_conn.cursor()
    cur.execute("select id, ip from devices_anchor;")
    res = list(cur)
    cur.close()

    return res


sockets = {}


if __name__ == '__main__':
    while True:
        logger.info('Refreshing anchors')
        anchors = get_anchors()
        logger.info(f'Anchors:\n{anchors}')
        for idx, anchor in enumerate(anchors):
            id_, ip_ = anchor
            if id_ in sockets.keys():
                continue

            bind_address = (_UDP_HOST, _UDP_PORT + idx)
            connect_address = (ip_, 5005)

            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            sock.bind(bind_address)
            sock.setblocking(False)
            # sock.connect(connect_address)
            logger.info(f'Created socket:\nBinded on: {bind_address}\nConnected to: {connect_address}')

            sockets[id_] = sock

            t = threading.Thread(None, listen_udp, args=(sock, ))
            t.start()
        time.sleep(15.0)
