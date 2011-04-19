/*
 * FastRPC -- Fast RPC library compatible with XML-RPC
 * Copyright (C) 2005-7  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Radlicka 2, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:fastrpc@firma.seznam.cz
 *
 * FILE          $Id: frpcserver.cc,v 1.17 2011-02-25 09:21:07 volca Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#include "frpcserver.h"

#include <string>
#include <algorithm>
#include <sstream>
#include <memory>
#include <functional>

#include <stdexcept>
#include <frpchttpclient.h>
#include <frpcstreamerror.h>
#include <frpchttpclient.h>
#include <frpctreebuilder.h>
#include <frpcunmarshaller.h>
#include <frpcmarshaller.h>
#include <frpchttperror.h>
#include <frpcinternals.h>
#include <frpc.h>
#include <frpcsocket.h>


namespace FRPC {
namespace {

inline unsigned int chooseType(unsigned int type) {
    switch(type) {
    case Server_t::BINARY_RPC:
        return Marshaller_t::BINARY_RPC;
    case Server_t::JSON:
        return Marshaller_t::JSON;
    case Server_t::BASE64_RPC:
        return Marshaller_t::BASE64_RPC;
    case Server_t::XML_RPC:
    default:
        return Marshaller_t::XML_RPC;
    }
}

} // namespace

Server_t::~Server_t()
{}

void Server_t::serve(int fd, struct sockaddr_in* addr) {
    HTTPHeader_t headerIn;
    HTTPHeader_t headerOut;
    serve(fd, addr, headerIn, headerOut);
}

void Server_t::serve(int fd,
                     struct sockaddr_in* addr,
                     HTTPHeader_t &headerIn,
                     HTTPHeader_t &headerOut)
{
    // prepare query storage
    queryStorage.push_back(std::string());
    queryStorage.back().reserve(BUFFER_SIZE + HTTP_BALLAST);
    contentLength = 0;
    closeConnection = false;
    headersSent = false;
    head = false;

    char tmpstr[256];
    std::string clientIP;

    io.setSocket(fd);

    if (!addr) {
        struct sockaddr_in clientaddr;
        socklen_t sinSize = sizeof( struct sockaddr_in );
        if (getpeername(fd,(struct sockaddr*) &clientaddr, &sinSize) == 0 ) {
            clientIP = inet_ntop(AF_INET, &(clientaddr.sin_addr), tmpstr, 256);
        } else {
            clientIP = "unknown";
        }
    } else {
        clientIP = inet_ntop(AF_INET, &(addr->sin_addr), tmpstr, 256);
    }

    unsigned int requestCount = 0;
    do {
        Pool_t pool;
        TreeBuilder_t builder(pool);
        try {
            methodRegistry.preReadCallback();
            readRequest(builder, headerIn);

        } catch(const StreamError_t &streamError) {
            std::auto_ptr<Marshaller_t>
                marshaller(Marshaller_t::create(chooseType(outType),
                                                *this,
                                                ProtocolVersion_t()));

            marshaller->packFault(MethodRegistry_t::FRPC_PARSE_ERROR,
                                  streamError.message().c_str());
            marshaller->flush();
            continue;

        } catch(const HTTPError_t &httpError) {
            sendHttpError(httpError);
            break;
        }

        headerOut = HTTPHeader_t();
        this->headerOut = &headerOut;

        try {
            if (head) {
                int result = methodRegistry.headCall();
                if (result == 0)
                    flush();
                else if (result < 0)
                    throw HTTPError_t(HTTP_METHOD_NOT_ALLOWED,
                                      "Method Not Allowed");
                else
                    throw HTTPError_t(HTTP_SERVICE_UNAVAILABLE,
                                      "Service Unavailable");
            } else {
                methodRegistry.processCall(clientIP,
                                           builder.getUnMarshaledMethodName(),
                                           Array(builder.getUnMarshaledData()),
                                           *this,
                                           chooseType(outType),
                                           protocolVersion);
            }
        } catch(const HTTPError_t &httpError) {
            sendHttpError(httpError);
            break;
        }

        requestCount++;


    } while (!(closeConnection == true
               || keepAlive == false
               || requestCount >= maxKeepalive));
}

void Server_t::readRequest(DataBuilder_t &builder) {
    HTTPHeader_t headerIn;
    readRequest(builder, headerIn);
}

void Server_t::readRequest(DataBuilder_t &builder,
                           HTTPHeader_t &headerIn)
{
    closeConnection = false;
    contentLength = 0;
    headersSent = false;
    head = false;
    queryStorage.clear();
    queryStorage.push_back(std::string());
    queryStorage.back().reserve(BUFFER_SIZE + HTTP_BALLAST);
    headerIn = HTTPHeader_t();

    std::string protocol;
    std::string transferMethod;
    std::string contentType;
    std::string uriPath;
    std::auto_ptr<UnMarshaller_t> unmarshaller;
    //SocketCloser_t closer(httpIO.socket());

    //read hlavicku
    try
    {
        // ln je èíslo øádky
        // read all lines until first non-empty
        for (;;)
        {
            // read line from the socket, we check security limits for url
            std::string line(io.readLine(true));

            if (line.empty())
                continue;
            // break request line down
            std::vector<std::string> header(io.splitBySpace(line, 3));
            if (header.size() != 3)
            {
                // invalid request line
                // get rid of old method
                //lastMethod.erase();
                throw HTTPError_t(HTTP_BAD_REQUEST, "Bad HTTP request: '%s'.",
                                  line.substr(0, 30).c_str());
            }
            protocol =  header[2];
            // save request line parts
            if(protocol != "HTTP/1.1" && protocol != "HTTP/1.0")
            {
                throw HTTPError_t(HTTP_HTTP_VERSION_NOT_SUPPORTED,
                                  "Bad HTTP protocol version or type: '%s'.",
                                  header[2].c_str());
            }

            uriPath = header[1];

//             if (!path.empty())
//             {
//                if (uriPath != path)
//                {
//                   throw HTTPError_t(HTTP_NOT_FOUND,
//                                   "Uri not found '%s'.",
//                                   header[1].c_str());
//                }

//             }

            transferMethod =  header[0];


            /*std::istringstream is(header[1].c_str());
            int status;
            is >> status;
            if(status != 200)
            {
                throw HTTPError_t(status, "%s",header[2].c_str());
            }*/

            break;
        }

        // read header from the request
        io.readHeader(headerIn);

        if(transferMethod != "POST")
        {

            if(transferMethod == "HEAD")
            {
                head = true;
                closeConnection = true;
                return;
            }
            else
            {
                throw HTTPError_t(HTTP_METHOD_NOT_ALLOWED,
                                  "Method Not Allowed");
            }
        }


        // get content type from header
        headerIn.get(HTTP_HEADER_CONTENT_TYPE, contentType);
        //create unmarshaller and datasink

        // what types is supported by client
        outType = XML_RPC;
        useChunks = false;

        std::string accept("");
        if (headerIn.get(HTTP_HEADER_ACCEPT, accept) == 0) {

            /* xmlrpc is default
            if (accept.find("text/xml") != std::string::npos) {
                outType = XML_RPC;
                useChunks = false;
            } */

            if (accept.find("application/x-frpc") != std::string::npos) {
                if (useBinary) {
                    outType = BINARY_RPC;
                    useChunks = true;
                }

            } else if (accept.find("application/json") != std::string::npos) {
                outType = JSON;
            } else if (accept.find("application/x-base64-frpc")
                       != std::string::npos) {
                outType = BASE64_RPC;
            }
        }

        // what type is request
        if (contentType.find("application/x-frpc") != std::string::npos) {
            unmarshaller = std::auto_ptr<UnMarshaller_t>(
                    UnMarshaller_t::create(
                            UnMarshaller_t::BINARY_RPC,
                            builder));

        } else if (contentType.find("text/xml") != std::string::npos) {
            unmarshaller = std::auto_ptr<UnMarshaller_t>(
                    UnMarshaller_t::create(
                            UnMarshaller_t::XML_RPC,
                            builder));

        } else if (contentType.find("application/x-www-form-urlencoded")
                   != std::string::npos)
        {
            unmarshaller = std::auto_ptr<UnMarshaller_t>(
                    UnMarshaller_t::create(
                            UnMarshaller_t::URL_ENCODED,
                            builder,
                            uriPath));

        } else if (contentType.find("application/x-base64-frpc")
                  != std::string::npos)
        {
            unmarshaller = std::auto_ptr<UnMarshaller_t>(
                    UnMarshaller_t::create(
                            UnMarshaller_t::BASE64,
                            builder));

        } else {
            throw StreamError_t("Unknown ContentType");
        }

        DataSink_t data(*unmarshaller, UnMarshaller_t::TYPE_METHOD_CALL);

        // read body of request
        io.readContent(headerIn, data, true);

        unmarshaller->finish();
        protocolVersion = unmarshaller->getProtocolVersion();

        std::string connection;
        headerIn.get("Connection", connection);
        std::transform(connection.begin(), connection.end(),
                       connection.begin(), std::ptr_fun<int, int>(toupper));
        closeConnection = false;

        if (protocol == "HTTP/1.1") {
            closeConnection = (connection == "CLOSE");
        } else {
            closeConnection = (connection != "KEEP-ALIVE");
            useChunks = false;
        }
    } catch (const Error_t &) {
        // get rid of old method
        throw;
    }
}

