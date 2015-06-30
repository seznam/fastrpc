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
 * FILE          $Id: frpcconnector.h,v 1.3 2007-07-31 14:18:47 vasek Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Vaclav Blazek <vaclav.blazek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCCONNECTOR_H
#define FRPCCONNECTOR_H

#include <frpcplatform.h>
#include <frpchttp.h>


namespace FRPC {

/** Socket connector. Base class.
 */
class FRPC_DLLEXPORT Connector_t {
public:
    Connector_t(const URL_t &url, int connectTimeout, bool keepAlive);

    virtual ~Connector_t();

    /** Create new connection to the address given by URL or check existing
     *  one.
     *
     * @param fd connected socket or -1 if not yet connected
     */
    virtual void connectSocket(int &fd) = 0;

    void setTimeout(int timeout) {
        connectTimeout = timeout;
    }

    bool getKeepAlive() const {
        return keepAlive;
    }

protected:
    URL_t url;
    int connectTimeout;
    bool keepAlive;

private:
    Connector_t(const Connector_t&);
    Connector_t& operator=(const Connector_t&);
};

/** Simple socket connector: connects to the first IP returned by host
 *  resolution.
 */
class FRPC_DLLEXPORT SimpleConnector_t : public Connector_t {
public:
    SimpleConnector_t(const URL_t &url, int connectTimeout, bool keepAlive);

    virtual ~SimpleConnector_t();

    virtual void connectSocket(int &fd);

private:

    /** Resolved IP address.
     */
    in_addr ipaddr;
};


/** Simple socket connector: connects to the first IP returned by host
 *  resolution.
 */
class FRPC_DLLEXPORT SimpleConnectorIPv6_t : public Connector_t {
public:
    SimpleConnectorIPv6_t(const URL_t &url, int connectTimeout, bool keepAlive);

    virtual ~SimpleConnectorIPv6_t();

    virtual void connectSocket(int &fd);

private:

    /** Resolved IP address.
     */
    struct addrinfo *addrInfo;
};

#ifndef WIN32

/** Simple unix socket connector.
 */
class FRPC_DLLEXPORT SimpleConnectorUnix_t : public Connector_t {
public:
    SimpleConnectorUnix_t(const URL_t &url, int connectTimeout, bool keepAlive);

    virtual ~SimpleConnectorUnix_t();

    virtual void connectSocket(int &fd);
};

#endif // !WIN32

} // namespace FRPC

#endif // FRPCCONNECTOR
