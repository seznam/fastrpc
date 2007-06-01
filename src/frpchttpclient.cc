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
 * FILE          $Id: frpchttpclient.cc,v 1.7 2007-06-01 13:44:53 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */

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
namespace
{


bool canReadBody(const std::string &method, int response)
{
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

inline bool canSendBody(const std::string &method)
{
    return (method == "POST") || (method == "PUT");
};

struct SocketCloser_t
{
    SocketCloser_t(int &fd)
            : fd(fd), doClose(true)
    {}

    ~SocketCloser_t()
    {
        if (doClose && (fd > -1))
        {
            ::close(fd);
            fd = -1;
        }
    }

    int &fd;
    bool doClose;
};

void connectSocket(int &fd, bool keepAlive, unsigned int connectTimeout,
                   const struct in_addr &ipaddr,
                   unsigned short int port)
{
    // zkontrolujeme socket a pøípadnì jej zavøeme
    if (!keepAlive && (fd > -1))
    {
        ::close(fd);
        fd = -1;
    }

    // initialize closer (initially closes socket when valid)
    SocketCloser_t closer(fd);

    // pokud je socket otevøený, zjistíme, jestli nám server nezavøel spojení
    if (fd > -1)
    {
        closer.doClose = false;

        // zjistíme, jestli se na socketu nìco nedìje
        fd_set rfdset;
        FD_ZERO(&rfdset);
        FD_SET(fd, &rfdset);
        // okam¾itý timeout
        struct timeval timeout =
            {
                0, 0
            };
        switch (::select(fd + 1, &rfdset, NULL, NULL, &timeout))
        {
        case 0:
            // OK
            break;

        case -1:
            // chyba na socketu -> zavøeme jej
            close(fd);
            fd = -1;
            break;

        default:
            // podíváme se, jestli jsou na socketu k dispozici nìjaká data
            char buff;
            switch (recv(fd, &buff, 1, MSG_PEEK))
            {
            case -1:
            case 0:
                // zavøeme socket
                close(fd);
                fd = -1;
                break;

            default:
                // OK
                break;
            }
        }
    }

    // pokud nemáme od minula otevøený socket
    if (fd < 0)
    {
        // otevøeme socket
        if ((fd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0)
        {
            // oops! error
            throw HTTPError_t(HTTP_SYSCALL,
                              "Cannot select on socketr: <%d, %s>.",
                              ERRNO, STRERROR(ERRNO));
        }

#ifdef WIN32
        unsigned int flag = 1;
        if (::ioctlsocket((SOCKET)fd, FIONBIO, &flag) < 0)
#else //WIN32

        if (::fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
#endif //WIN32

        {
            throw HTTPError_t(HTTP_SYSCALL,
                              "Cannot set socket non-blocking: "
                              "<%d, %s>.", ERRNO, STRERROR(ERRNO));
        }

        // nastavíme okam¾ité odesílání packetù po TCP
        // disabluje Nagle algoritmus, zdrojáky Apache øíkají, ¾e je to OK
        int just_say_no = 1;
        if (::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                         (char*) &just_say_no, sizeof(int)) < 0)
        {
            throw HTTPError_t(HTTP_SYSCALL,
                              "Cannot set socket non-delaying: "
                              "<%d, %s>.", ERRNO, STRERROR(ERRNO));
        }

        // adresa protìj¹í strany
        struct sockaddr_in addr;

        // inicializujeme adresu
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr = ipaddr;
        memset(addr.sin_zero, 0x0, 8);

        // spojíme socket s protìj¹í stranou
        if (::connect(fd, (struct sockaddr *) &addr,
                      sizeof(struct sockaddr)) < 0 )
        {
            switch (ERRNO)
            {
            case EINPROGRESS:
                // connect probíhá
            case EALREADY:
                // connect jsme u¾ jednou zavolali
                // tyto chyby nepova¾ujeme za chyby
            case EWOULDBLOCK:
                break;

            default:
                throw HTTPError_t(HTTP_SYSCALL,
                                  "Cannot connect socket: <%d, %s>.",
                                  ERRNO, STRERROR(ERRNO));
            }

            // vyrobíme a inicializujeme mno¾inu deskriptorù
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(fd, &wfds);

            // vytvoøíme timeout
            struct timeval timeout =
                {
                    connectTimeout / 1000,
                    (connectTimeout % 1000) * 1000
                };

            // zjistíme, jestli connect dobìhl
            int ready = ::select(fd + 1, NULL, &wfds, NULL,
                                 (connectTimeout < 0)
                                 ? NULL : &timeout);

            if (ready <= 0)
            {
                switch (ready)
                {
                case 0:
                    throw HTTPError_t(HTTP_SYSCALL, "Timeout while connecting.");

                default:
                    throw HTTPError_t(HTTP_SYSCALL,
                                      "Cannot select on socket: <%d, %s>.",
                                      ERRNO, STRERROR(ERRNO));
                }
            }

            // zjistíme, jak dobìhl connect
            socklen_t len = sizeof(int);
            int status;
#ifdef WIN32

            if (::getsockopt(fd, SOL_SOCKET, SO_ERROR,
                             (char*)&status, &len))
#else //WIN32

            if (::getsockopt(fd, SOL_SOCKET, SO_ERROR,
                             &status, &len))
#endif //WIN32

            {
                throw HTTPError_t(HTTP_SYSCALL,
                                  "Cannot get socket info: <%d, %s>.",
                                  ERRNO, STRERROR(ERRNO));
            }
            if (status)
            {
                throw HTTPError_t(HTTP_SYSCALL,
                                  "Cannot connect socket: <%d, %s>.",
                                  status, STRERROR(status));
            }
        }

        // connect OK => do not close socket!
        closer.doClose = false;
    }
}





}


namespace FRPC
{

HTTPClient_t::HTTPClient_t(HTTPIO_t &httpIO, URL_t &url, unsigned int connectTimeout,
                           bool keepAlive)
        : httpIO(httpIO), url(url), connectTimeout(connectTimeout),keepAlive(keepAlive),
        headersSent(false),useChunks(false),supportedProtocols(XML_RPC),
        useProtocol(XML_RPC),contentLenght(0),connectionMustClose(false),
        unmarshaller(0), useHTTP10(false)
{
    queryStorage.push_back(std::string(""));
    queryStorage.back().reserve(BUFFER_SIZE);

}

HTTPClient_t::HTTPClient_t(HTTPIO_t &httpIO, URL_t &url, unsigned int connectTimeout,
                           bool keepAlive, bool useHTTP10)
        : httpIO(httpIO), url(url), connectTimeout(connectTimeout),keepAlive(keepAlive),
        headersSent(false),useChunks(false),supportedProtocols(XML_RPC),
        useProtocol(XML_RPC),contentLenght(0),connectionMustClose(false),
        unmarshaller(0), useHTTP10(useHTTP10)
{
    queryStorage.push_back(std::string(""));
    queryStorage.back().reserve(BUFFER_SIZE);

}


HTTPClient_t::~HTTPClient_t()
{

    if(unmarshaller)
        delete unmarshaller;

}


void HTTPClient_t::flush()
{
    if(!useChunks)
    {
        sendRequest();

    }
    else
    {
        sendRequest();
        try
        {
            httpIO.sendData("0\r\n\r\n", 5,true);
        }
        catch(const ResponseError_t &e)
        {
            connectionMustClose = true;
            throw   ResponseError_t();
        }
    }

}

void HTTPClient_t::write(const char* data, unsigned int size)
{

    contentLenght += size;

    if(size > BUFFER_SIZE - queryStorage.back().size())
    {
        if(useChunks)
        {

            sendRequest();

            queryStorage.back().append(data, size);
        }
        else
        {
            if(size > BUFFER_SIZE)
            {
                queryStorage.push_back(std::string(data, size));
            }
            else
            {
                queryStorage.back().append(data, size);
            }
        }
    }
    else
    {
        queryStorage.back().append(data, size);
    }


}

void HTTPClient_t::readResponse(DataBuilder_t &builder)
{


    HTTPHeader_t httpHead;
    std::string protocol;
    std::string contentType;
    SocketCloser_t closer(httpIO.socket());
    bool closeConnection = false;
    //read hlavicku
    try
    {
        // ln je èíslo øádky
        // read all lines until first non-empty
        for (;;)
        {
            // read line from the socket, we check security limits for url
            std::string line(httpIO.readLine(true));

            if (line.empty())
                continue;
            // break request line down
            std::vector<std::string> header(httpIO.splitBySpace(line, 3));
            if (header.size() != 3)
            {
                // invalid request line
                // get rid of old method
                //lastMethod.erase();
                throw HTTPError_t(HTTP_VALUE, "Bad HTTP request: '%s'.",
                                  line.substr(0, 30).c_str());
            }
            protocol =  header[0];
            // save request line parts
            if((protocol != "HTTP/1.0") && (protocol != "HTTP/1.1"))
            {
                throw HTTPError_t(HTTP_VALUE, "Bad HTTP protocol version or type: '%s'.",
                                  header[0].c_str());
            }
            std::istringstream is(header[1].c_str());
            int status;
            is >> status;
            if(status != 200)
            {
                throw HTTPError_t(status, "%s",header[2].c_str());
            }

            break;
        }

        // read header from the request
        httpIO.readHeader(httpHead);

        // get content type from header
        httpHead.get(HTTP_HEADER_CONTENT_TYPE, contentType );
        //create unmarshaller and datasink

        //ake typy podporuje server ?
        std::string accept("");
        if( httpHead.get(HTTP_HEADER_ACCEPT, accept) == 0)
        {
            supportedProtocols = 0;

            if(accept.find("text/xml") != std::string::npos)
            {
                supportedProtocols |= XML_RPC;
            }

            if(accept.find("application/x-frpc") != std::string::npos)
            {
                supportedProtocols |= BINARY_RPC;
            }
        }

        if(unmarshaller)
            delete unmarshaller;

        if(contentType.find("text/xml") != std::string::npos )
        {
            unmarshaller = UnMarshaller_t::create(UnMarshaller_t::XML_RPC, builder);
        }
        else if (contentType.find("application/x-frpc")!= std::string::npos )
        {
            unmarshaller = UnMarshaller_t::create(UnMarshaller_t::BINARY_RPC, builder);
        }

        else
        {
            throw StreamError_t("Unknown ContentType");
        }



        DataSink_t data(*unmarshaller);



        // read body of response
        httpIO.readContent(httpHead, data, false);

        unmarshaller->finish();


        std::string connection;
        httpHead.get("Connection", connection);
        std::transform(connection.begin(), connection.end(),
                       connection.begin(),std::ptr_fun<int, int>(toupper));
        //bool closeConnection = false;

        if (protocol == "HTTP/1.1")
        {
            closeConnection = (connection == "CLOSE");
        }
        else
        {
            closeConnection = (connection != "KEEP-ALIVE");
        }




    }
    catch (const Error_t &)
    {
        // get rid of old method

        throw;
    }

    // close socket
    if (!((!keepAlive || closeConnection || connectionMustClose) && (httpIO.socket() > -1)))
    {
        closer.doClose = false;
    }



}

void HTTPClient_t::sendRequest()
{



    SocketCloser_t closer(httpIO.socket());



    if(!headersSent)
    {
        HTTPHeader_t header;
        StreamHolder_t os;


        //create header
        os.os << "POST" << ' ' << url.path << ' '
        << (useHTTP10 ? "HTTP/1.0" : "HTTP/1.1") << "\r\n";

        os.os << "Host: " << url.host << ':' << url.port << "\r\n";
        switch(useProtocol)
        {
        case XML_RPC:
            {

                os.os << HTTP_HEADER_CONTENT_TYPE << ": " << "text/xml"<< "\r\n";
            }
            break;
        case BINARY_RPC:
            {
                os.os << HTTP_HEADER_CONTENT_TYPE << ": " << "application/x-frpc"<< "\r\n";
            }
            break;

        default:
            throw StreamError_t("Unknown protocol");
            break;

        }

        os.os  << HTTP_HEADER_ACCEPT << ": " << "text/xml, application/x-frpc"<< "\r\n";

        //append connection header
        os.os << HTTP_HEADER_CONNECTION << (keepAlive ? ": keep-alive" : ": close")
        << "\r\n";

        // write content-length or content-transfer-encoding when we can send
        // content

        if (!useChunks)
        {
            os.os << HTTP_HEADER_CONTENT_LENGTH << ": " << contentLenght
            << "\r\n";
        }
        else
        {
            os.os << HTTP_HEADER_TRANSFER_ENCODING << ": chunked\r\n";
        }


        // terminate header
        // append separator
        os.os << "\r\n";


        if(connectionMustClose)
        {
            if( httpIO.socket() > -1)
            {
                close(httpIO.socket());
                httpIO.socket() = -1;
            }

            connectionMustClose = false;
        }

        //connect socket
        connectSocket(httpIO.socket(), keepAlive, connectTimeout, url.addr, url.port);
        try
        {
            // send header
            httpIO.sendData(os.os.str());
        }
        catch(const ResponseError_t &e)
        {
            connectionMustClose = true;
            closer.doClose = false;
            throw ResponseError_t();
        }
        headersSent = true;

    }

    try
    {
        if(useChunks)
        {
            // write chunk size
            StreamHolder_t os;
            os.os << std::hex << queryStorage.back().size() << "\r\n";
            httpIO.sendData(os.os.str(),true);
            // write chunk
            httpIO.sendData(queryStorage.back().data(),queryStorage.back().size(),true );
            // write chunk terminator
            httpIO.sendData("\r\n", 2,true);
            queryStorage.back().erase();


        }
        else
        {
            while(queryStorage.size() != 0)
            {
                httpIO.sendData(queryStorage.back().data(),queryStorage.back().size(),true );
                queryStorage.pop_back();
            }

        }
    }
    catch(const ResponseError_t &e)
    {
        connectionMustClose = true;
        closer.doClose = false;
        throw ResponseError_t();
    }

    closer.doClose = false;


    //throw ResponseError_t();
}

}
;
