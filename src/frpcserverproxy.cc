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
 * FILE          $Id: frpcserverproxy.cc,v 1.11 2011-02-18 10:37:45 skeleton-golem Exp $
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
#include <map>
#include <memory>
#include <mutex>

#include <stdarg.h>

#include "frpcconnector.h"
#include "frpchttp.h"
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
    static FRPC::Pool_t localPool;

    static int getTimeout(const FRPC::Struct_t &config, const std::string &name,
                   int defaultValue)
    {
        // get key from config and check for existence
        const FRPC::Value_t *val(config.get(name));
        if (!val) return defaultValue;

        // OK
        return FRPC::Int(*val);
    }

    static FRPC::ProtocolVersion_t parseProtocolVersion(const FRPC::Struct_t &config,
                                                 const std::string &name)
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

    static FRPC::ServerProxy_t::Config_t configFromStruct(const FRPC::Struct_t &s) {
        FRPC::ServerProxy_t::Config_t config;

        config.proxyUrl = FRPC::String(s.get("proxyUrl", FRPC::String_t::FRPC_EMPTY));
        config.readTimeout = getTimeout(s, "readTimeout", 10000);
        config.writeTimeout = getTimeout(s, "writeTimeout", 1000);
        config.useBinary = FRPC::Int(s.get("transferMode", FRPC::Int_t::FRPC_ZERO));
        config.useHTTP10 = FRPC::Bool(s.get("useHTTP10", FRPC::Bool_t::FRPC_FALSE));
        config.protocolVersion = parseProtocolVersion(s, "protocolVersion");
        config.connectTimeout = getTimeout(s, "connectTimeout", 10000);
        config.keepAlive = FRPC::Bool(s.get("keepAlive", FRPC::Bool_t::FRPC_FALSE));

        return config;
    }

    static FRPC::Connector_t* makeConnector(
        const FRPC::URL_t &url,
        const unsigned &connectTimeout,
        const bool &keepAlive)
    {
        if (url.isUnix()) {
            return new FRPC::SimpleConnectorUnix_t(
               url, connectTimeout, keepAlive);
        }
        return new FRPC::SimpleConnectorIPv6_t(url, connectTimeout, keepAlive);
    }
}

namespace FRPC {

class ServerProxyImpl_t {
public:
    ServerProxyImpl_t(URL_t url,
                      const ServerProxy_t::Config_t &config)
        : url(std::move(url)),
          io(-1, config.readTimeout, config.writeTimeout, -1 ,-1),
          rpcTransferMode(config.useBinary),
          useHTTP10(config.useHTTP10),
          serverSupportedProtocols(HTTPClient_t::XML_RPC),
          protocolVersion(config.protocolVersion),
          connector(makeConnector(this->url, config.connectTimeout,
                                             config.keepAlive))
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
        connector->setTimeout(timeout);
    }
    
    void setRpcTransferMode(unsigned int v) {
        rpcTransferMode = v;
    }

    void setUseHTTP10(bool v) {
        useHTTP10 = v;
    }

