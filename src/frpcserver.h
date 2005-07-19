/*
 * FILE          $Id: frpcserver.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2002
 * All Rights Reserved
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCSERVER_H
#define FRPCFRPCSERVER_H

#include <frpcplatform.h>

#include <frpcmethodregistry.h>
#include <frpchttpio.h>
#include <frpchttp.h>
#include <frpcwriter.h>
#include <frpc.h>
#include <frpchttperror.h>
#include <list>
#include <string>


namespace FRPC
{
class DataBuilder_t;
class UnMarshaller_t;


/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Server_t:public Writer_t
{
public:
    enum{XML_RPC = 0x01, BINARY_RPC = 0x02};

    class Config_t
    {
    public:

        /**
            @brief Constructor of config class
            @param readTimeout - it is the read timeout in miliseconds
                                used in read data from the scoket
            @param writeTimeout - it is the write timeout 
                                 used in wite data to the socket
            @param keepAlive -  it is keep alive connection parameter 
            @param maxKeepalive - mas request on keep alive
            @param introspectionEnabled see MethodRegistry_t
            @param callbacks pointer to callback class for logging
        */
        Config_t(long readTimeout, long writeTimeout,
                 bool keepAlive, long maxKeepalive, bool introspectionEnabled,
                 MethodRegistry_t::Callbacks_t *callbacks )
                :readTimeout(readTimeout),writeTimeout(writeTimeout),
                keepAlive(keepAlive),maxKeepalive(maxKeepalive),
                introspectionEnabled(introspectionEnabled),
                callbacks(callbacks)
        {}
        /**
            @brief Default constructor 
            
            Setting default values:
            
            @n @b readTimeout = 10000 ms
            @n @b writeTimeout = 1000 ms
            @n @b keepAlive = false
            @n @b maxKeepalive = 0
            @n @b introspectionEnabled = true
            @n @b callbacks = 0
            
        */
        Config_t():readTimeout(10000),writeTimeout(1000),keepAlive(false),maxKeepalive(0),
                introspectionEnabled(true),
                callbacks(0)
        {}

        ///@brief internal representation of readTimeout value
        long readTimeout;
        ///@brief internal representation of writeTimeout  value
        long writeTimeout;
        ///@brief internal representation of keepAlive value
        bool keepAlive;

        long maxKeepalive;

        bool introspectionEnabled;

        MethodRegistry_t::Callbacks_t *callbacks;

    };

    Server_t(Config_t &config)
            :Writer_t(), methodRegistry(config.callbacks, config.introspectionEnabled),
            io(0,config.readTimeout, config.writeTimeout, -1 ,-1),
            keepAlive(config.keepAlive),maxKeepalive(config.maxKeepalive),
            callbacks(config.callbacks), outType(XML_RPC), closeConnection(true),
            contentLenght(0),useChunks(false),headersSent(false),head(false)

    {
        queryStorage.push_back(std::string(""));
        queryStorage.back().reserve(BUFFER_SIZE);

    }
    void serve(int fd, struct sockaddr_in* addr = 0);

    ~Server_t();

    MethodRegistry_t &registry()
    {
        return methodRegistry;
    }

    void readRequest(DataBuilder_t &builder);

    /**
    * @brief says to server that all data was writed
    *
    */
    virtual void flush();
    /**
    * @brief write data to server
    * @param data pointer to data 
    * @param size size of data
    */
    virtual void write(const char* data, long size);
    /**
    * @brief send response to client
    *
    */
    void sendResponse();
    /**
    * @brief send HTTP header with HTTPError_t to client
    *
    */
    void sendHttpError(const HTTPError_t &httpError);

private:
    Server_t();

    MethodRegistry_t methodRegistry;
    HTTPIO_t io;
    bool keepAlive;
    long maxKeepalive;
    MethodRegistry_t::Callbacks_t *callbacks;
    long outType;
    bool closeConnection;
    std::list<std::string> queryStorage;
    //UnMarshaller_t *unmarshaller;
    unsigned long contentLenght;
    bool  useChunks;
    bool headersSent;
    bool head;
};

};

#endif
