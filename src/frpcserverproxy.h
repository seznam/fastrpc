/*
 * FILE          $Id: frpcserverproxy.h,v 1.6 2006-10-31 11:19:42 vasek Exp $
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
#ifndef FRPCFRPCSERVERPROXY_H
#define FRPCFRPCSERVERPROXY_H

#include <frpcplatform.h>

#include <string>

#include <frpc.h>
#include <frpchttpio.h>
#include <frpchttp.h>
#include <frpchttpclient.h>
#include <frpcmarshaller.h>

namespace FRPC
{

/**
@brief ServerProxy Object
 
 
Server proxy is FastRpc client which call method on remote server 
@author Miroslav Talasek
*/

class FRPC_DLLEXPORT ServerProxy_t
{
public:
    /**
        @brief ServerProxy_t configuartion class 
        
        Is used to setting parameters as connectTimeout, readTimeout,...
        @author Miroslav Talasek
    */
    class Config_t
    {
    public:
        enum{ON_SUPPORT_ON_KEEP_ALIVE = 0, ON_SUPPORT, ALWAYS};
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
        Config_t(long connectTimeout, long readTimeout, long writeTimeout,
                 bool keepAlive, long useBinary, bool useHTTP10 = false):
                connectTimeout(connectTimeout),readTimeout(readTimeout),writeTimeout(writeTimeout),
                keepAlive(keepAlive), useBinary(useBinary), useHTTP10(useHTTP10)
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
                useHTTP10(false)
        {}
        ///@brief internal representation of connectTimeout value
        long connectTimeout;
        ///@brief internal representation of readTimeout value
        long readTimeout;
        ///@brief internal representation of writeTimeout  value
        long writeTimeout;
        ///@brief internal representation of keepAlive value
        bool keepAlive;
        ///@brief internal representation of useBinary value
        long useBinary;
        ///@brief use HTTP protocol version 1.0
        bool useHTTP10;

        ///@brief URL of proxy (empty if none)
        std::string proxyUrl;
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

    /** Compatibility version...
     */
    ServerProxy_t(const std::string &server, Config_t &config);

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
         likes:
            @n
            @n ret = box(pool, "getStatus");
            @n
            @n Where ret is return Value_t from remote method
    */
    Value_t& operator() (Pool_t &pool, const std::string &methodName);
    /**
       @brief Calling method one parameter
       
       This operator is used for calling method with one parameter on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @param param1 is Value_t parameter for the remote method
       @return return value from remote method Value_t
       
       @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with one parameter
        on remote server likes:
           @n ret = box("getStatus",pool.String("System"));
           @n Where ret is return Value_t from remote method and pool is alocator
    */
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1);
    /**
       @brief Calling method two parameters
       
       This operator is used for calling method with two parameters on remote server
       @param pool is reference to pool using to construct return values
       @param methodName is the remote method name
       @param param1 is Value_t parameter nr.1 for the remote method
       @param param2 is Value_t parameter nr.2 for the remote method
       @return return value from remote method Value_t
       
       @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with two parameters 
       on remote server likes:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true));
           @n
           @n Where ret is return Value_t from remote method and pool is alocator
    */
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                         const Value_t &param2);
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
       parameters on remote server likes:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true), pool.Double(2.1));
           @n
           @n Where ret is return Value_t from remote method  and pool is alocator
    */
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                         const Value_t &param2,
                         const Value_t &param3);
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
        parameters on remote server likes:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true),
                   pool.Int(1000),pool.Binary("AbCd"));
           @n
           @n Where ret is return Value_t from remote method and pool is alocator
    */
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                         const Value_t &param2,
                         const Value_t &param3,
                         const Value_t &param4);
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
     on remote server likes:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true),
                   pool.Int(1000),pool.Binary("AbCd"),pool,Array(pool.Int(5)));
           @n
           @n Where ret is return Value_t from remote method and pool is alocator
    */
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                         const Value_t &param2,
                         const Value_t &param3, const Value_t &param4, const Value_t &param5);
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
     on remote server likes:
           @n
           @n ret = box("getStatus",pool.String("System"),pool.Bool(true),
                   pool.Int(1000),pool.Binary("AbCd"),pool,Array(pool.Int(5)),pool.Int(10));
           @n
           @n Where ret is return Value_t from remote method and pool is alocator
    */
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                         const Value_t &param2,
                         const Value_t &param3, const Value_t &param4, const Value_t &param5,
                         const Value_t &param6);

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
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                         const Value_t &param2,
                         const Value_t &param3, const Value_t &param4, const Value_t &param5,
                         const Value_t &param6, const Value_t &param7 );
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
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                         const Value_t &param2,
                         const Value_t &param3, const Value_t &param4, const Value_t &param5,
                         const Value_t &param6, const Value_t &param7, const Value_t &param8 );
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
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                         const Value_t &param2,
                         const Value_t &param3, const Value_t &param4, const Value_t &param5,
                         const Value_t &param6, const Value_t &param7, const Value_t &param8,
                         const Value_t &param9 );
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
    Value_t& operator() (Pool_t &pool, const std::string &methodName, const Value_t &param1,
                         const Value_t &param2,
                         const Value_t &param3, const Value_t &param4, const Value_t &param5,
                         const Value_t &param6, const Value_t &param7, const Value_t &param8,
                         const Value_t &param9, const Value_t &param10  );

    /**
        @brief  Calling method with many parameters
        @param pool is reference to pool using to construct return values
        @param methodName is the remote method name
        @param params is Array_t. Is is an array of parameters
        @return return value from remote method Value_t
        
        @n @b Example:   box is ServerProxy_t object and calling methot (getStatus) with three 
       parameters on remote server likes:
           @n
           @n ret = box.call("getStatus",pool.Array(pool.String("System"),pool.Bool(true), pool.Double(2.1)));
           @n
           @n Where ret is return Value_t from remote method  and pool is alocator
    */
    Value_t& call(Pool_t &pool, const std::string &methodName, Array_t &params);
    /**
     *    @brief set new read timeout
    */
    inline void setReadTimeout(int timeout)
    {
        io.setReadTimeout(timeout);
    }
    /**
    *    @brief set new write timeout
    */
    inline void setWriteTimeout(int timeout)
    {
        io.setWriteTimeout(timeout);
    }
    /**
     *    @brief set new connect timeout
     */
    inline void setConnectTimeout(int timeout)
    {
        connectTimeout = timeout;
    }

    const URL_t& getURL();

