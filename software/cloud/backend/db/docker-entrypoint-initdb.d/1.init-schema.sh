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
    subnet_mask char(16)
    );

    ALTER TABLE anchor OWNER TO root;

    ALTER SEQUENCE anchor_id_seq OWNED BY anchor.id;
EOSQL
