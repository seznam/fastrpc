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
 * FILE          $Id: frpchttpclient.cc,v 1.14 2010-05-10 08:39:33 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include "nonglibc.h"

#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>

#include <string>
#include <algorithm>
#include <functional>
#include <sstream>

#include <stdexcept>


#include "frpcsocket.h"
#include "frpchttpclient.h"
#include <frpchttperror.h>
#include <frpcunmarshaller.h>
#include <frpchttp.h>
#include <frpcstreamerror.h>
#include <frpcinternals.h>
#include <frpcresponseerror.h>
#include <frpc.h>


using namespace FRPC;

namespace {


bool canReadBody(const std::string &method, int response) {
    // HEAD cannot have any content
    if (method == "HEAD")
        return false;

    // CONNECT cannot have body on success
    if (method == "CONNECT")
        return (response != 200);

    // 100 Continue => no content
    if (response == 100)
        return false;

    // OK we can have body
    return true;
};

inline bool canSendBody(const std::string &method) {
    return (method == "POST") || (method == "PUT");
};

struct SocketCloser_t {
    SocketCloser_t(int &fd)
        : fd(fd), doClose(true)
    {}

    ~SocketCloser_t() {
        if (doClose && (fd > -1)) {
            TEMP_FAILURE_RETRY(::close(fd));
            fd = -1;
        }
    }

    int &fd;
    bool doClose;
};

// just replaces newline whitespace characters with spaces
std::string cleanup_header_value(const std::string &input) {
    std::string clensed;
    clensed.reserve(input.size());
    for (std::string::const_iterator ic = input.begin(),
                                   iend = input.end();
         ic != iend;
         ++ic)
    {
        switch (*ic) {
        case '\r':
        case '\n':
            clensed.push_back(' ');
            break;
        default:
            clensed.push_back(*ic);

        }
    }
    return clensed;
}

} // namespace

