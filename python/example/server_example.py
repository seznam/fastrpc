#!/usr/bin/python

import sys
import fastrpc
import socket

data = {}

def get_method(key):
    try:
        return {"status": 200, "value": data[key]}
    except KeyError:
        return {"status": 404}


def set_method(key, value):
    data[key] = value
    return {"status": 200, "value": value}


server = fastrpc.Server(readTimeout=1000)
server.registry.register("get", get_method, "S:s")
server.registry.register("set", set_method, "S:ss")
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind(("0", 3132))
sock.listen(1000)

while 1:
    (clientsocket, address) = sock.accept()
    server.serve(clientsocket, address)
