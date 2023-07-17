# FastRPC-static

FastRPC is a XML-RPC based protocol that uses binary data serialization format (see [Binary protocol specification](https://github.com/seznam/fastrpc/wiki/FastRPC-binary-protocol-specification)).
Library includes C++, Python and TypeScript/JavaScript implementations and a console client.

The library supports both protocols due to FastRPC's full data compatibility
with the XML-RPC. Data format is negotiated via the HTTP content negotiation.

NOTE: This is static, meson built, version of fastrpc that has no debian packaging *by design*. It is used as a dependency in other projects using a wrap file/subproject.

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

* Python module build is unsupported for now - feel free to send MRs.

## TypeScript/JavaScript

See separate [instructions on how to install and use TypeScript/JavaScript library](typescript/README.md).
