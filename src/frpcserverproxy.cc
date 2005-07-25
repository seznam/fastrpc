/*
 * FILE          $Id: frpcserverproxy.cc,v 1.2 2005-07-25 06:10:48 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2005
 * All Rights Reserved
 *
 * HISTORY
 *       
 */
#include "frpcserverproxy.h"
#include <frpc.h>
#include <frpctreebuilder.h>
#include <frpctreefeeder.h>
#include <memory>
#include <frpcfault.h>
#include <frpcresponseerror.h>

namespace FRPC
{


ServerProxy_t::ServerProxy_t(const std::string &server, Config_t &config)
    : url(server, config.proxyUrl), socket(-1),
      io(socket, config.readTimeout, config.writeTimeout, -1 ,-1),
      connectTimeout(config.connectTimeout), keepAlive(config.keepAlive),
      rpcTransferMode(config.useBinary),
      useHTTP10(config.useHTTP10),
      serverSupportedProtocols(HTTPClient_t::XML_RPC)
{}

ServerProxy_t::~ServerProxy_t()
{}

Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName)
{
	HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));

    try
    {
        marshaller->packMethodCall(methodName.c_str());
        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();

}

Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1)
{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);
    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();

}
Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                                    const Value_t &param2)
{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        feeder.feedValue(const_cast<Value_t&>(param2));

        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();

}

Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                                    const Value_t &param2,
                                    const Value_t &param3)
{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        feeder.feedValue(const_cast<Value_t&>(param2));
        feeder.feedValue(const_cast<Value_t&>(param3));

        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}


    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();

}
Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                                    const Value_t &param2,
                                    const Value_t &param3,
                                    const Value_t &param4)
{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        feeder.feedValue(const_cast<Value_t&>(param2));
        feeder.feedValue(const_cast<Value_t&>(param3));
        feeder.feedValue(const_cast<Value_t&>(param4));

        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();


}
Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                                    const Value_t &param2,
                                    const Value_t &param3, const Value_t &param4,
                                    const Value_t &param5)
{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        feeder.feedValue(const_cast<Value_t&>(param2));
        feeder.feedValue(const_cast<Value_t&>(param3));
        feeder.feedValue(const_cast<Value_t&>(param4));
        feeder.feedValue(const_cast<Value_t&>(param5));

        marshaller->flush();

    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();


}
Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                                    const Value_t &param2,
                                    const Value_t &param3, const Value_t &param4,
                                    const Value_t &param5, const Value_t &param6)

{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        feeder.feedValue(const_cast<Value_t&>(param2));
        feeder.feedValue(const_cast<Value_t&>(param3));
        feeder.feedValue(const_cast<Value_t&>(param4));
        feeder.feedValue(const_cast<Value_t&>(param5));
        feeder.feedValue(const_cast<Value_t&>(param6));

        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();


}
Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                                    const Value_t &param2,
                                    const Value_t &param3, const Value_t &param4,
                                    const Value_t &param5, const Value_t &param6,
                                    const Value_t &param7 )
{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        feeder.feedValue(const_cast<Value_t&>(param2));
        feeder.feedValue(const_cast<Value_t&>(param3));
        feeder.feedValue(const_cast<Value_t&>(param4));
        feeder.feedValue(const_cast<Value_t&>(param5));
        feeder.feedValue(const_cast<Value_t&>(param6));
        feeder.feedValue(const_cast<Value_t&>(param7));

        marshaller->flush();

    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();


}
Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                                    const Value_t &param2,
                                    const Value_t &param3, const Value_t &param4,
                                    const Value_t &param5, const Value_t &param6,
                                    const Value_t &param7, const Value_t &param8 )
{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        feeder.feedValue(const_cast<Value_t&>(param2));
        feeder.feedValue(const_cast<Value_t&>(param3));
        feeder.feedValue(const_cast<Value_t&>(param4));
        feeder.feedValue(const_cast<Value_t&>(param5));
        feeder.feedValue(const_cast<Value_t&>(param6));
        feeder.feedValue(const_cast<Value_t&>(param7));
        feeder.feedValue(const_cast<Value_t&>(param8));

        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();



}
Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                                    const Value_t &param2,
                                    const Value_t &param3, const Value_t &param4,
                                    const Value_t &param5, const Value_t &param6,
                                    const Value_t &param7, const Value_t &param8,
                                    const Value_t &param9 )
{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);
    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        feeder.feedValue(const_cast<Value_t&>(param2));
        feeder.feedValue(const_cast<Value_t&>(param3));
        feeder.feedValue(const_cast<Value_t&>(param4));
        feeder.feedValue(const_cast<Value_t&>(param5));
        feeder.feedValue(const_cast<Value_t&>(param6));
        feeder.feedValue(const_cast<Value_t&>(param7));
        feeder.feedValue(const_cast<Value_t&>(param8));
        feeder.feedValue(const_cast<Value_t&>(param9));

        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();


}
Value_t& ServerProxy_t::operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                                    const Value_t &param2,
                                    const Value_t &param3, const Value_t &param4,
                                    const Value_t &param5, const Value_t &param6,
                                    const Value_t &param7, const Value_t &param8,
                                    const Value_t &param9,const Value_t &param10)
{
    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try
    {
        marshaller->packMethodCall(methodName.c_str());

        feeder.feedValue(const_cast<Value_t&>(param1));
        feeder.feedValue(const_cast<Value_t&>(param2));
        feeder.feedValue(const_cast<Value_t&>(param3));
        feeder.feedValue(const_cast<Value_t&>(param4));
        feeder.feedValue(const_cast<Value_t&>(param5));
        feeder.feedValue(const_cast<Value_t&>(param6));
        feeder.feedValue(const_cast<Value_t&>(param7));
        feeder.feedValue(const_cast<Value_t&>(param8));
        feeder.feedValue(const_cast<Value_t&>(param9));
        feeder.feedValue(const_cast<Value_t&>(param10));

        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();
}


Value_t& ServerProxy_t::call(Pool_t &pool, const std::string &methodName, Array_t &params)
{

    HTTPClient_t client(io,url,connectTimeout,keepAlive, useHTTP10);
    TreeBuilder_t builder(pool);
    std::auto_ptr<Marshaller_t>marshaller(createMarshaller(client));
    TreeFeeder_t feeder(*marshaller);

    try
    {
        marshaller->packMethodCall(methodName.c_str());

        for(Array_t::iterator i = params.begin(); i != params.end(); ++i)
        {
            feeder.feedValue(**i);
        }

        marshaller->flush();
    }
    catch(const ResponseError_t &e)
    {}

    client.readResponse(builder);
    serverSupportedProtocols = client.getSupportedProtocols();
    if(&(builder.getUnMarshaledData()) == 0)
        throw  Fault_t(builder.getUnMarshaledErrorNumber(),
                       builder.getUnMarshaledErrorMessage());

    return builder.getUnMarshaledData();

}
const URL_t& ServerProxy_t::getURL()
{
    return url;
}

void ServerProxy_t::closeSocket()
{
    if(socket != -1 )
    {
        close(socket);
        socket = -1 ;
    }
}

}

