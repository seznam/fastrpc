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
 * FILE          $Id: frpcserverproxy.h,v 1.20 2011-02-25 09:21:07 volca Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCSERVERPROXY_H
#define FRPCFRPCSERVERPROXY_H

#include <memory>

#include <frpcplatform.h>

#include <string>

#include <frpc.h>
#include <frpchttpio.h>
#include <frpchttp.h>
#include <frpchttpclient.h>
#include <frpcmarshaller.h>

namespace FRPC {

/** Server proxy implementation holder.
 */
class ServerProxyImpl_t;

class Struct_t;

class DataBuilder_t;

/**
@brief ServerProxy Object


Server proxy is FastRpc client which call method on remote server
@author Miroslav Talasek
*/

class FRPC_DLLEXPORT ServerProxy_t {
public:
    /**
        @brief ServerProxy_t configuartion class

        Is used to setting parameters as connectTimeout, readTimeout,...
        @author Miroslav Talasek
    */
    class Config_t {
    public:
        enum {ON_SUPPORT_ON_KEEP_ALIVE = 0, ON_SUPPORT, ALWAYS,NEVER};
        /**
            @brief Constructor of config class
            @param connectTimeout - it is connection timeout in miliseconds
                                    used in connect to the RPC server
            @param readTimeout - it is the read timeout in miliseconds
                                used in read data from the scoket
            @param writeTimeout - it is the write timeout
                                 used in wite data to the socket
            @param keepAlive -  it is keep alive connection parameter

            @param useBinary - says how client switching modes(XML, binary)

            @param useHTTP10 - says that client must use HTTP/1.0 in all
                               conditions
        */
        Config_t(unsigned int connectTimeout, unsigned int readTimeout,
                 unsigned int writeTimeout,
                 bool keepAlive, unsigned int useBinary, bool useHTTP10 = false)
            : connectTimeout(connectTimeout),readTimeout(readTimeout),
              writeTimeout(writeTimeout),
              keepAlive(keepAlive), useBinary(useBinary), useHTTP10(useHTTP10)
        {}

        /**
            @brief Constructor of config class
            @param connectTimeout - it is connection timeout in miliseconds
            used in connect to the RPC server
            @param readTimeout - it is the read timeout in miliseconds
            used in read data from the scoket
            @param writeTimeout - it is the write timeout
            used in wite data to the socket
            @param keepAlive -  it is keep alive connection parameter

            @param useBinary - says how client switching modes(XML, binary)

            @param useHTTP10 - says that client must use HTTP/1.0 in all
            conditions
            @param protocolVersionMajor - major version of protocol
            @param protocolVersionMinor - minor version of protocol

         */
        Config_t(unsigned int connectTimeout, unsigned int readTimeout,
                 unsigned int writeTimeout,
                 bool keepAlive, unsigned int useBinary,
                unsigned char protocolVersionMajor = 2,
                unsigned char protocolVersionMinor = 0, bool useHTTP10 = false)
            : connectTimeout(connectTimeout),readTimeout(readTimeout),
              writeTimeout(writeTimeout),
              keepAlive(keepAlive), useBinary(useBinary),
              useHTTP10(useHTTP10), useChunks(!useHTTP10),
              protocolVersion(protocolVersionMajor,protocolVersionMinor)
        {}

        /**
           @brief Default constructor

           Setting default values:
           @n @b connectTimeout = 10000 ms
           @n @b readTimeout = 10000 ms
           @n @b writeTimeout = 1000 ms
        */
        Config_t()
            : connectTimeout(10000), readTimeout(10000), writeTimeout(1000),
              keepAlive(false), useBinary(ON_SUPPORT_ON_KEEP_ALIVE),
              useHTTP10(false), useChunks(true)
        {}

        ///@brief internal representation of connectTimeout value
        unsigned int connectTimeout;
        ///@brief internal representation of readTimeout value
        unsigned int readTimeout;
        ///@brief internal representation of writeTimeout  value
        unsigned int writeTimeout;
        ///@brief internal representation of keepAlive value
        bool keepAlive;
        ///@brief internal representation of useBinary value
        unsigned int useBinary;
        ///@brief use HTTP protocol version 1.0
        bool useHTTP10;
        ///@brief use chunked encoding
        bool useChunks;
        ///@brief URL of proxy (empty if none)
        std::string proxyUrl;
        ///@brief Protocol version
        ProtocolVersion_t protocolVersion;
    };

