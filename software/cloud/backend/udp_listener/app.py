import socket
import time
import psycopg2
import logging
import json

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


def _get_anchors(fields=('id', 'ip', 'port', 'server_ip', 'server_port', 'subnet_mask')):
    cur = pg_conn.cursor()
    cur.execute(f"select {', '.join(fields)} from anchor order by id asc;")
    res = list(cur)
    cur.close()

    anchors_ = []

    for row in res:
        row_ = []
        for v in row:
            if type(v) is str:
                v = v.strip(' ')
            row_.append(v)
        anchors_.append(row_)

    return anchors_


def is_anchor_exists(address: tuple):
    cur = pg_conn.cursor()
    cur.execute(f'select ip, port from anchor where ip = \'{address[0]}\' and port = \'{address[1]}\';')
    res = list(cur)
    cur.close()

    return len(res) > 0


for anchor in _get_anchors(fields=('ip', 'port')):
    _ACQUAINTED_STATIONS.append((anchor[0], anchor[1]))

logger.info(f'Acquainted anchors: {_ACQUAINTED_STATIONS}')


def _create_anchor(ip, port, subnet_mask, server_ip_address, server_port):
    cur = pg_conn.cursor()
    logger.info(f'Creating database anchor entry for {ip}')
    try:
        cur.execute(
            f'''insert into anchor
             (ip, port, server_ip, server_port, subnet_mask) 
             select 
             \'{ip}\', \'{port}\', \'{server_ip_address}\', \'{server_port}\', \'{subnet_mask}\'
             where not exists 
             (select ip from anchor where ip = \'{ip}\' and port = \'{port}\');''')
        logger.info(f'Inserted {cur.statusmessage.split(" ")[-1]} rows\nFull db status message: {cur.statusmessage}')
        pg_conn.commit()
    except Exception as e:
        logger.exception(e)
        cur.close()
    else:
        logger.info(f'Creating database anchor entry for {ip} [Success]')
    finally:
        cur.close()


def _set_received_read_network_settings_command_response(address):
    cur = pg_conn.cursor()
    try:
        cur.execute(
            f"""update anchor 
                set is_waiting_for_read_network_settings_command_response = false 
                where ip = \'{address[0]}\' and port = \'{address[1]}\';""")
        logger.info(f'Updated {address} anchor. Full db status message: {cur.statusmessage}')
        pg_conn.commit()
    except Exception as e:
        logger.exception(e)
        cur.close()
    finally:
        cur.close()


def _handle_read_network_settings_command(data, address):
    station_ip = data[8:16]
    subnet_mask = data[16:24]
    server_ip = data[24:32]
    server_port = data[32:36]

    station_ip = '.'.join([str(int(station_ip[i] + station_ip[i + 1], 16)) for i in range(0, len(station_ip), 2)])
    subnet_mask = '.'.join([str(int(subnet_mask[i] + subnet_mask[i + 1], 16)) for i in range(0, len(subnet_mask), 2)])
    server_ip = '.'.join([str(int(server_ip[i] + server_ip[i + 1], 16)) for i in range(0, len(server_ip), 2)])
    server_port = int(server_port, 16)

    logger.info(f'''Handling read_network_settings command response:
                    \t\tAddress (from socket): {address}
                    \t\tStation ip (from payload):{station_ip}
                    \t\tServer ip: {server_ip}
                    \t\tServer port: {server_port}
                    \t\tSubnet mask: {subnet_mask}''')

    if not is_anchor_exists(address):
        logger.info(f'Anchor with address {address} not represented in db.')
        _create_anchor(address[0], address[1], subnet_mask, server_ip, server_port)

    _set_received_read_network_settings_command_response(address)

    fields = ('id', 'ip', 'port', 'server_ip', 'server_port', 'subnet_mask',
              'is_waiting_for_read_network_settings_command_response',
              'is_waiting_for_write_network_settings_command_response')
    data = [dict(zip(fields, tuple(row))) for row in _get_anchors(fields=fields)]

    socket_payload = {
        'type': 'anchors',
        'data': data
    }

    socket_payload = json.dumps(socket_payload)
    ws_sock.send(socket_payload.encode())


def _handle_write_network_settings_command(data, address):
    is_command_completed_successful = data[6:8]

    cur = pg_conn.cursor()
    try:
        cur.execute(
            f"""update anchor 
                set is_waiting_for_write_network_settings_command_response = false 
                where ip = \'{address[0]}\' and port = \'{address[1]}\';""")
        logger.info(f'Updated {address} anchor. Full db status message: {cur.statusmessage}')
        pg_conn.commit()
    except Exception as e:
        logger.exception(e)
        cur.close()
    finally:
        cur.close()

    logger.info(f'Write network settings command returned code: {is_command_completed_successful}')

    fields = ('id', 'ip', 'port', 'server_ip', 'server_port', 'subnet_mask',
              'is_waiting_for_read_network_settings_command_response',
              'is_waiting_for_write_network_settings_command_response')
    data = [dict(zip(fields, tuple(row))) for row in _get_anchors(fields=fields)]

    socket_payload = {
        'type': 'anchors',
        'data': data
    }

    socket_payload = json.dumps(socket_payload)
    ws_sock.send(socket_payload.encode())


def _handle_read_module_settings_command(data, address):
    channel_number = data[6:8]
    pulse_repetition_frequency = data[8:10]
    tx_preamble_length = data[10:12]
    rxpac = data[12:14]
    tx_preamble_code = data[14:16]
    rx_preamble_code = data[16:18]
    nonstd_sfd = data[18:20]
    data_rate = data[20:22]
    pht_mode = data[22:24]
    sfd_timeout_value = data[24:26]

    # write to db


def _handle_location_data(data, address):
    tag_id = data[6:10]
    message_id = data[10:14]
    hardware_timestamp = data[14:22]

    tag_id = int(tag_id, 16)
    message_id = int(message_id, 16)
    hardware_timestamp = int(hardware_timestamp, 16)

    cur = pg_conn.cursor()
    try:
        cur.execute(
            f"""insert into message 
                (tag_id, message_id, hardware_timestamp)
                values
                (\'{tag_id}\', \'{message_id}\', {hardware_timestamp})
                """)
        logger.info(f'Inserted message from {address} anchor. Full db status message: {cur.statusmessage}')
        pg_conn.commit()
    except Exception as e:
        logger.exception(e)
        cur.close()
    finally:
        cur.close()


operations_handlers = {
    '06': _handle_read_network_settings_command,
    '07': _handle_write_network_settings_command,
    '08': _handle_read_module_settings_command,
    '0c': _handle_location_data,
}


def parse(data, address):
    data = str(data)[2:-3]
    operation_code = data[4:6]

    handler = operations_handlers.get(operation_code.lower())
    if not handler:
        logger.info(f'No handler found for operation code: {operation_code}')
        return

    try:
        handler(data, address)
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
