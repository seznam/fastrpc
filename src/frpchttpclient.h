/*
 * FILE          $Id: frpchttpclient.h,v 1.1 2005-07-19 13:02:53 vasek Exp $
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
#ifndef FRPCFRPCHTTPCLIENT_H
#define FRPCFRPCHTTPCLIENT_H

#include <frpcplatform.h>

#include <frpcwriter.h>
#include <frpchttpio.h>
#include <frpctypeerror.h>
#include <frpcunmarshaller.h>
#include <list>
#include <frpc.h>



namespace FRPC
{

class DataBuilder_t;
class HTTPIO_t;
struct URL_t;
class UnMarshaller_t;


class FRPC_DLLEXPORT DataSink_t
{
public:
    inline DataSink_t(UnMarshaller_t& um,
                      long type = UnMarshaller_t::TYPE_METHOD_RESPONSE )
                      :um(um),dataWritten(0), type(type)
    {}

    inline ~DataSink_t()
    {}
    inline void write(const char *data, long size)
    {
        um.unMarshall(data, size, type);
        dataWritten += size;
    }
    inline unsigned long written()
    {
        return dataWritten;
    }

private:
    UnMarshaller_t &um;
    unsigned long dataWritten;
    long type;
};

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT HTTPClient_t : public Writer_t
{
public:
    
    HTTPClient_t(HTTPIO_t &httpIO, URL_t &url, long connectTimeout,
                 bool keepAlive);

    HTTPClient_t(HTTPIO_t &httpIO, URL_t &url, long connectTimeout,
                 bool keepAlive, bool useHTTP10);


    enum{XML_RPC = 0x01, BINARY_RPC = 0x02};

    virtual ~HTTPClient_t();

    /**
    * @brief says to HTTP Client  which protol be used to send request
    * @param contentType maybe  XML_RPC or BINARY_RPC
    */

    inline void prepare(unsigned long contentType)
    {
        switch(contentType)
        {
        case XML_RPC:
            useProtocol = contentType;
            useChunks = false;
            break;
        case BINARY_RPC:
            useProtocol = contentType;
			useChunks = useHTTP10 ? false : true;
            break;
        default:
            throw TypeError_t("Unknown ContentType");
            break;
        }
    }
    /**
    * @brief getting server support protocols from last response
    * @return maybe XML_RPC , BINARY_RPC or both
    */

    inline unsigned long getSupportedProtocols()
    {
        return supportedProtocols;
    }
    /**
    * @brief says to HTTP client that all data was writed
    *
    */
    virtual void flush();
    /**
    * @brief write data to HTTP client
    * @param data pointer to data 
    * @param size size of data
    */
    virtual void write(const char* data, long size);
    /**
    *@brief read response from socket and unmarshaling
    *@param builder is a builder required for unmarshaller to build data tree
    */
    void readResponse(DataBuilder_t &builder);


private:
    void sendRequest();
    HTTPClient_t();

    HTTPIO_t &httpIO;
    URL_t    &url;
    long connectTimeout;
    bool keepAlive;


    bool headersSent;
    bool useChunks;
    unsigned long supportedProtocols;
    unsigned long useProtocol;
    unsigned long contentLenght;
    bool connectionMustClose;

    std::list<std::string> queryStorage;
    UnMarshaller_t *unmarshaller;
    bool useHTTP10;
};

};

#endif