    /**
        @brief Constructor
        @param server is string contain of address of FarstRpc server
                      @n Format of address is "http://machine:port/PRC2"
                      @n @b Examles:
                      @li "http://box:2600/RPC2"
                      @li "http://192.168.10.2:2800/RPC2"
        @param config is configuration structure ServerProxy_t::Config_t
    */
    ServerProxy_t(const std::string &server, const Config_t &config);

    /**
        @brief Constructor
        @param server string containing address of FarstRpc server
                      @n Format of address is "http://machine:port/PRC2"
                      @n @b Examles:
                      @li "http://box:2600/RPC2"
                      @li "http://192.168.10.2:2800/RPC2"
        @param config generic configuration via FRPC::Struct_t
    */
    ServerProxy_t(const std::string &server, const Struct_t &config);

    /**
        @brief Default destructor
    */
    ~ServerProxy_t();

    /**
        @brief Calling method without parameters

        This operator is used for calling method without parameters on remote server
         @param pool is reference to pool using to construct return values
        @param methodName is the remote method name
        @return return value from remote method Value_t

        @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) on remote server
         like:
            @n
            @n ret = box(pool, "getStatus");
            @n
            @n Where ret is return Value_t from remote method
    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName) {
        return call(pool, methodName.c_str(), static_cast<void*>(0x0));
    }

    /**
       @brief Calling method one parameter

       This operator is used for calling method with one parameter on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @param param1 is Value_t parameter for the remote method
       @return return value from remote method Value_t

       @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with one parameter
        on remote server like:
           @n ret = box("getStatus",pool.String("System"));
           @n Where ret is return Value_t from remote method and pool is alocator
    */

    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1)
    {
        return call(pool, methodName.c_str(), &param1,
                    static_cast<void*>(0x0));
    }

