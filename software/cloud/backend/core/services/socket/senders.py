import socket
import binascii

_STATION_TO_CLOUD_PREFIX = 0x94
_CLOUD_TO_STATION_PREFIX = 0x9D

_CRC8_POLY = 0x8B
_CRC16_POLY = 0x1021

_AVAILABLE_OPERATION_CODES = [
    0x06,
    0x07,
    0x08,
    0x09,
    0xA,
    0xB,
    0xC,
    0xD,
    0xE
]


def crc8(pbuf):
    crc = 0xFF
    for byte_ in pbuf:
        crc = (crc ^ byte_)
        for _ in range(8):
            crc = ((crc << 1) ^ _CRC8_POLY) if (crc & 0x80) else (crc << 1)
            crc = crc & 0xFF
    return crc


def crc16(pbuf):
    crc = 0xFFFF
    crc ^= pbuf << 8
    for _ in range(8):
        crc = ((crc << 1) ^ _CRC16_POLY) if crc & 0x8000 else crc << 1
        crc &= 0xFFFF
    return crc


def send_read_network_settings_command(sock, network_address: str):
    operation_code = 0x06
    network_address = [int(i) for i in network_address.split('.')]
    body = [_CLOUD_TO_STATION_PREFIX, *network_address, operation_code]
    checksum = crc8(body)
    sock.send(bytes([*body, checksum]))


def send_write_network_settings_command(sock, current_network_address: str, new_network_address: str, subnet_mask: str, server_ip: str, server_port: int):
    operation_code = 0x07
    current_network_address = [int(i) for i in current_network_address.split('.')]
    new_network_address = [int(i) for i in new_network_address.split('.')]
    subnet_mask = [int(i) for i in subnet_mask.split('.')]
    server_ip = [int(i) for i in server_ip.split('.')]
    body = [_CLOUD_TO_STATION_PREFIX, *current_network_address,
            operation_code, *new_network_address, *subnet_mask, *server_ip, server_port]
    checksum = crc8(body)
    sock.send(bytes([*body, checksum]))


def send_read_module_settings_command(sock, network_address: str):
    operation_code = 0x08
    network_address = [int(i) for i in network_address.split('.')]
    body = [_CLOUD_TO_STATION_PREFIX, *network_address, operation_code]
    checksum = crc8(body)
    sock.send(bytes([*body, checksum]))
