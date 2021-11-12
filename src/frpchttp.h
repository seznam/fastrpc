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
 * FILE             $Id: frpchttp.h,v 1.4 2007-07-31 13:01:18 vasek Exp $
 *
 * DESCRIPTION      HTTP Base types
 *
 * AUTHOR           Vasek Blazek <blazek@firma.seznam.cz>
 *                  Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *                  Zbynek Novotny <zbynek.novotny@firma.seznam.cz>
 *
 * HISTORY
 *          2005-02-07
 *                  First draft.
 */

#ifndef FRPCHTTP_H
#define FRPCHTTP_H

#include <frpcplatform.h>

#include <string>
#include <vector>
#include <utility>
#include <iosfwd>

#include <frpcsocket.h>


namespace FRPC {
    
    enum HTTPStatus_t {
        HTTP_UNKNOWN = 0,
        
        HTTP_CONTINUE = 100,
        HTTP_SWITCHING_PROTOCOLS = 101,
        
        HTTP_OK = 200,
        HTTP_CREATED = 201,
        HTTP_ACCEPTED = 202,
        HTTP_NON_AUTHORITATIVE_INFORMATION = 203,
        HTTP_NO_CONTENT = 204,
        HTTP_RESET_CONTENT = 205,
        HTTP_PARTIAL_CONTENT = 206,
        
        HTTP_MULTIPLE_CHOICES = 300,
        HTTP_MOVED_PERMANENTLY = 301,
        HTTP_FOUND = 302,
        HTTP_SEE_OTHER = 303,
        HTTP_NOT_MODIFIED = 304,
        HTTP_USE_PROXY = 305,
        HTTP_TEMPORARY_REDIRECT = 307,
        
        HTTP_BAD_REQUEST = 400,
        HTTP_UNAUTHORIZED = 401,
        HTTP_PAYMENT_REQUIRED = 402,
        HTTP_FORBIDDEN = 403,
        HTTP_NOT_FOUND = 404,
        HTTP_METHOD_NOT_ALLOWED = 405,
        HTTP_NOT_ACCEPTABLE = 406,
        HTTP_PROXY_AUTHENTIFICATION_REQUIRED = 407,
        HTTP_REQUEST_TIMEOUT = 408,
        HTTP_CONFLICT = 409,
        HTTP_GONE = 410,
        HTTP_LENGTH_REQUIRED = 411,
        HTTP_PRECONDITION_FAILED = 412,
        HTTP_REQUEST_ENTITY_TOO_LARGE = 413,
        HTTP_REQUEST_URI_TOO_LARGE = 414,
        HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
        HTTP_REQUEST_RANGE_NOT_SATISFIABLE = 416,
        HTTP_EXPECTATION_FAILED = 417,
        
        HTTP_INTERNAL_SERVER_ERROR = 500,
        HTTP_NOT_IMPLEMENTED = 501,
        HTTP_BAD_GATEWAY = 502,
        HTTP_SERVICE_UNAVAILABLE = 503,
        HTTP_GATEWAY_TIMEOUT = 504,
        HTTP_HTTP_VERSION_NOT_SUPPORTED = 505,
    };
    
    const std::string HTTP_HEADER_CONTENT_TYPE("Content-Type");
    const std::string HTTP_HEADER_CONTENT_LENGTH("Content-Length");
    const std::string HTTP_HEADER_TRANSFER_ENCODING("Transfer-Encoding");
    const std::string HTTP_HEADER_REFERER("Referer");
    const std::string HTTP_HEADER_ACCEPT("Accept");
    const std::string HTTP_HEADER_USER_AGENT("User-Agent");
    const std::string HTTP_HEADER_SERVER("Server");
    const std::string HTTP_HEADER_HOST("Host");
    const std::string HTTP_HEADER_DATE("Date");
    const std::string HTTP_HEADER_EXPIRES("Expires");
    const std::string HTTP_HEADER_CONNECTION("Connection");
    const std::string HTTP_HEADER_LAST_MODIFIED("Last-Modified");
    const std::string HTTP_HEADER_IF_MODIFIED_SINCE("If-Modified-Since");
    const std::string HTTP_HEADER_IF_UNMODIFIED_SINCE("If-Unmodified-Since");
    const std::string HTTP_HEADER_COOKIE("Cookie");
    const std::string HTTP_HEADER_SET_COOKIE("Set-Cookie");
    const std::string HTTP_HEADER_P3P("P3P");
    const std::string HTTP_HEADER_LOCATION("Location");
    const std::string HTTP_HEADER_CACHE_CONTROL("Cache-Control");
    const std::string HTTP_HEADER_CACHE_PRAGMA("Pragma");
    const std::string HTTP_HEADER_ALLOW("Allow");
    const std::string HTTP_HEADER_X_FORWARDED_FOR("X-Forwarded-For");
    const std::string HTTP_ACCEPT_RANGES("Accept-Ranges");
    
    class FRPC_DLLEXPORT HTTPHeader_t {
        private:
            typedef std::pair<std::string, std::string> Header_t;
            typedef std::vector<Header_t> HeaderMap_t;

        public:
            typedef HeaderMap_t::const_iterator const_iterator;


        public:
            int get(const std::string &name, std::string &value,
                    unsigned int index = 0) const;
        
            std::vector<std::string> getList(const std::string &name) const;

            void set(const std::string &name, const std::string &value,
                     bool create = true, unsigned int index = 0);
        
            void add(const std::string &name, const std::string &value);
        
            int remove(const std::string &name, unsigned int index = 0);

            void appendValue(const std::string &value);

            inline bool empty() const {
                return header.empty();
            }

            void clear() {
                header.clear();
            }

            friend std::ostream& operator<<(std::ostream &os,
                                            const HTTPHeader_t &header);

            const_iterator begin() const { 
                return header.begin(); 
            }

            const_iterator end() const {
                return header.end();
            }

            inline const HeaderMap_t& getHeaders() const {
                return header;
            }

        private:
            
            HeaderMap_t header;
    };
    
    struct FRPC_DLLEXPORT URL_t {
        URL_t(const std::string &url,
              const std::string &proxyVia = std::string());

        std::string getUrl() const;

        std::string host;

        unsigned short int port;

        std::string path;

        bool sslUsed() const;

        bool isUnix() const;

    private:
        void parse(const std::string &url);
        bool usesProxy;
        bool isSSL;
    };

    enum ErrorCode_t {
        HTTP_TIMEOUT,
        HTTP_SYSCALL,
        HTTP_CLOSED,
        HTTP_VALUE,
        HTTP_DNS,
        HTTP_LINE_TOO_LONG,
        HTTP_BODY_TOO_LONG,
        HTTP_BLOCK_TOO_LONG, // block of input data is too long
        HTTP_NO_REQUEST_SENT, // attempt to read response without sending
                              // request
        HTTP_RESPONSE_NOT_READ, // attempt to send request without reading
                                // response
        HTTP_NO_REQUEST_RECEIVED, // attempted to read request, but connection was closed
                                  // or timed out without receiving anything.
    };
    


        
    

} // namespace FRPC

#endif // FRPCHTTP_H
