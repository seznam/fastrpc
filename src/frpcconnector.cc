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
 * FILE          $Id: frpcconnector.cc,v 1.7 2011-02-11 08:56:17 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Vaclav Blazek <vaclav.blazek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include "nonglibc.h"

#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

#ifndef UNIX_PATH_MAX
# define UNIX_PATH_MAX   108
#endif

#include "frpcplatform.h"
#include "frpcconnector.h"
#include "frpchttperror.h"
#include "frpcsocket.h"

using namespace FRPC;

Connector_t::Connector_t(const URL_t &url, int connectTimeout,
                         bool keepAlive)
    : url(url), connectTimeout(connectTimeout), keepAlive(keepAlive)
{}

Connector_t::~Connector_t() {}

namespace {
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

        void release() {
            doClose = false;
        }

        int &fd;
        bool doClose;
    };

    const char *host_strerror(int num) {
        switch (num) {
        case HOST_NOT_FOUND:
            return "The specified host is unknown";

        case NO_ADDRESS:
            return "The requested name is valid but does "
                "not have an IP address.";

        case NO_RECOVERY:
            return "A non-recoverable name server error occurred.";

        case TRY_AGAIN:
            return "A temporary error occurred on an authoritative "
                "name server.  Try again later.";

        default:
            return "Unknown DNS related error.";
        }
    }

    void checkSocket(int &fd)
    {
        // check for connect status
        socklen_t len = sizeof(int);
        int status;
#ifdef WIN32
        if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&status, &len))
#else //WIN32
        if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &status, &len))
#endif //WIN32
        {
            STRERROR_PRE();
            throw HTTPError_t::format(
                    HTTP_SYSCALL, "Cannot get socket info: <%d, %s>.",
                    ERRNO, STRERROR(ERRNO));
        }

        // check for error
        if (status) {
            STRERROR_PRE();
            throw HTTPError_t::format(
                    HTTP_SYSCALL, "Cannot connect socket: <%d, %s>.",
                    status, STRERROR(status));
        }
    }

    void setNonBlockingSocket(int& fd)
    {
#ifdef WIN32
        unsigned int flag = 1;
        if (::ioctlsocket((SOCKET)fd, FIONBIO, &flag) < 0)
#else //WIN32
        if (::fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
#endif //WIN32
        {
            STRERROR_PRE();
            throw HTTPError_t::format(
                    HTTP_SYSCALL, "Cannot set socket non-blocking: <%d, %s>.",
                    ERRNO, STRERROR(ERRNO));
        }
    }

    void setNonDelayedSocket(int& fd)
    {
        // set not-delayed IO (we are not telnet)
        int just_say_no = 1;
        if (::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                         (char*) &just_say_no, sizeof(int)) < 0)
        {
            STRERROR_PRE();
            throw HTTPError_t::format(
                    HTTP_SYSCALL, "Cannot set socket non-delaying: <%d, %s>.",
                    ERRNO, STRERROR(ERRNO));
        }
    }

    void waitConnectSocket(int& fd, const URL_t& url, const int& connectTimeout)
    {
        // create poll struct
        pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLOUT;

        // wait for connect completion or timeout
        int ready = TEMP_FAILURE_RETRY(
                ::poll(&pfd, 1, (connectTimeout < 0) ? -1 : connectTimeout));

        if (ready <= 0) {
            switch (ready) {
            case 0:
                throw HTTPError_t::format(
                        HTTP_SYSCALL, "Timeout while connecting to %s.",
                        url.getUrl().c_str());
            default:
                STRERROR_PRE();
                throw HTTPError_t::format(
                        HTTP_SYSCALL, "Cannot select on socket: <%d, %s>.",
                        ERRNO, STRERROR(ERRNO));
            }
        }
    }

    void closeSocketIfPeerClosed(int& fd)
    {
        // check for activity on socket
        pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;

        // okam¾itý timeout
        switch (TEMP_FAILURE_RETRY(::poll(&pfd, 1, 0))) {
        case 0:
            // OK
            break;

        case -1:
            // some error on socket => close it
            TEMP_FAILURE_RETRY(::close(fd));
            fd = -1;
            break;

        default:
            // check whether any data can be read from the socket
            char buff;
            switch (TEMP_FAILURE_RETRY(recv(fd, &buff, 1, MSG_PEEK))) {
            case -1:
            case 0:
                // zavøeme socket
                TEMP_FAILURE_RETRY(::close(fd));
                fd = -1;
                break;

            default:
                // OK
                break;
            }
        }
    }

} // namespace

