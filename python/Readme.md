# Fastrpc

FastRPC is a RPC (Remote Procedure Call) protocol based on the XML-RPC which
uses binary format for data serialization. It uses HTTP as transport protocol
as the XML-RPC does.

The library supports both protocols due to FastRPC's full data compatibility
with the XML-RPC. Data format is negotiated via the HTTP content negotiation.

The fastrpc Python module is compatible with the xmlrpclib module. It's very
simple to port project from xmlrpclib to fastrpc.

## How to build

1. first you need to build `libfastrpc`

```sh
# in fastrpc folder
autoreconf -if
./configure
make
```

2. build python wrapper (wheel package is recommended) and run tests

```sh
# in fastrpc/python folder
python setup.py bdist_wheel
```
