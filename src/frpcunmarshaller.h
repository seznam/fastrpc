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
 * FILE          $Id: frpcunmarshaller.h,v 1.4 2011-01-10 22:25:15 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCUNMARSHALLER_H
#define FRPCFRPCUNMARSHALLER_H

#include <frpcplatform.h>
#include <frpc.h>
namespace FRPC {

class DataBuilder_t;


/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT UnMarshaller_t {

public:
    enum {
        TYPE_METHOD_CALL=13,
        TYPE_METHOD_RESPONSE,
        TYPE_FAULT,
        TYPE_ANY
    };

    enum {
        BINARY_RPC,
        XML_RPC,
        URL_ENCODED,
    };

    UnMarshaller_t();

    virtual ~UnMarshaller_t();

    /**
        @brief unmarshall data
        @param data is input data 
        
        @param size is size of input data
        @param type expected main data type 
            @li @b TYPE_METHOD_CALL - method call
            @li @b TYPE_METHOD_RESPONSE - method response
            @li @b TYPE_ANY - any

    */
    virtual void unMarshall(const char *data, unsigned int size, char type) = 0;

    /**
        @brief create marshaller object with contentType
        @param contentType is an content type 
            @li @b XML_RPC - create xml marshaller
            @li @b BINARY_RPC create binary marshaler
        
        @param dataBuilder is abstract object used to build data tree
        @return reference to new unMarshaller

    */
    static UnMarshaller_t* create(unsigned int contentType,
                                  DataBuilder_t& dataBuilder);

    /**
        @brief create marshaller object with contentType
        @param contentType is an content type 
            @li @b XML_RPC - create xml marshaller
            @li @b BINARY_RPC create binary marshaler
        
        @param dataBuilder is abstract object used to build data tree
        @param path uri path
        @return reference to new unMarshaller

    */
    static UnMarshaller_t* create(unsigned int contentType,
                                  DataBuilder_t& dataBuilder,
                                  const std::string &path);

    /**
        @brief create marshaller object from first container of data min 4 bytes
        @param data pointer to data which be unmarshalled
        @param size is data size min 4 bytes
        @param dataBuilder is abstract object used to build data tree
        @return reference to new unMarshaller

    */
    static UnMarshaller_t* create(const char* data, unsigned int size,
                                  DataBuilder_t& dataBuilder);
    /**
        @brief finishing unmarshalling  
    */
    virtual void finish() = 0;

    /**
    @brief get actual protocol version 
    */
    virtual ProtocolVersion_t getProtocolVersion() {
        return ProtocolVersion_t();
    }

};

};

#endif