SimpleConnector_t::SimpleConnector_t(const URL_t &url, int connectTimeout,
                                     bool keepAlive)
    : Connector_t(url, connectTimeout, keepAlive)
{
    struct hostent h, *he;
    char tmpbuf[1024];
    int errcode;


#if (defined(sun) || defined(__sun)) && (defined(__SVR4) || defined(__svr4__))
    he = gethostbyname_r(url.host.c_str(), &h, tmpbuf, 1024, &errcode);
#else
    gethostbyname_r(url.host.c_str(), &h, tmpbuf, 1024, &he, &errcode);
#endif

    if (!he) {
        // oops
        throw HTTPError_t::format(HTTP_DNS,
                                  "Cannot resolve host '%s': <%d, %s>.",
                                  url.host.c_str(), errcode,
                                  host_strerror(errcode));
    }

    // remember IP address
    ipaddr = *reinterpret_cast<in_addr*>(he->h_addr_list[0]);
}

SimpleConnector_t::~SimpleConnector_t() {}

void SimpleConnector_t::connectSocket(int &fd) {
    // check socket
    if (!keepAlive && (fd > -1)) {
        TEMP_FAILURE_RETRY(::close(fd));
        fd = -1;
    }

    // initialize closer (initially closes socket when valid)
    SocketCloser_t closer(fd);

    // check open socket whether the peer had not closed it
    if (fd > -1) {
        closer.release();

        closeSocketIfPeerClosed(fd);
    }

    // if open socket is not availabe open new one
    if (fd < 0) {
        // otevøeme socket
        if ((fd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            // oops! error
            STRERROR_PRE();
            throw HTTPError_t::format(
                    HTTP_SYSCALL,
                    "Cannot select on socket: <%d, %s>.",
                    ERRNO, STRERROR(ERRNO));
        }

        setNonBlockingSocket(fd);

        setNonDelayedSocket(fd);

        // peer address
        struct sockaddr_in addr;

        // initialize it
        addr.sin_family = AF_INET;
        addr.sin_port = htons(url.port);
        addr.sin_addr = ipaddr;
        memset(addr.sin_zero, 0x0, 8);

        // connect the socket
        if (TEMP_FAILURE_RETRY(::connect(fd, (struct sockaddr *)&addr,
                               sizeof(struct sockaddr))) < 0)
        {
            switch (ERRNO) {
            case EINPROGRESS:
                // connection already in progress
            case EALREADY:
                // connection already in progress
            case EWOULDBLOCK:
                // connection launched on the background
                break;

            default:
                STRERROR_PRE();
                throw HTTPError_t::format(
                        HTTP_SYSCALL,
                        "Cannot connect socket: <%d, %s>.",
                        ERRNO, STRERROR(ERRNO));
            }

            waitConnectSocket(fd, url, connectTimeout);

            checkSocket(fd);
        }

        // connect OK => do not close the socket!
        closer.release();
    }
}



SimpleConnectorIPv6_t::SimpleConnectorIPv6_t(const URL_t &url, int connectTimeout,
                                     bool keepAlive)
    : Connector_t(url, connectTimeout, keepAlive), addrInfo(0)
{
    int errcode;
    struct addrinfo hints;
    char port[8] = {0};

    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = 0;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    std::string host = url.host;
    if ( *host.begin() == '[' && *host.rbegin() == ']' ) {
        host = host.substr(1, host.size() - 2);
        hints.ai_family = AF_INET6;
    }

    snprintf(port, sizeof(port), "%u", url.port);
    errcode = getaddrinfo(host.c_str(), port, &hints, &addrInfo);
    if ( errcode != 0 ) {
        // oops
        throw HTTPError_t::format(
                HTTP_DNS, "Cannot resolve host '%s'('%s'): <%d, %s>.",
                url.host.c_str(), host.c_str(), errcode,
                gai_strerror(errcode));
    }

}

SimpleConnectorIPv6_t::~SimpleConnectorIPv6_t() {
    if ( addrInfo )
        freeaddrinfo(addrInfo);
}

void SimpleConnectorIPv6_t::connectSocket(int &fd) {
    // check socket
    if (!keepAlive && (fd > -1)) {
        TEMP_FAILURE_RETRY(::close(fd));
        fd = -1;
    }

    // initialize closer (initially closes socket when valid)
    SocketCloser_t closer(fd);

    // check open socket whether the peer had not closed it
    if (fd > -1) {
        closer.release();

        closeSocketIfPeerClosed(fd);
    }

    // if open socket is not availabe open new one
    if (fd < 0) {
        // otevøeme socket
        if ((fd = ::socket(addrInfo->ai_family, SOCK_STREAM, 0)) < 0) {
            // oops! error
            STRERROR_PRE();
            throw HTTPError_t::format(
                    HTTP_SYSCALL,
                    "Cannot select on socket: <%d, %s>.",
                    ERRNO, STRERROR(ERRNO));
        }

        setNonBlockingSocket(fd);

        setNonDelayedSocket(fd);

        // connect the socket
        if (TEMP_FAILURE_RETRY(::connect(fd, addrInfo->ai_addr,
                               addrInfo->ai_addrlen)) < 0)
        {
            switch (ERRNO) {
            case EINPROGRESS:
                // connection already in progress
            case EALREADY:
                // connection already in progress
            case EWOULDBLOCK:
                // connection launched on the background
                break;

            default:
                STRERROR_PRE();
                throw HTTPError_t::format(
                        HTTP_SYSCALL,
                        "Cannot connect socket: <%d, %s>.",
                        ERRNO, STRERROR(ERRNO));
            }

            waitConnectSocket(fd, url, connectTimeout);

            checkSocket(fd);
        }

        // connect OK => do not close the socket!
        closer.release();
    }
}

#ifndef WIN32

SimpleConnectorUnix_t::SimpleConnectorUnix_t(const URL_t &url, int connectTimeout,
                                     bool keepAlive)
    : Connector_t(url, connectTimeout, keepAlive)
{ }

SimpleConnectorUnix_t::~SimpleConnectorUnix_t()
{ }

void SimpleConnectorUnix_t::connectSocket(int &fd) {
    // check socket
    if (!keepAlive && (fd > -1)) {
        TEMP_FAILURE_RETRY(::close(fd));
        fd = -1;
    }

    // initialize closer (initially closes socket when valid)
    SocketCloser_t closer(fd);

    // check open socket whether the peer had not closed it
    if (fd > -1) {
        closer.release();

        closeSocketIfPeerClosed(fd);
    }

    // if open socket is not availabe open new one
    if (fd < 0) {
        // otevøeme socket
        if ((fd = ::socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
            // oops! error
            STRERROR_PRE();
            throw HTTPError_t::format(
                    HTTP_SYSCALL,
                    "Cannot select on socket: <%d, %s>.",
                    ERRNO, STRERROR(ERRNO));
        }

        setNonBlockingSocket(fd);

        struct sockaddr_un remote;
        remote.sun_family = AF_UNIX;
        strncpy(remote.sun_path, url.path.c_str(), UNIX_PATH_MAX);
        remote.sun_path[UNIX_PATH_MAX-1] = 0;

        // connect the socket
        if (TEMP_FAILURE_RETRY(::connect(fd, (struct sockaddr *)&remote,
                               sizeof(remote))) < 0)
        {
            switch (ERRNO) {
            case EINPROGRESS:
                // connection already in progress
            case EALREADY:
                // connection already in progress
            case EWOULDBLOCK:
                // connection launched on the background
                break;
            default:
                STRERROR_PRE();
                throw HTTPError_t::format(HTTP_SYSCALL,
                                          "Cannot connect socket: <%d, %s>.",
                                          ERRNO, STRERROR(ERRNO));
            }

            waitConnectSocket(fd, url, connectTimeout);
            checkSocket(fd);
        }

        // connect OK => do not close the socket!
        closer.release();
    }
}
#endif // !WIN32