    void setProtocolVersion(ProtocolVersion_t v) {
        protocolVersion = v;
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

    void call(DataBuilder_t &builder, const std::string &methodName,
                                 const Array_t &params);

    /** Call method with variable number of arguments.
     */
    Value_t& call(Pool_t &pool, const char *methodName, va_list args);

    void addRequestHttpHeaderForCall(const HTTPClient_t::Header_t& header);
    void addRequestHttpHeaderForCall(const HTTPClient_t::HeaderVector_t& headers);

    void addRequestHttpHeader(const HTTPClient_t::Header_t& header);
    void addRequestHttpHeader(const HTTPClient_t::HeaderVector_t& headers);

    void deleteRequestHttpHeaders();

    const Connector_t& getConnector() const { return *connector; }

private:
    URL_t url;
    HTTPIO_t io;
    unsigned int rpcTransferMode;
    bool useHTTP10;
    unsigned int serverSupportedProtocols;
    ProtocolVersion_t protocolVersion;
    std::unique_ptr<Connector_t> connector;
    HTTPClient_t::HeaderVector_t requestHttpHeadersForCall;
    HTTPClient_t::HeaderVector_t requestHttpHeaders;
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
            // never using BINARY_RPC
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
                || connector->getKeepAlive() == false
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

/**
 * Multifaceted data structure used to cache ServerProxy_t objects.
 * Maintains map for lookup by url, and timer queues for removing old objects.
 * The map and queues are interlinked. Timer queues are organized in powers of
 * two.
 */
class ProxyCache_t {
    struct LessURL_t {
        bool operator()(const URL_t &lhs, const URL_t &rhs) const {
            if (lhs.port == rhs.port && lhs.path == rhs.path) {
                return lhs.host < rhs.host;
            }

            if (lhs.port == rhs.port) {
                return lhs.path < rhs.path;
            }

            return lhs.port < rhs.port;
        }
    };

    class TimeoutListHead;
    using Time = uint64_t;
    using Key_t = URL_t;
    using Value_t = std::unique_ptr<TimeoutListHead>;
    using Map_t = std::multimap<Key_t, Value_t, LessURL_t>;
    static constexpr size_t TIMEOUT_QUEUES = 20;

    ProxyCache_t(int timeout) : timeout(timeout) {}

    static Time gettimemilliseconds() {
        timeval tv;
        gettimeofday(&tv, nullptr);
        return tv.tv_sec * 1000 + ((tv.tv_usec + 500) / 1000);
    };

    /** Intrusive linked-list structure for timer queues. */
    class TimeoutListHead {
        friend class ProxyCache_t;
        TimeoutListHead() : next(this), prev(this) {}

    public:
        TimeoutListHead(std::unique_ptr<ServerProxyImpl_t> impl)
            : impl(std::move(impl)), next(this), prev(this)
        {}
        ~TimeoutListHead() { delink(); }

        void delink() {
            next->prev = prev;
            prev->next = next;
            prev = next = this;
        }

        bool is_linked() const { return next != this; }

        std::unique_ptr<ServerProxyImpl_t> impl;

    protected:
        TimeoutListHead *next;
        TimeoutListHead *prev;
        Map_t::iterator self;
        Time next_setoff_time = 0;
        Time final_setoff_time = 0;
    };

    /** Inserts timer into correct queue */
    void schedule(TimeoutListHead &t, Time now) {
        if (t.is_linked()) {
            t.delink();
        }

        // find the correct timer queue
        auto timeout = t.final_setoff_time - now;
        t.next_setoff_time = t.final_setoff_time;
        size_t i = 0;
        for (; i < TIMEOUT_QUEUES - 1; ++i) {
            timeout >>= 1;
            if (timeout == 0) {
                timeout = 1 << i;
                t.next_setoff_time = now + timeout;
                break;
            }
        }

        // insert into the queue
        auto &queue = timeouts[i];
        queue.prev->next = &t;
        t.next = &queue;
        t.prev = queue.prev;
        queue.prev = &t;
        if (timeouts[i].next_setoff_time > t.next_setoff_time) {
            timeouts[i].next_setoff_time = t.next_setoff_time;
        }
    }

    /** Reschedule to msec from now */
    void scheduleRelative(TimeoutListHead &t, Time now, uint32_t msec) {
        t.final_setoff_time = now + msec;
        schedule(t, now);
    }

    /** Retire old timers and update queues */
    void perform_upkeep_locked() {
        Time now = gettimemilliseconds();
        for (size_t i = 0; i < TIMEOUT_QUEUES; ++i) {
            if (timeouts[i].next_setoff_time > now) {
                continue;
            }

            while (timeouts[i].is_linked()) {
                auto current_timeout = timeouts[i].next;
                if (current_timeout->next_setoff_time > now) {
                    // There are no more timers to retire.
                    timeouts[i].next_setoff_time = current_timeout->next_setoff_time;
                    break;
                }

                if (current_timeout->final_setoff_time > now) {
                    // The timer is retired in this queue but has some time left yet
                    schedule(*current_timeout, now);
                }
                else {
                    // The timer is completely retired and should be removed
                    map.erase(current_timeout->self);
                }
            }
        }
    }

public:
    /** Creates singleton object */
    static ProxyCache_t* instance() {
        auto factory = []() -> ProxyCache_t* {
            const char *s = getenv("FASTRPC_SERVER_PROXY_CACHE_TIMEOUT");
            int timeout = (s == nullptr) ? 0 : atoi(s);
            return new ProxyCache_t(timeout);
        };

        static std::unique_ptr<ProxyCache_t> cache(factory());
        return cache.get();
    }

    /** Pull ServerProxyImpl_t object from cache. */
    std::unique_ptr<ServerProxyImpl_t> lookup(const URL_t &url, const ServerProxy_t::Config_t &config) {
        if (timeout == 0) return nullptr;

        std::lock_guard<std::mutex> lock(mutex);
        perform_upkeep_locked();
        auto it = map.find(url);
        if (it == map.end()) {
            return nullptr;
        }
        auto impl = std::move(it->second->impl);
        map.erase(it);
        impl->setReadTimeout(config.readTimeout);
        impl->setWriteTimeout(config.writeTimeout);
        impl->setRpcTransferMode(config.useBinary);
        impl->setUseHTTP10(config.useHTTP10);
        impl->setProtocolVersion(config.protocolVersion);
        impl->setConnectTimeout(config.connectTimeout);

        return impl;
    }

    /** Put used ServerProxyImpl_t object into cache */
    void move_into(std::auto_ptr<ServerProxyImpl_t> &sp) {
        if (timeout == 0) return;

        std::lock_guard<std::mutex> lock(mutex);
        perform_upkeep_locked();
        std::unique_ptr<ServerProxyImpl_t> impl(sp.release());
        std::unique_ptr<TimeoutListHead> tlh(new TimeoutListHead(std::move(impl)));
        auto &url = tlh->impl->getURL();
        auto it = map.emplace(url, std::move(tlh));
        it->second->self = it;
        auto now = gettimemilliseconds();
        scheduleRelative(*it->second, now, timeout * 1000);
    }

protected:
    std::mutex mutex;
    Map_t map;
    const int timeout = 0;
    TimeoutListHead timeouts[TIMEOUT_QUEUES];
};

static ServerProxyImpl_t* createImpl(const std::string &server, const ServerProxy_t::Config_t &config) {
    URL_t url(server, config.proxyUrl);
    if (config.keepAlive) {
        auto *cache = ProxyCache_t::instance();
        auto *impl = cache->lookup(url, config).release();
        if (impl) {
            return impl;
        }
    }

    return new ServerProxyImpl_t(std::move(url), config);
}

ServerProxy_t::ServerProxy_t(const std::string &server, const Config_t &config)
    : sp(createImpl(server, config))
{}

ServerProxy_t::ServerProxy_t(const std::string &server, const Struct_t &config)
    : sp(createImpl(server, configFromStruct(config)))
{}

ServerProxy_t::~ServerProxy_t() {
    // get rid of implementation
    if (sp->getConnector().getKeepAlive()) {
        ProxyCache_t::instance()->move_into(sp);
    }
}


Value_t& ServerProxyImpl_t::call(Pool_t &pool, const std::string &methodName,
                                 const Array_t &params)
{
    HTTPClient_t client(io, url, connector.get(), useHTTP10);
    {
        client.addCustomRequestHeader(requestHttpHeaders);
        client.addCustomRequestHeader(requestHttpHeadersForCall);
        requestHttpHeadersForCall.clear();
    }
    TreeBuilder_t builder(pool);
    std::unique_ptr<Marshaller_t>marshaller(createMarshaller(client));
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

    // OK, return unmarshalled data (throws fault if NULL)
    return builder.getUnMarshaledData();
}


void ServerProxyImpl_t::call(DataBuilder_t &builder, const std::string &methodName,
                                 const Array_t &params)
{
    HTTPClient_t client(io, url, connector.get(), useHTTP10);
    {
        client.addCustomRequestHeader(requestHttpHeaders);
        client.addCustomRequestHeader(requestHttpHeadersForCall);
        requestHttpHeadersForCall.clear();
    }
    std::unique_ptr<Marshaller_t>marshaller(createMarshaller(client));
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
}


Value_t& ServerProxy_t::call(Pool_t &pool, const std::string &methodName,
                             const Array_t &params)
{
    return sp->call(pool, methodName, params);
}

Value_t& ServerProxyImpl_t::call(Pool_t &pool, const char *methodName,
                                 va_list args)
{
    HTTPClient_t client(io, url, connector.get(), useHTTP10);
    {
        client.addCustomRequestHeader(requestHttpHeaders);
        client.addCustomRequestHeader(requestHttpHeadersForCall);
        requestHttpHeadersForCall.clear();
    }
    TreeBuilder_t builder(pool);
    std::unique_ptr<Marshaller_t>marshaller(createMarshaller(client));
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

    // OK, return unmarshalled data (throws fault if NULL)
    return builder.getUnMarshaledData();
}

void ServerProxyImpl_t::addRequestHttpHeaderForCall(const HTTPClient_t::Header_t& header)
{
    requestHttpHeadersForCall.push_back(header);
}

void ServerProxyImpl_t::addRequestHttpHeaderForCall(const HTTPClient_t::HeaderVector_t& headers)
{
    for (HTTPClient_t::HeaderVector_t::const_iterator it=headers.begin(); it!=headers.end(); it++) {
        addRequestHttpHeaderForCall(*it);
    }
}

void ServerProxyImpl_t::addRequestHttpHeader(const HTTPClient_t::Header_t& header)
{
    requestHttpHeaders.push_back(header);
}

void ServerProxyImpl_t::addRequestHttpHeader(const HTTPClient_t::HeaderVector_t& headers)
{
    for (HTTPClient_t::HeaderVector_t::const_iterator it=headers.begin(); it!=headers.end(); it++) {
        addRequestHttpHeader(*it);
    }
}

void ServerProxyImpl_t::deleteRequestHttpHeaders() {
    requestHttpHeaders.clear();
    requestHttpHeadersForCall.clear();
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

void ServerProxy_t::call(DataBuilder_t &builder,
        const std::string &methodName, const Array_t &params)
{
    sp->call(builder, methodName, params);
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

void ServerProxy_t::setForwardHeader(const std::string &fwd) {
    sp->addRequestHttpHeaderForCall(std::make_pair(HTTP_HEADER_X_FORWARDED_FOR, fwd));
}

const URL_t& ServerProxy_t::getURL() {
    return sp->getURL();
}

void ServerProxy_t::addRequestHttpHeaderForCall(const HTTPClient_t::Header_t& header) {
    sp->addRequestHttpHeaderForCall(header);
}
void ServerProxy_t::addRequestHttpHeaderForCall(const HTTPClient_t::HeaderVector_t& headers) {
    sp->addRequestHttpHeaderForCall(headers);
}

void ServerProxy_t::addRequestHttpHeader(const HTTPClient_t::Header_t& header) {
    sp->addRequestHttpHeader(header);
}

void ServerProxy_t::addRequestHttpHeader(const HTTPClient_t::HeaderVector_t& headers) {
    sp->addRequestHttpHeader(headers);
}

void ServerProxy_t::deleteRequestHttpHeaders() {
    sp->deleteRequestHttpHeaders();
}

} // namespace FRPC
