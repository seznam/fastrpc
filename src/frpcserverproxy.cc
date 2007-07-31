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
 * FILE          $Id: frpcserverproxy.cc,v 1.9 2007-07-31 13:01:18 vasek Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include <sstream>
#include <memory>

#include <stdarg.h>

#include "frpcserverproxy.h"
#include <frpc.h>
#include <frpctreebuilder.h>
#include <frpctreefeeder.h>
#include <memory>
#include <frpcfault.h>
#include <frpcresponseerror.h>

#include <frpcstruct.h>
#include <frpcstring.h>
#include <frpcint.h>
#include <frpcbool.h>


namespace {
    FRPC::Pool_t localPool;
    FRPC::Int_t &DEFAULT_READ_TIMEOUT(localPool.Int(10000));
    FRPC::Int_t &DEFAULT_WRITE_TIMEOUT(localPool.Int(1000));
    FRPC::Int_t &DEFAULT_CONNECT_TIMEOUT(localPool.Int(10000));

    FRPC::ProtocolVersion_t parseProtocolVersion(const FRPC::Struct_t &config,
                                                 const std::string name)
    {
        std::string strver
            (FRPC::String(config.get(name, FRPC::String_t::FRPC_EMPTY)));
        // empty/undefined => current version
        if (strver.empty()) return FRPC::ProtocolVersion_t();

        // parse input
        std::istringstream is(strver);
        int major, minor;
        is >> major >> minor;


        // OK
        return FRPC::ProtocolVersion_t(major, minor);
    }
}

namespace FRPC {


class ServerProxyImpl_t {
public:
    ServerProxyImpl_t(const std::string &server,
                      const ServerProxy_t::Config_t &config)
        : url(server, config.proxyUrl),
          io(-1, config.readTimeout, config.writeTimeout, -1 ,-1),
          connectTimeout(config.connectTimeout), keepAlive(config.keepAlive),
          rpcTransferMode(config.useBinary),
          useHTTP10(config.useHTTP10),
          serverSupportedProtocols(HTTPClient_t::XML_RPC),
          protocolVersion(config.protocolVersion),
          connector(new SimpleConnector_t(url))
    {}

    ServerProxyImpl_t(const std::string &server, const Struct_t &config)
        : url(server,
              FRPC::String(config.get("proxyUrl", String_t::FRPC_EMPTY))),
          io(-1,
             FRPC::Int(config.get("readTimeout", DEFAULT_READ_TIMEOUT)),
             FRPC::Int(config.get("writeTimeout", DEFAULT_WRITE_TIMEOUT)),
             -1, -1),
          connectTimeout(FRPC::Int(config.get("connectTimeout",
                                              DEFAULT_CONNECT_TIMEOUT))),
          keepAlive(FRPC::Bool(config.get("keepAlive", Bool_t::FRPC_FALSE))),
          rpcTransferMode(FRPC::Int(config.get("transferMode",
                                               Int_t::FRPC_ZERO))),
          useHTTP10(FRPC::Bool(config.get("useHTTP10", Bool_t::FRPC_FALSE))),
          serverSupportedProtocols(HTTPClient_t::XML_RPC),
          protocolVersion(parseProtocolVersion(config, "protocolVersion"))
    {}

    /** Set new read timeout */
    void setReadTimeout(int timeout) {
        io.setReadTimeout(timeout);
    }

    /** Set new write timeout */
    void setWriteTimeout(int timeout) {
        io.setWriteTimeout(timeout);
    }

    /** Set new connect timeout */
    void setConnectTimeout(int timeout) {
        connectTimeout = timeout;
    }

    const URL_t& getURL() {
        return url;
    }

    /** Create marshaller.
     */
    Marshaller_t* createMarshaller(HTTPClient_t &client);

    /** Call method.
     */
    Value_t& call(Pool_t &pool, const std::string &methodName,
                  const Array_t &params);

