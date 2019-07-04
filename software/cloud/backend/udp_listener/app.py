import socket
import time
import psycopg2
import logging

from senders import send_read_network_settings_command

_UDP_HOST = '0.0.0.0'
_UDP_PORT = 5005

_WS_UDP_HOST = 'ws'
_WS_UDP_PORT = 5004

pg_conn = psycopg2.connect(host="db", database="core", user="root", password="root")

logger = logging.getLogger('udp_listener')
logger.setLevel(logging.DEBUG)
ch = logging.StreamHandler()
ch_format = logging.Formatter('%(asctime)s : [%(name)s] : [%(levelname)s] : %(message)s')
ch.setFormatter(ch_format)

logger.addHandler(ch)
logger.setLevel(logging.DEBUG)

ws_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
ws_sock.connect((_WS_UDP_HOST, _WS_UDP_PORT))

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setblocking(False)
sock.bind((_UDP_HOST, _UDP_PORT))

_ACQUAINTED_STATIONS = []


def get_anchors():
    cur = pg_conn.cursor()
    cur.execute("select id, ip from devices_anchor;")
    res = list(cur)
    cur.close()

    return res


def _create_anchor(ip, subnet_mask, server_ip_address, server_port):
    cur = pg_conn.cursor()
    logger.info(f'Creating database anchor entry for {ip}')
    try:
        cur.execute(
            f'''insert into devices_anchor
             (ip, subnet_mask, server_ip_address, server_port) 
             select 
             \'{ip}\', \'{subnet_mask}\', \'{server_ip_address}\', \'{server_port}\'
             where not exists 
             (select ip from devices_anchor where ip = \'{ip}\');''')
        pg_conn.commit()
    except Exception as e:
        logger.exception(e)
    else:
        logger.info(f'Creating database anchor entry for {ip} [Success]')
    finally:
        cur.close()


def _handle_read_network_settings_command(data):
    station_ip = data[8:16]
    subnet_mask = data[16:24]
    server_ip = data[24:32]
    server_port = data[32:36]

    station_ip = '.'.join([str(int(station_ip[i] + station_ip[i + 1], 16)) for i in range(0, len(station_ip), 2)])
    subnet_mask = '.'.join([str(int(subnet_mask[i] + subnet_mask[i + 1], 16)) for i in range(0, len(subnet_mask), 2)])
    server_ip = '.'.join([str(int(server_ip[i] + server_ip[i + 1], 16)) for i in range(0, len(server_ip), 2)])
    server_port = int(server_port, 16)

    _create_anchor(station_ip, subnet_mask, server_ip, server_port)


def _handle_write_network_settings_command(data):
    pass


def _handle_read_module_settings_command(data):
    pass


operations_handlers = {
    '06': _handle_read_network_settings_command,
    '07': _handle_write_network_settings_command,
    '08': _handle_read_module_settings_command,
}


def parse(data, address):
    data = str(data)[2:-3]
    operation_code = data[4:6]

    handler = operations_handlers.get(operation_code)
    if not handler:
        logger.info(f'No handler found for operation code: {operation_code}')
        return

    try:
        handler(data)
    except Exception as e:
        logger.exception(e)


def listen_udp():
    sock_host = sock.getsockname()
    reqs = 0
    t1 = time.time()
    while True:
        t2 = time.time()
        if t2 - t1 >= 1:
            # logger.info(f'Socket reqs/sec: {reqs}')
            t1 = t2
            reqs = 0
        try:
            data, address = sock.recvfrom(4096)
            logger.info(f'Socket {sock_host} received: {data}')
            if address not in _ACQUAINTED_STATIONS:
                logger.info(f'New base station: {address}. Sending read_network_settings command. Skipping parsing')
                send_read_network_settings_command(sock, address, address[0])
                _ACQUAINTED_STATIONS.append(address)
            else:
                parse(data, address)
            # ws_sock.send(data)
            reqs += 1
        except socket.timeout as e:
            pass
        except socket.error as e:
            pass
        else:
            pass


if __name__ == '__main__':
    listen_udp()