namespace FRPC {

HTTPClient_t::HTTPClient_t(HTTPIO_t &httpIO, URL_t &url,
                           Connector_t *connector, bool useHTTP10)
    : httpIO(httpIO), url(url), connector(connector),
      headersSent(false), useChunks(false), supportedProtocols(XML_RPC),
      useProtocol(XML_RPC), contentLenght(0), connectionMustClose(false),
      unmarshaller(0), useHTTP10(useHTTP10)
{
    queryStorage.push_back(std::string());
    queryStorage.back().reserve(BUFFER_SIZE + HTTP_BALLAST);
}


HTTPClient_t::~HTTPClient_t() {
    delete unmarshaller;
}


void HTTPClient_t::flush() {
    if (!useChunks) {
        sendRequest();
    } else {
        //append empty chunk to data

        sendRequest(true);
   /*     try {
            httpIO.sendData("0\r\n\r\n", 5, true);
        } catch(const ResponseError_t &e) {
            connectionMustClose = true;
            throw;
        }*/
    }
}

void HTTPClient_t::write(const char* data, unsigned int size) {
    contentLenght += size;

    if (size > (BUFFER_SIZE - queryStorage.back().size())) {
        if (useChunks) {
            sendRequest();
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

void HTTPClient_t::readResponse(DataBuilder_t &builder) {
    HTTPHeader_t httpHead;
    std::string protocol;
    std::string contentType;
    SocketCloser_t closer(httpIO.socket());
    bool closeConnection = false;

    // read header
    try {
        // ln is line number
        // read all lines until first non-empty
        for (;;) {
            // read line from the socket, we check security limits for url
            std::string line(httpIO.readLine(true));

            if (line.empty())
                continue;
            // break request line down
            std::vector<std::string> header(httpIO.splitBySpace(line, 3));
            if (header.size() != 3) {
                // invalid request line
                // get rid of old method
                //lastMethod.erase();
                throw HTTPError_t(HTTP_VALUE, "Bad HTTP request: '%s'.",
                                  line.substr(0, 30).c_str());
            }

            protocol =  header[0];
            // save request line parts
            if ((protocol != "HTTP/1.0") && (protocol != "HTTP/1.1")) {
                throw HTTPError_t(HTTP_VALUE,
                                  "Bad HTTP protocol version or type: '%s'.",
                                  header[0].c_str());
            }

            std::istringstream is(header[1].c_str());
            int status;
            is >> status;

            if (status != 200) {
                throw HTTPError_t(status, "%s",header[2].c_str());
            }

            break;
        }

        // read header from the request
        httpIO.readHeader(httpHead);

        // get content type from header
        httpHead.get(HTTP_HEADER_CONTENT_TYPE, contentType);
        //create unmarshaller and datasink

        // what content-types are supported by server?
        std::string accept("");
        if (httpHead.get(HTTP_HEADER_ACCEPT, accept) == 0) {
            supportedProtocols = 0;

            if (accept.find("text/xml") != std::string::npos) {
                supportedProtocols |= XML_RPC;
            }

            if (accept.find("application/x-frpc") != std::string::npos) {
                supportedProtocols |= BINARY_RPC;
            }
        }

        delete unmarshaller;

        if (contentType.find("text/xml") != std::string::npos) {
            unmarshaller = UnMarshaller_t::create(UnMarshaller_t::XML_RPC,
                                                  builder);
        } else if (contentType.find("application/x-frpc")
                   != std::string::npos) {
            unmarshaller = UnMarshaller_t::create(UnMarshaller_t::BINARY_RPC,
                                                  builder);
        } else {
            throw StreamError_t("Unknown ContentType");
        }

        DataSink_t data(*unmarshaller);

        // read body of response
        httpIO.readContent(httpHead, data, false);

        unmarshaller->finish();
        protocolVersion = unmarshaller->getProtocolVersion();

        std::string connection;
        httpHead.get("Connection", connection);
        std::transform(connection.begin(), connection.end(),
                       connection.begin(),std::ptr_fun<int, int>(toupper));
        //bool closeConnection = false;

        if (protocol == "HTTP/1.1") {
            closeConnection = (connection == "CLOSE");
        } else {
            closeConnection = (connection != "KEEP-ALIVE");
        }
    } catch (const Error_t &) {
        // get rid of old method
        throw;
    }

    // close socket
    if (!((!connector->getKeepAlive() || closeConnection || connectionMustClose)
          && (httpIO.socket() > -1))) {
        closer.doClose = false;
    }
}

void HTTPClient_t::sendRequest(bool last) {
    SocketCloser_t closer(httpIO.socket());

    std::string headerData;

    if (!headersSent) {
        HTTPHeader_t header;
        StreamHolder_t os;

        //create header
        os.os << "POST" << ' ' << url.path << ' '
        << (useHTTP10 ? "HTTP/1.0" : "HTTP/1.1") << "\r\n";

        os.os << "Host: " << url.host << ':' << url.port << "\r\n";
        switch(useProtocol) {
        case XML_RPC:
            {
                os.os << HTTP_HEADER_CONTENT_TYPE << ": "
                      << "text/xml"<< "\r\n";
            }
            break;
        case BINARY_RPC:
            {
                os.os << HTTP_HEADER_CONTENT_TYPE << ": "
                      << "application/x-frpc"<< "\r\n";
            }
            break;

        default:
            throw StreamError_t("Unknown protocol");
            break;

        }

        os.os  << HTTP_HEADER_ACCEPT << ": "
               << "text/xml, application/x-frpc"<< "\r\n";

        //append connection header
        os.os << HTTP_HEADER_CONNECTION
              << (connector->getKeepAlive() ? ": keep-alive" : ": close")
        << "\r\n";

        if (!forward.empty()) {
            os.os << HTTP_HEADER_X_FORWARDED_FOR << ": "
                  << cleanup_header_value(forward) << "\r\n";
        }

        // write content-length or content-transfer-encoding when we can send
        // content

        if (!useChunks) {
            os.os << HTTP_HEADER_CONTENT_LENGTH << ": " << contentLenght
                  << "\r\n";
        } else {
            os.os << HTTP_HEADER_TRANSFER_ENCODING << ": chunked\r\n";
        }

        // terminate header
        // append separator
        os.os << "\r\n";

        if (connectionMustClose) {
            if (httpIO.socket() > -1) {
                TEMP_FAILURE_RETRY(close(httpIO.socket()));
                httpIO.socket() = -1;
            }

            connectionMustClose = false;
        }

        //connect socket
        connector->connectSocket(httpIO.socket());

       /* try {
            // send header
            httpIO.sendData(os.os.str());
        } catch(const ResponseError_t &e) {
            connectionMustClose = true;
            closer.doClose = false;
            throw ResponseError_t();
        }*/
        headerData = os.os.str();
       // headersSent = true;
    }

    try {
        if (useChunks) {
            // write chunk size
            StreamHolder_t os;
            os.os << std::hex << queryStorage.back().size() << "\r\n";
            //httpIO.sendData(os.os.str(), true);
            if (!headersSent){
                headerData.append(os.os.str());
                queryStorage.back().insert(0,headerData);
                headersSent = true;
            }else{
                queryStorage.back().insert(0,os.os.str());
            }
            // add chunk terminator to data
            if (last){
                queryStorage.back().append("\r\n0\r\n\r\n");
            }else{
                queryStorage.back().append("\r\n");
            }            

            // write chunk
            httpIO.sendData(queryStorage.back().data(),
                            queryStorage.back().size(), true);

            queryStorage.back().erase();
        } else {
            if (!headersSent){
                queryStorage.front().insert(0,headerData);
                headersSent = true;
            }

            while (queryStorage.size() != 1) {

                httpIO.sendData(queryStorage.front().data(),
                                queryStorage.front().size(), true);
                queryStorage.pop_front();
            }
            httpIO.sendData(queryStorage.back().data(),queryStorage.back().size() );
            queryStorage.back().erase();

        }
    } catch(const ResponseError_t &e) {
        connectionMustClose = true;
        closer.doClose = false;
        throw ResponseError_t();
    }

    closer.doClose = false;
}

} // namespace FRPC
