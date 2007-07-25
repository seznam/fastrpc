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
 * FILE          $Id: frpcserverproxy.cc,v 1.8 2007-07-25 10:50:03 mirecta Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */

#include <stdarg.h>

#include "frpcserverproxy.h"
#include <frpc.h>
#include <frpctreebuilder.h>
#include <frpctreefeeder.h>
#include <memory>
#include <frpcfault.h>
#include <frpcresponseerror.h>

namespace FRPC
{


ServerProxy_t::ServerProxy_t(const std::string &server, const Config_t &config)
    : url(server, config.proxyUrl),
      io(-1, config.readTimeout, config.writeTimeout, -1 ,-1),
      connectTimeout(config.connectTimeout), keepAlive(config.keepAlive),
      rpcTransferMode(config.useBinary),
      useHTTP10(config.useHTTP10),
      serverSupportedProtocols(HTTPClient_t::XML_RPC),
      protocolVersion(config.protocolVersion)
{}

ServerProxy_t::ServerProxy_t(const std::string &server, Config_t &config)
    : url(server, config.proxyUrl),
      io(-1, config.readTimeout, config.writeTimeout, -1 ,-1),
      connectTimeout(config.connectTimeout), keepAlive(config.keepAlive),
      rpcTransferMode(config.useBinary),
      useHTTP10(config.useHTTP10),
      serverSupportedProtocols(HTTPClient_t::XML_RPC),
      protocolVersion(config.protocolVersion)
{}

ServerProxy_t::~ServerProxy_t()
{
}

Value_t& ServerProxy_t::call(Pool_t &pool, const std::string &methodName,
                             const Array_t &params)
{
    HTTPClient_t client(io, url, connectTimeout,keepAlive, useHTTP10);
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

    HTTPClient_t client(io, url, connectTimeout, keepAlive, useHTTP10);
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

const URL_t& ServerProxy_t::getURL() {
    return url;
}

}

