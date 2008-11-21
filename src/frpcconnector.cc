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
 * FILE          $Id: frpcconnector.cc,v 1.5 2008-11-21 10:31:27 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Vaclav Blazek <vaclav.blazek@firma.seznam.cz>
 *
 * HISTORY
 *
 */


#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

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
} // namespace

SimpleConnector_t::SimpleConnector_t(const URL_t &url, int connectTimeout,
                                     bool keepAlive)
    : Connector_t(url, connectTimeout, keepAlive)
{
    struct hostent *he = gethostbyname(url.host.c_str());
    if (!he) {
        // oops
        throw HTTPError_t(HTTP_DNS, "Cannot resolve host '%s': <%d, %s>.",
                          url.host.c_str(), h_errno,
                          host_strerror(h_errno));
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

        // check for activity on socket
        fd_set rfdset;
        FD_ZERO(&rfdset);
        FD_SET(fd, &rfdset);
        // okam¾itý timeout
        struct timeval timeout = { 0, 0 };
        switch (TEMP_FAILURE_RETRY(::select(fd + 1, &rfdset, 0, 0, &timeout))) {
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

    // if open socket is not availabe open new one
    if (fd < 0) {
        // otevøeme socket
        if ((fd = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
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

        // set not-delayed IO (we are not telnet)
        int just_say_no = 1;
        if (::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                         (char*) &just_say_no, sizeof(int)) < 0)
        {
            throw HTTPError_t(HTTP_SYSCALL,
                              "Cannot set socket non-delaying: "
                              "<%d, %s>.", ERRNO, STRERROR(ERRNO));
        }

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
                throw HTTPError_t(HTTP_SYSCALL,
                                  "Cannot connect socket: <%d, %s>.",
                                  ERRNO, STRERROR(ERRNO));
            }

            // create fd-set
            fd_set wfds;
            FD_ZERO(&wfds);
            FD_SET(fd, &wfds);

            // and timeout
            struct timeval timeout = {
                connectTimeout / 1000,
                (connectTimeout % 1000) * 1000
            };

            // wait for connect completion or timeout
            int ready = TEMP_FAILURE_RETRY(
                    ::select(fd + 1, 0, &wfds, 0, (connectTimeout < 0)
                                 ? 0 : &timeout));

            if (ready <= 0) {
                switch (ready) {
                case 0:
                    throw HTTPError_t(HTTP_SYSCALL,
                                      "Timeout while connecting.");

                default:
                    throw HTTPError_t(HTTP_SYSCALL,
                                      "Cannot select on socket: <%d, %s>.",
                                      ERRNO, STRERROR(ERRNO));
                }
            }

            // check for connect status
            socklen_t len = sizeof(int);
            int status;
#ifdef WIN32

            if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&status, &len))
#else //WIN32

            if (::getsockopt(fd, SOL_SOCKET, SO_ERROR, &status, &len))
#endif //WIN32

            {
                throw HTTPError_t(HTTP_SYSCALL,
                                  "Cannot get socket info: <%d, %s>.",
                                  ERRNO, STRERROR(ERRNO));
            }

            // check for error
            if (status) {
                throw HTTPError_t(HTTP_SYSCALL,
                                  "Cannot connect socket: <%d, %s>.",
                                  status, STRERROR(status));
            }
        }

        // connect OK => do not close the socket!
        closer.release();
    }
}