    /**
       @brief Calling method two parameters

       This operator is used for calling method with two parameters on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @param param1 is Value_t parameter nr.1 for the remote method
       @param param2 is Value_t parameter nr.2 for the remote method
       @return return value from remote method Value_t

       @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with two parameters
       on remote server like:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true));
           @n
           @n Where ret is return Value_t from remote method and pool is alocator
    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1, const Value_t &param2)
    {
        return call(pool, methodName.c_str(), &param1, &param2,
                    static_cast<void*>(0x0));
    }

    /**
       @brief Calling method three parameters

       This operator is used for calling method with three parameters on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @return return value from remote method Value_t
       @param param1 is Value_t parameter nr.1 for the remote method
       @param param2 is Value_t parameter nr.2 for the remote method
       @param param3 is Value_t parameter nr.3 for the remote method
       @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with three
       parameters on remote server like:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true), pool.Double(2.1));
           @n
           @n Where ret is return Value_t from remote method  and pool is alocator
    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1, const Value_t &param2,
                        const Value_t &param3)
    {
        return call(pool, methodName.c_str(), &param1, &param2, &param3,
                    static_cast<void*>(0x0));
    }

    /**
        @brief Calling method four parameters

        This operator is used for calling method with four parameters on remote server
        @param pool is reference to pool using to construct return values
        @param methodName is the remote method name
        @return return value from remote method Value_t
        @param param1 is Value_t parameter nr.1 for the remote method
        @param param2 is Value_t parameter nr.2 for the remote method
        @param param3 is Value_t parameter nr.3 for the remote method
        @param param4 is Value_t parameter nr.4 for the remote method
        @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with four
        parameters on remote server like:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true),
                   pool.Int(1000),pool.Binary("AbCd"));
           @n
           @n Where ret is return Value_t from remote method and pool is alocator
    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1, const Value_t &param2,
                        const Value_t &param3, const Value_t &param4)
    {
        return call(pool, methodName.c_str(), &param1, &param2, &param3,
                    &param4, static_cast<void*>(0x0));
    }

    /**
       @brief Calling method with five parameters

       This operator is used for calling method with five parameters on remote server
        @param pool is reference to pool using to construct return values
        @param methodName is the remote method name
        @return return value from remote method Value_t
        @param param1 is Value_t parameter nr.1 for the remote method
        @param param2 is Value_t parameter nr.2 for the remote method
        @param param3 is Value_t parameter nr.3 for the remote method
        @param param4 is Value_t parameter nr.4 for the remote method
        @param param5 is Value_t parameter nr.5 for the remote method
        @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with five parameters
     on remote server like:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true),
                   pool.Int(1000),pool.Binary("AbCd"),pool,Array(pool.Int(5)));
           @n
           @n Where ret is return Value_t from remote method and pool is alocator
    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1, const Value_t &param2,
                        const Value_t &param3, const Value_t &param4,
                        const Value_t &param5)
    {
        return call(pool, methodName.c_str(), &param1, &param2,
                    &param3, &param4, &param5, static_cast<void*>(0x0));
    }

    /**
       @brief Calling method with six parameters

       This operator is used for calling method with six parameters on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @return return value from remote method Value_t
       @param param1 is Value_t parameter nr.1 for the remote method
       @param param2 is Value_t parameter nr.2 for the remote method
       @param param3 is Value_t parameter nr.3 for the remote method
       @param param4 is Value_t parameter nr.4 for the remote method
       @param param5 is Value_t parameter nr.5 for the remote method
       @param param6 is Value_t parameter nr.6 for the remote method
       @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with six parameters
     on remote server like:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true),
                   pool.Int(1000),pool.Binary("AbCd"),pool,Array(pool.Int(5)),pool.Int(10));
           @n
           @n Where ret is return Value_t from remote method and pool is alocator
    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1, const Value_t &param2,
                        const Value_t &param3, const Value_t &param4,
                        const Value_t &param5, const Value_t &param6)
    {
        return call(pool, methodName.c_str(), &param1, &param2, &param3,
                    &param4, &param5, &param6, static_cast<void*>(0x0));
    }

    /**
       @brief Calling method with seven parameters

       This operator is used for calling method with seven parameters on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @param param1 is Value_t parameter nr.1 for the remote method
       @param param2 is Value_t parameter nr.2 for the remote method
       @param param3 is Value_t parameter nr.3 for the remote method
       @param param4 is Value_t parameter nr.4 for the remote method
       @param param5 is Value_t parameter nr.5 for the remote method
       @param param6 is Value_t parameter nr.6 for the remote method
       @param param7 is Value_t parameter nr.7 for the remote method
       @return return value from remote method Value_t

       Using similar as other

    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1, const Value_t &param2,
                        const Value_t &param3, const Value_t &param4,
                        const Value_t &param5, const Value_t &param6,
                        const Value_t &param7)
    {
        return call(pool, methodName.c_str(), &param1, &param2, &param3,
                    &param4, &param5, &param6, &param7,
                    static_cast<void*>(0x0));
    }

    /**
       @brief Calling method with eight parameters

       This operator is used for calling method with eight parameters on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @param param1 is Value_t parameter nr.1 for the remote method
       @param param2 is Value_t parameter nr.2 for the remote method
       @param param3 is Value_t parameter nr.3 for the remote method
       @param param4 is Value_t parameter nr.4 for the remote method
       @param param5 is Value_t parameter nr.5 for the remote method
       @param param6 is Value_t parameter nr.6 for the remote method
       @param param7 is Value_t parameter nr.7 for the remote method
       @param param8 is Value_t parameter nr.8 for the remote method
       @return return value from remote method Value_t

       Using similar as other

    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1, const Value_t &param2,
                        const Value_t &param3, const Value_t &param4,
                        const Value_t &param5, const Value_t &param6,
                        const Value_t &param7, const Value_t &param8)
    {
        return call(pool, methodName.c_str(), &param1, &param2, &param3,
                    &param4, &param5, &param6, &param7, &param8,
                    static_cast<void*>(0x0));
    }

    /**
       @brief Calling method with nine parameters

       This operator is used for calling method with nine parameters on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @param param1 is Value_t parameter nr.1 for the remote method
       @param param2 is Value_t parameter nr.2 for the remote method
       @param param3 is Value_t parameter nr.3 for the remote method
       @param param4 is Value_t parameter nr.4 for the remote method
       @param param5 is Value_t parameter nr.5 for the remote method
       @param param6 is Value_t parameter nr.6 for the remote method
       @param param7 is Value_t parameter nr.7 for the remote method
       @param param8 is Value_t parameter nr.8 for the remote method
       @param param9 is Value_t parameter nr.9 for the remote method
       @return return value from remote method Value_t

       Using similar as other

    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1, const Value_t &param2,
                        const Value_t &param3, const Value_t &param4,
                        const Value_t &param5, const Value_t &param6,
                        const Value_t &param7, const Value_t &param8,
                        const Value_t &param9)
    {
        return call(pool, methodName.c_str(), &param1, &param2, &param3,
                    &param4, &param5, &param6, &param7, &param8, &param9,
                    static_cast<void*>(0x0));
    }

    /**
       @brief Calling method with ten parameters

       This operator is used for calling method with ten parameters on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @param param1 is Value_t parameter nr.1 for the remote method
       @param param2 is Value_t parameter nr.2 for the remote method
       @param param3 is Value_t parameter nr.3 for the remote method
       @param param4 is Value_t parameter nr.4 for the remote method
       @param param5 is Value_t parameter nr.5 for the remote method
       @param param6 is Value_t parameter nr.6 for the remote method
       @param param7 is Value_t parameter nr.7 for the remote method
       @param param8 is Value_t parameter nr.8 for the remote method
       @param param9 is Value_t parameter nr.9 for the remote method
       @param param10 is Value_t parameter nr.10 for the remote method
       @return return value from remote method Value_t

       Using similar as other

    */
    Value_t& operator()(Pool_t &pool, const std::string &methodName,
                        const Value_t &param1, const Value_t &param2,
                        const Value_t &param3, const Value_t &param4,
                        const Value_t &param5, const Value_t &param6,
                        const Value_t &param7, const Value_t &param8,
                        const Value_t &param9, const Value_t &param10)
    {
        return call(pool, methodName.c_str(), &param1, &param2, &param3,
                    &param4, &param5, &param6, &param7, &param8, &param9,
                    &param10, static_cast<void*>(0x0));
    }


