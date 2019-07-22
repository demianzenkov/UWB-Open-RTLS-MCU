#!/bin/bash
set -e

psql -a -v ON_ERROR_STOP=1 --username "$POSTGRES_USER" <<-EOSQL
    CREATE ROLE root LOGIN PASSWORD 'root' NOSUPERUSER NOCREATEDB NOCREATEROLE NOREPLICATION;

    CREATE DATABASE core WITH ENCODING='UTF8' OWNER=root;

    \c core

    CREATE SEQUENCE anchor_id_seq;

    ALTER SEQUENCE anchor_id_seq OWNER TO root;

    CREATE TABLE anchor (
    id int primary key not null default nextval('anchor_id_seq'),
    ip char(16),
    port int,
    server_ip char(16),
    server_port int,
    subnet_mask char(16),
    is_waiting_for_read_network_settings_command_response bool default false,
    is_waiting_for_write_network_settings_command_response bool default false
    );

    ALTER TABLE anchor OWNER TO root;

    ALTER SEQUENCE anchor_id_seq OWNED BY anchor.id;

    CREATE SEQUENCE message_id_seq;

    alter sequence message_id_seq owner to root;

    create table message (
    id int primary key not null default nextval('message_id_seq'),
    tag_id char(128),
    message_id char (128),
    hardware_timestamp int,
    received_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
    );

    ALTER TABLE message OWNER TO root;

    ALTER SEQUENCE message_id_seq OWNED BY message.id;

EOSQL