private:
    inline Marshaller_t* createMarshaller(HTTPClient_t &client)
    {
        Marshaller_t *marshaller;

        switch(rpcTransferMode)
        {
        case Config_t::ON_SUPPORT:
            {
                if(serverSupportedProtocols & HTTPClient_t::BINARY_RPC)
                {
                    //using BINARY_RPC
                    marshaller= Marshaller_t::create(Marshaller_t::BINARY_RPC,client);
                    client.prepare(HTTPClient_t::BINARY_RPC);
                }
                else
                {
                    //using XML_RPC
                    marshaller= Marshaller_t::create(Marshaller_t::XML_RPC,client);
                    client.prepare(HTTPClient_t::XML_RPC);
                }
            }
            break;

        case Config_t::ALWAYS:
            {
                //using BINARY_RPC  always
                marshaller= Marshaller_t::create(Marshaller_t::BINARY_RPC,client);
                client.prepare(HTTPClient_t::BINARY_RPC);
            }
            break;
        case Config_t::ON_SUPPORT_ON_KEEP_ALIVE:
        default:
            {
                if(serverSupportedProtocols & HTTPClient_t::XML_RPC || keepAlive == false
                        || socket != -1)
                {
                    //using XML_RPC
                    marshaller= Marshaller_t::create(Marshaller_t::XML_RPC,client);
                    client.prepare(HTTPClient_t::XML_RPC);
                }
                else
                {
                    //using BINARY_RPC
                    marshaller= Marshaller_t::create(Marshaller_t::BINARY_RPC,client);
                    client.prepare(HTTPClient_t::BINARY_RPC);
                }


            }
            break;
        }
        return marshaller;
    }

    void closeSocket();

    ServerProxy_t(const ServerProxy_t&);

    ServerProxy_t& operator=(const ServerProxy_t&);


    URL_t url;
    int socket;
    HTTPIO_t io;
    int connectTimeout;
    bool keepAlive;
    int rpcTransferMode;
    bool useHTTP10;
    unsigned long serverSupportedProtocols;
}
;
}

#endif