    /**
        @brief  Calling method with many parameters
        @param pool is reference to pool using to construct return values
        @param methodName is the remote method name
        @param params is Array_t. Is is an array of parameters
        @return return value from remote method Value_t

        @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with three
       parameters on remote server like:
           @n
           @n ret = box.call("getStatus",pool.Array(pool.String("System"),pool.Bool(true), pool.Double(2.1)));
           @n
           @n Where ret is return Value_t from remote method  and pool is alocator
    */
    Value_t& call(Pool_t &pool, const std::string &methodName,
                  const Array_t &params);

    /**
        @brief  Calling method with many parameters
        @param builder is virtual class used to build custom response
        @param methodName is the remote method name
        @param params is Array_t. Is is an array of parameters
    */
    void call(DataBuilder_t &builder, const std::string &methodName,
            const Array_t &params);

    /**
        @brief  Calling method with many parameters
        @param pool is reference to pool using to construct return values
        @param methodName is the remote method name
        @param ... any number of Value_t* parameters terminated by
                   null-pointer (0).
        @return return value from remote method Value_t

        @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with three
       parameters on remote server like:
           @n
           @n ret = box.call("getStatus", &pool.String("System"),
                             &pool.Bool(true), &pool.Double(2.1));
           @n
           @n Where ret is return Value_t from remote method and pool is
           alocator
    */
    Value_t& call(Pool_t &pool, const char *methodName, ...);

    /** @brief set new read timeout */
    void setReadTimeout(int timeout);

    /** @brief set new write timeout */
    void setWriteTimeout(int timeout);

    /** @brief set new connect timeout */
    void setConnectTimeout(int timeout);

    /** @brief sets a value to the X-Forwarded-For header for next call,
     *         used on proxy servers. */
    void setForwardHeader(const std::string &forwarded);

    const URL_t& getURL();

    void addRequestHttpHeaderForCall(const HTTPClient_t::Header_t& header);
    void addRequestHttpHeaderForCall(const HTTPClient_t::HeaderVector_t& headers);

    void addRequestHttpHeader(const HTTPClient_t::Header_t& header);
    void addRequestHttpHeader(const HTTPClient_t::HeaderVector_t& headers);

    void deleteRequestHttpHeaders();

private:
    ServerProxy_t(const ServerProxy_t&);

    ServerProxy_t& operator=(const ServerProxy_t&);

    std::auto_ptr<ServerProxyImpl_t> sp;
};

} // namespace FRPC

#endif
