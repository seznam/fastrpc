# Python FastRPC library

The fastrpc Python package provides funkcionality for easy creating both HTTP clients and
servers using FastRPC or XML-RPC protocols.

The fastrpc Python package is compatible with the xmlrpclib (xmlrpc.client in Python 3) package.
It's very simple to port project from xmlrpclib to fastrpc.

## Installation

The binary fastrpc package needs to be installed first.
See the [installation instructions](../README.md#installation).

Python package can be installed using setuptools:

```bash
git clone https://github.com/seznam/fastrpc.git
cd fastrpc/python

sudo python setup.py install
```

## Usage

### Client

`ServerProxy` can be used very straightforwardly for client side code.

```python
client = fastrpc.ServerProxy(url)
client.system.listMethods()
```

### Server

There are handlers for adding FastRPC endpoints to Flask, Aiohttp and Tornado web applications.

```python
from fastrpc.handler.flask import FastRPCHandler

rpc = FastRPCHandler(flask_app, url='/RPC2')
rpc.register_method('add', add)
```

`Server` can be also used to directly handle client sockets.

```python
server = fastrpc.Server()
server.registry.register('add', add, 'S:s')

while True:
    (clientsocket, address) = sock.accept()
    server.serve(clientsocket, address)
```

For more specific needs, one can also use `loads` and `dumps` functions for converting the data between FastRPC/XML-RPC structures and Python objects and thus can create its own handler.

See [Python examples](https://github.com/seznam/fastrpc/tree/master/python/example) for more.
