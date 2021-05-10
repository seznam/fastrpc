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
 * FILE          $Id: frpchttpclient.h,v 1.7 2010-05-10 08:39:33 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
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
#include <frpcconnector.h>
#include <list>
#include <frpc.h>
#include <sstream>



namespace FRPC
{

class DataBuilder_t;
class HTTPIO_t;
struct URL_t;
class UnMarshaller_t;


class FRPC_DLLEXPORT DataSink_t {
public:
    inline DataSink_t(UnMarshaller_t& um,
                      unsigned int type = UnMarshaller_t::TYPE_METHOD_RESPONSE)
        : um(um),dataWritten(0), type(type)
    {}

    inline ~DataSink_t() {}

    inline void write(const char *data, unsigned int size) {
        um.unMarshall(data, size, static_cast<char>(type));
        dataWritten += size;
    }

    inline unsigned int written() {
        return dataWritten;
    }

private:
    UnMarshaller_t &um;
    unsigned int dataWritten;
    unsigned int type;
};


class FRPC_DLLEXPORT HTTPClient_t : public Writer_t {
public:
    HTTPClient_t(HTTPIO_t &httpIO, URL_t &url, Connector_t *connector,
                 bool useHTTP10 = false);
    HTTPClient_t(HTTPIO_t &httpIO, URL_t &url, Connector_t *connector,
                 bool useHTTP10, bool useChunks);


    enum {XML_RPC = 0x01, BINARY_RPC = 0x02};

    virtual ~HTTPClient_t();

    /**
    * @brief says to HTTP Client  which protol be used to send request
    * @param contentType maybe  XML_RPC or BINARY_RPC
    */

    inline void prepare(unsigned int contentType) {
        switch(contentType) {
        case XML_RPC:
            useProtocol = contentType;
            useChunks = false;
            break;

        case BINARY_RPC:
            useProtocol = contentType;
            useChunks = useChunks && !useHTTP10;
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
    inline unsigned int getSupportedProtocols() {
        return supportedProtocols;
    }

    /**
    * @brief getting server protocol version
    * @return ProtocolVersion_t
    */
    inline ProtocolVersion_t getProtocolVersion() {
        return protocolVersion;
    }

    /**
    * @brief says to HTTP client that all data was writed
    *
    */
    virtual void flush();

    /**
     * @brief tries to poll data before actual read, can be skipped
     */
    inline void waitOnReadyRead(){httpIO.waitOnReadyRead();};

    /**
    * @brief write data to HTTP client
    * @param data pointer to data
    * @param size size of data
    */
    virtual void write(const char* data, unsigned int size);

    /**
    *@brief read response from socket and unmarshaling
    *@param builder is a builder required for unmarshaller to build data tree
    */
    void readResponse(DataBuilder_t &builder);

    static const std::string HOST;
    static const std::string POST;
    static const std::string HTTP10;
    static const std::string HTTP11;
    static const std::string TYPE_XML;
    static const std::string TYPE_FRPC;
    static const std::string ACCEPTED;
    static const std::string CLOSE;
    static const std::string KEEPALIVE;

    typedef std::pair<std::string, std::string> Header_t;
    typedef std::vector<Header_t> HeaderVector_t;

    template <typename T>
    void addCustomRequestHeader(const std::string& name, const T& value);
    void addCustomRequestHeader(const Header_t& header);
    void addCustomRequestHeader(const HeaderVector_t& headers);
    void deleteCustomRequestHeaders();

private:
    void sendRequest(bool last = false );
    HTTPClient_t();
    HTTPClient_t(const HTTPClient_t&);
    HTTPClient_t& operator=(const HTTPClient_t&);

    HTTPIO_t &httpIO;
    URL_t &url;
    Connector_t *connector;

    bool headersSent;
    bool useChunks;
    unsigned int supportedProtocols;
    unsigned int useProtocol;
    unsigned int contentLenght;
    bool connectionMustClose;

    std::list<std::string> queryStorage;
    UnMarshaller_t *unmarshaller;
    bool useHTTP10;
    ProtocolVersion_t protocolVersion;

    std::ostringstream m_customRequestHeaders;
};

} // namespace FRPC

#endif
