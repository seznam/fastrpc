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

template <typename T>
inline void addHeader(StreamHolder_t& stream, const std::string& name, const T& value) {
        stream.os << name << ": " << value << "\r\n";
    }

} // namespace

namespace FRPC {

const std::string HTTPClient_t::HOST = "HOST";
const std::string HTTPClient_t::POST = "POST";
const std::string HTTPClient_t::HTTP10 = "HTTP/1.0";
const std::string HTTPClient_t::HTTP11 = "HTTP/1.1";
const std::string HTTPClient_t::TYPE_XML = "text/xml";
const std::string HTTPClient_t::TYPE_FRPC = "application/x-frpc";
const std::string HTTPClient_t::ACCEPTED = "text/xml, application/x-frpc";
const std::string HTTPClient_t::CLOSE = "close";
const std::string HTTPClient_t::KEEPALIVE = "keep-alive";

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

        sendRequest(true);
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
                throw HTTPError_t::format(
                        HTTP_VALUE, "Bad HTTP request: '%s'.",
                        line.substr(0, 30).c_str());
            }

            protocol =  header[0];
            // save request line parts
            if ((protocol != "HTTP/1.0") && (protocol != "HTTP/1.1")) {
                throw HTTPError_t::format(
                        HTTP_VALUE,
                        "Bad HTTP protocol version or type: '%s'.",
                        header[0].c_str());
            }

            std::istringstream is(header[1].c_str());
            int status;
            is >> status;

            if (status != 200) {
                // Note: Was a %s format of the c_str, without any other text?!
                throw HTTPError_t(status, header[2]);
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

            if (accept.find(TYPE_XML) != std::string::npos) {
                supportedProtocols |= XML_RPC;
            }

            if (accept.find(TYPE_FRPC) != std::string::npos) {
                supportedProtocols |= BINARY_RPC;
            }
        }

        delete unmarshaller;

        if (contentType.find(TYPE_XML) != std::string::npos) {
            unmarshaller = UnMarshaller_t::create(UnMarshaller_t::XML_RPC, builder);
        } else if (contentType.find(TYPE_FRPC) != std::string::npos) {
            unmarshaller = UnMarshaller_t::create(UnMarshaller_t::BINARY_RPC, builder);
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

        if (protocol == HTTP11) {
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
        os.os << POST << " " << url.path << ' ' << (useHTTP10 ? HTTP10 : HTTP11) << "\r\n";
        os.os << HOST << ": " << url.host << ':' << url.port << "\r\n";

        switch(useProtocol) {
        case XML_RPC:
            addHeader(os, HTTP_HEADER_CONTENT_TYPE, TYPE_XML);
            break;
        case BINARY_RPC:
            addHeader(os, HTTP_HEADER_CONTENT_TYPE, TYPE_FRPC);
            break;
        default:
            throw StreamError_t("Unknown protocol");
            break;

        }

        addHeader(os, HTTP_HEADER_ACCEPT, ACCEPTED);

        //append connection header
        addHeader(os, HTTP_HEADER_CONNECTION, (connector->getKeepAlive() ? KEEPALIVE : CLOSE));

        // write content-length or content-transfer-encoding when we can send
        // content

        if (!useChunks) {
            addHeader(os, HTTP_HEADER_CONTENT_LENGTH, contentLenght);
        } else {
            addHeader(os, HTTP_HEADER_TRANSFER_ENCODING, "chunked");
        }

        // add custom headers
        os.os << m_customRequestHeaders.str();

        // terminate header
        os.os << "\r\n";

        if (connectionMustClose) {
            if (httpIO.socket() > -1) {
                TEMP_FAILURE_RETRY(close(httpIO.socket()));
                httpIO.socket() = -1;
            }

            connectionMustClose = false;
        }

        connector->connectSocket(httpIO.socket());

        headerData = os.os.str();
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

template <typename T>
void HTTPClient_t::addCustomRequestHeader(const std::string& name, const T& value) {
    m_customRequestHeaders << name << ": " << value << "\r\n";
}

void HTTPClient_t::addCustomRequestHeader(const Header_t& header) {
    addCustomRequestHeader(header.first, header.second);
}

void HTTPClient_t::addCustomRequestHeader(const HeaderVector_t& headers) {
    for (HeaderVector_t::const_iterator it=headers.begin(); it!=headers.end(); ++it) {
        addCustomRequestHeader(it->first, it->second);
    }
}

void HTTPClient_t::deleteCustomRequestHeaders() {
    m_customRequestHeaders.str(std::string());
}

} // namespace FRPC
