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
 * FILE          $Id: frpcserver.h,v 1.10 2011-02-25 09:21:07 volca Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCSERVER_H
#define FRPCFRPCSERVER_H

#include <memory>

#include <frpcplatform.h>

#include <frpcmethodregistry.h>
#include <frpchttpio.h>
#include <frpchttp.h>
#include <frpcwriter.h>
#include <frpc.h>
#include <frpchttperror.h>
#include <list>
#include <string>


namespace FRPC {

class DataBuilder_t;
class UnMarshaller_t;

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Server_t:public Writer_t {
public:
    enum{XML_RPC = 0x01, BINARY_RPC = 0x02, JSON = 0x03, BASE64_RPC = 0x04};

    class Config_t {
    public:

        /**
            @brief Constructor of config class - compatible constructor.
            @param readTimeout - it is the read timeout in miliseconds
                                used in read data from the scoket
            @param writeTimeout - it is the write timeout
                                 used in wite data to the socket
            @param keepAlive -  it is keep alive connection parameter
            @param useBinary - allow or disallow binary protocol
            @param maxKeepalive - mas request on keep alive
            @param introspectionEnabled see MethodRegistry_t
            @param callbacks pointer to callback class for logging
            @param path uri path
        */
        Config_t(unsigned int readTimeout, unsigned int writeTimeout,
                 bool keepAlive, unsigned int maxKeepalive,
                 bool introspectionEnabled,
                 MethodRegistry_t::Callbacks_t *callbacks
                /*, const std::string path*/)
            : readTimeout(readTimeout), writeTimeout(writeTimeout),
              keepAlive(keepAlive), useBinary(true),
              maxKeepalive(maxKeepalive),
              introspectionEnabled(introspectionEnabled), callbacks(callbacks)
                //,path(path)
        {}

        /**
            @brief Constructor of config class
            @param readTimeout - it is the read timeout in miliseconds
                                used in read data from the scoket
            @param writeTimeout - it is the write timeout
                                 used in wite data to the socket
            @param keepAlive -  it is keep alive connection parameter
            @param useBinary - allow or disallow binary protocol
            @param maxKeepalive - mas request on keep alive
            @param introspectionEnabled see MethodRegistry_t
            @param callbacks pointer to callback class for logging
            @param path uri path
        */
        Config_t(unsigned int readTimeout, unsigned int writeTimeout,
                 bool keepAlive, bool useBinary, unsigned int maxKeepalive,
                 bool introspectionEnabled,
                 MethodRegistry_t::Callbacks_t *callbacks
                /*, const std::string path*/)
            : readTimeout(readTimeout), writeTimeout(writeTimeout),
              keepAlive(keepAlive), useBinary(useBinary),
              maxKeepalive(maxKeepalive),
              introspectionEnabled(introspectionEnabled), callbacks(callbacks)
                //,path(path)
        {}
        /**
            @brief Default constructor

            Setting default values:

            @n @b readTimeout = 10000 ms
            @n @b writeTimeout = 1000 ms
            @n @b keepAlive = false
            @n @b useBinary = true
            @n @b maxKeepalive = 0
            @n @b introspectionEnabled = true
            @n @b callbacks = 0

        */
        Config_t()
            : readTimeout(10000), writeTimeout(1000), keepAlive(false),
              useBinary(true), maxKeepalive(0), introspectionEnabled(true),
              callbacks(0)
        {}

        ///@brief internal representation of readTimeout value
        unsigned int readTimeout;
        ///@brief internal representation of writeTimeout  value
        unsigned int writeTimeout;
        ///@brief internal representation of keepAlive value
        bool keepAlive;

        ///@brief allow or disallow binary protocol
        bool useBinary;

        unsigned int maxKeepalive;

        bool introspectionEnabled;

        MethodRegistry_t::Callbacks_t *callbacks;
    };

    Server_t(Config_t &config)
        : Writer_t(),
          methodRegistry(config.callbacks, config.introspectionEnabled),
          io(0, config.readTimeout, config.writeTimeout, -1, -1),
          keepAlive(config.keepAlive), useBinary(config.useBinary),
          maxKeepalive(config.maxKeepalive), callbacks(config.callbacks),
          /*path(config.path), */outType(XML_RPC), closeConnection(true),
          queryStorage(new std::list<std::string>()), headerOut(0x0),
          contentLength(0), useChunks(false), headersSent(false), head(false)
    {}

    void serve(int fd, struct sockaddr_in* addr = 0);

    void serve(int fd,
               struct sockaddr_in* addr,
               HTTPHeader_t &headerIn,
               HTTPHeader_t &headerOut);

    ~Server_t();

    MethodRegistry_t &registry() {
        return methodRegistry;
    }

private:
    void readRequest(DataBuilder_t &builder);

    void readRequest(DataBuilder_t &builder,
                     HTTPHeader_t &headersIn);


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
    virtual void write(const char* data, unsigned int size);
    /**
    * @brief send response to client
    *
    */
    void sendResponse(bool last = false);
    /**
    * @brief send HTTP header with HTTPError_t to client
    *
    */
    void sendHttpError(const HTTPError_t &httpError);

    Server_t();

    MethodRegistry_t methodRegistry;
    HTTPIO_t io;
    bool keepAlive;
    bool useBinary;                              //!< allow or disallow binary
    unsigned int maxKeepalive;
    MethodRegistry_t::Callbacks_t *callbacks;
//     std::string path;
    unsigned int outType;
    bool closeConnection;
    std::auto_ptr<std::list<std::string> > queryStorage;
    HTTPHeader_t *headerOut;
    //std::list<std::string> queryStorage;
    //UnMarshaller_t *unmarshaller;
    unsigned int contentLength;
    bool  useChunks;
    bool headersSent;
    bool head;
    ProtocolVersion_t protocolVersion;
};

}

#endif
