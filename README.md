# FastRPC

FastRPC is a XML-RPC based protocol that uses binary data serialization format (see [Binary protocol specification](https://github.com/seznam/fastrpc/wiki/FastRPC-binary-protocol-specification)).
Library includes C++, Python and TypeScript/JavaScript implementations and a console client.

The library supports both protocols due to FastRPC's full data compatibility
with the XML-RPC. Data format is negotiated via the HTTP content negotiation.

## Installation

Installation is possible by hand. Normally though, this would be used as a dependency for other meson projects using a subproject approach.

```bash
git clone https://github.com/seznam/fastrpc.git
cd fastrpc
mkdir build
cd build
meson setup ..
ninja
sudo ninja install
```

## Usage

See [examples](https://github.com/seznam/fastrpc/tree/master/example) for learning how to work with the C++ library.

## Python

See separate [instructions on how to install and use python library](python/README.md).

## TypeScript/JavaScript

See separate [instructions on how to install and use TypeScript/JavaScript library](typescript/README.md).