    /** Call method with variable number of arguments.
     */
    Value_t& call(Pool_t &pool, const char *methodName, va_list args);

private:
    URL_t url;
    HTTPIO_t io;
    unsigned int connectTimeout;
    bool keepAlive;
    unsigned int rpcTransferMode;
    bool useHTTP10;
    unsigned int serverSupportedProtocols;
    ProtocolVersion_t protocolVersion;
    std::auto_ptr<Connector_t> connector;
};

Marshaller_t* ServerProxyImpl_t::createMarshaller(HTTPClient_t &client) {
    Marshaller_t *marshaller;
    switch (rpcTransferMode) {
    case ServerProxy_t::Config_t::ON_SUPPORT:
        {
            if (serverSupportedProtocols & HTTPClient_t::BINARY_RPC) {
                //using BINARY_RPC
                marshaller= Marshaller_t::create(Marshaller_t::BINARY_RPC,
                                                 client,protocolVersion);
                client.prepare(HTTPClient_t::BINARY_RPC);
            } else {
                //using XML_RPC
                marshaller = Marshaller_t::create
                    (Marshaller_t::XML_RPC,client, protocolVersion);
                client.prepare(HTTPClient_t::XML_RPC);
            }
        }
    break;

    case ServerProxy_t::Config_t::NEVER:
        {
            //using BINARY_RPC  always
            marshaller= Marshaller_t::create(Marshaller_t::XML_RPC,
                                             client,protocolVersion);
            client.prepare(HTTPClient_t::XML_RPC);
        }
    break;

    case ServerProxy_t::Config_t::ALWAYS:
        {
            //using BINARY_RPC  always
            marshaller= Marshaller_t::create(Marshaller_t::BINARY_RPC,
                                             client,protocolVersion);
            client.prepare(HTTPClient_t::BINARY_RPC);
        }
    break;

    case ServerProxy_t::Config_t::ON_SUPPORT_ON_KEEP_ALIVE:
    default:
        {
            if ((serverSupportedProtocols & HTTPClient_t::XML_RPC)
                || keepAlive == false
                || io.socket() != -1) {
                //using XML_RPC
                marshaller= Marshaller_t::create
                    (Marshaller_t::XML_RPC,client, protocolVersion);
                client.prepare(HTTPClient_t::XML_RPC);
            } else {
                //using BINARY_RPC
                marshaller= Marshaller_t::create
                    (Marshaller_t::BINARY_RPC, client,protocolVersion);
                client.prepare(HTTPClient_t::BINARY_RPC);
            }
        }
        break;
    }

    // OK
    return marshaller;
}

ServerProxy_t::ServerProxy_t(const std::string &server, const Config_t &config)
    : sp(new ServerProxyImpl_t(server, config))
{}

ServerProxy_t::ServerProxy_t(const std::string &server, const Struct_t &config)
    : sp(new ServerProxyImpl_t(server, config))
{}


ServerProxy_t::~ServerProxy_t() {
    // get rid of implementation
    delete sp;
}

Value_t& ServerProxyImpl_t::call(Pool_t &pool, const std::string &methodName,
                                 const Array_t &params)
{
    HTTPClient_t client(io, url, connectTimeout,keepAlive, connector.get(),
                        useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try {
        marshaller->packMethodCall(methodName.c_str());
        for (Array_t::const_iterator
                 iparams = params.begin(),
                 eparams = params.end();
             iparams != eparams; ++iparams) {
            feeder.feedValue(**iparams);
        }

        marshaller->flush();
    } catch (const ResponseError_t &e) {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    protocolVersion = client.getProtocolVersion();
    if (&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    // OK, return unmarshalled data
    return builder.getUnMarshaledData();
}

Value_t& ServerProxy_t::call(Pool_t &pool, const std::string &methodName,
                             const Array_t &params)
{
    return sp->call(pool, methodName, params);
}

Value_t& ServerProxyImpl_t::call(Pool_t &pool, const char *methodName,
                                 va_list args)
{
    HTTPClient_t client(io, url, connectTimeout, keepAlive, connector.get(),
                        useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try {
        marshaller->packMethodCall(methodName);

        // marshall all passed values until null pointer
        while (const Value_t *value = va_arg(args, Value_t*))
            feeder.feedValue(*value);

        marshaller->flush();
    } catch (const ResponseError_t &e) {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    protocolVersion = client.getProtocolVersion();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    // OK, return unmarshalled data
    return builder.getUnMarshaledData();
}

namespace {
    /** Hold va_list and destroy it (via va_end) on destruction.
     */
    struct VaListHolder_t {
        VaListHolder_t(va_list &args) : args(args) {}
        ~VaListHolder_t() { va_end(args); }
        va_list &args;
    };
}

Value_t& ServerProxy_t::call(Pool_t &pool, const char *methodName, ...) {
    // get variadic arguments
    va_list args;
    va_start(args, methodName);
    VaListHolder_t argsHolder(args);

    // use implementation
    return sp->call(pool, methodName, args);
}

void ServerProxy_t::setReadTimeout(int timeout) {
    sp->setReadTimeout(timeout);
}

void ServerProxy_t::setWriteTimeout(int timeout) {
    sp->setWriteTimeout(timeout);
}

void ServerProxy_t::setConnectTimeout(int timeout) {
    sp->setConnectTimeout(timeout);
}

const URL_t& ServerProxy_t::getURL() {
    return sp->getURL();
}

} // namespace FRPC