void Server_t::write(const char* data, unsigned int size) {
    contentLength += size;
    if (size > BUFFER_SIZE - queryStorage.back().size()) {
        if (useChunks) {
            sendResponse();
            queryStorage.back().append(data, size);
        } else {
            if (size > BUFFER_SIZE) {
                queryStorage.push_back(std::string(data, size));
            } else {
                queryStorage.back().append(data, size);
            }
        }
    } else {
        queryStorage.back().append(data, size);
    }
}

void Server_t::flush() {
    if (!useChunks) {
        sendResponse();
    } else {
        sendResponse(true);
    }
}

void Server_t::sendHttpError(const HTTPError_t &httpError) {
    StreamHolder_t os;
    //create header
    os.os << "HTTP/1.1" << ' '
          << httpError.errorNum() << ' ' << httpError.message() << "\r\n";
    os.os << HTTP_HEADER_ACCEPT
          << ": " << "text/xml";
    if (useBinary)
        os.os << ", application/x-frpc";
    os.os << ", application/x-www-form-urlencoded";
    os.os << "\r\n";
    os.os << "Server:" << " Fast-RPC  Server Linux\r\n";

    // terminate header
    // append separator
    os.os << "\r\n";
    // send header
    io.sendData(os.os.str());
}

void Server_t::sendResponse(bool last) {
    std::string headerData;
    if(!headersSent) {
        StreamHolder_t os;


        //create header
        os.os << "HTTP/1.1" << ' ' << "200" << ' ' << "OK" << "\r\n";

        //os.os << "Host: " << url.host << ':' << url.port << "\r\n";
        switch (outType) {
        case XML_RPC:
            {

                os.os << HTTP_HEADER_CONTENT_TYPE
                      << ": " << "text/xml"<< "\r\n";
            }
            break;
        case BINARY_RPC:
            {
                os.os << HTTP_HEADER_CONTENT_TYPE
                      << ": " << "application/x-frpc"<< "\r\n";
            }
            break;

        case JSON:
            {
                os.os << HTTP_HEADER_CONTENT_TYPE
                      << ": " << "application/json"<< "\r\n";
            }
            break;

        case BASE64_RPC:
            {
                os.os << HTTP_HEADER_CONTENT_TYPE
                      << ": " << "application/x-base64-frpc"<< "\r\n";
            }
            break;

        default:
            throw StreamError_t("Unknown protocol");
            break;

        }

        os.os << HTTP_HEADER_ACCEPT
              << ": " << "text/xml";
        if (useBinary)
            os.os << ", application/x-frpc";
        os.os << ", application/x-www-form-urlencoded";
        os.os << ", application/x-base64-frpc";
        os.os << "\r\n";

        //append connection header
        os.os << HTTP_HEADER_CONNECTION
              << (keepAlive ? ": keep-alive" : ": close")
              << "\r\n";

        // write content-length or content-transfer-encoding when we can send
        // content

        if (!useChunks) {
            os.os << HTTP_HEADER_CONTENT_LENGTH << ": " << contentLength
            << "\r\n";
        } else {
            os.os << HTTP_HEADER_TRANSFER_ENCODING << ": chunked\r\n";
        }

        os.os << "Server:" << " Fast-RPC  Server Linux\r\n";

        if (headerOut) {
            os.os << *headerOut;
            // for security reset the header ptr => next call can come without
            // http out headers then it can point to somewhere in memory...
            headerOut = 0x0;
        }

        // terminate header
        // append separator
        os.os << "\r\n";


        // send header
        //io.sendData(os.os.str());
        headerData = os.os.str();
        //headersSent = true;

        if (head) {
            // send header
            io.sendData(os.os.str());
            return;
        }
    }

    if(useChunks) {
        // write chunk size
        StreamHolder_t os;
        os.os << std::hex << queryStorage.back().size() << "\r\n";
        //io.sendData(os.os.str());
        if (!headersSent){
            headerData.append(os.os.str());
            queryStorage.back().insert(0,headerData);
            headersSent = true;
        } else {
            queryStorage.back().insert(0,os.os.str());
        }

        //add chunk terminator
        if (last){
            queryStorage.back().append("\r\n0\r\n\r\n");
        } else {
            queryStorage.back().append("\r\n");
        }

        // write chunk
        io.sendData(queryStorage.back().data(),queryStorage.back().size() );
        queryStorage.back().erase();


    } else {
        if (!headersSent){
            queryStorage.front().insert(0,headerData);
            headersSent = true;
        }
        while(queryStorage.size() != 1) {
            io.sendData(queryStorage.front().data(),
                        queryStorage.front().size() );
            queryStorage.pop_front();
        }
        io.sendData(queryStorage.back().data(),queryStorage.back().size() );
        queryStorage.back().erase();
    }
}

}
