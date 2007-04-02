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
 * FILE          $Id: frpcmarshaller.h,v 1.2 2007-04-02 15:28:20 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCMARSHALLER_H
#define FRPCFRPCMARSHALLER_H

#include <frpcplatform.h>

#include <string>

namespace FRPC
{

/**
@brief Abstract Marshaller Class
@author Miroslav Talasek
*/

class Writer_t;
class FRPC_DLLEXPORT Marshaller_t
{
public:
    enum{BINARY_RPC,XML_RPC};
    
    /**
        @brief Default constructor
    */
    Marshaller_t();
    /**
        @brief Default destructor
    */
    virtual ~Marshaller_t();
    /**
        @brief Marshall a sturct  member
        @param memberName pointer to struct's member name must ending with special character "\0"
        @param size size of member name. If size is -1 method obtain size with using strlen()
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */  
    virtual void packStructMember(const char* memberName, long size = -1) = 0;
    /**
        @brief Marshall an arrray type
        @param numOfItems  - is count of items in array type
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */ 
    virtual void packArray(long numOfItems) = 0;
    /**
        @brief Marshall a binary  type
        @param value pointer to binary data
        @param size size of binary data
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */   
    virtual void packBinary(const char* value, long size) = 0;
    /**
        @brief Marshall an bool type
        @param value  - is a boolean value which be marshalled
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */
    virtual void packBool(bool value) = 0;
    /**
        @brief Marshall an datetime type
        @param year - Year is offset (0 - 2047) to zero-year 1600 
        (0=1600, ... 370 = 1970, ...)
        @param month - Month is 1 - 12
        @param day -   Day is 1 - 31
        @param hour -  Hour is 0 - 23
        @param min -   Minute is 0 - 59
        @param sec -   Second is 0 - 59
        @param weekDay  - day of week if exists else -1
        @param unixTime - unix timestamp if exists else -1
        @param timeZone - timezone if exits else -1
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */
        
    virtual void packDateTime(short year, char month, char day,char hour, char min,  char sec,
                              char weekDay, time_t unixTime, char timeZone ) = 0;
    /**
        @brief Marshall a double type
        @param value -is a double value which be marshalled
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)  
    */
    virtual void packDouble(double value) = 0;
    /**
        @brief Marshall an int type
        @param value -is a long value which be marshalled
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)  
    */
    virtual void packInt(long value) = 0;
    /**
        @brief Marshall a string  type
        @param value pointer to string data must ending with special character "\0"
        @param size size of string data. If size is -1 method obtain size with using strlen()
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */
    virtual void packString(const char* value, long size = -1) = 0;
    /**
        @brief Marshall an struct type
        @param numOfMembers  - is count of members in struct type
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */
    virtual void packStruct(long numOfMembers) = 0;
    /**
        @brief Marshall a fault message 
        @param errNumber is error number 
        @param errMsg pointer to message must ending with special character "\0"
        @param size size of message. If size is -1 method obtain size with using strlen()
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */
    virtual void packFault(long errNumber, const char* errMsg, long size = -1) = 0;
    /**
        @brief Marshall a method call
        @param methodName pointer to method name must ending with special character "\0"
        @param size size of message. If size is -1 method obtain size with using strlen()
        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */
    virtual void packMethodCall(const char* methodName, long size = -1) = 0;
   /**
        @brief Marshall a method response

        
        If marshaller is create as binary  using method Binary(FastRPC)
        or if created as XML using method XML(Xml-RPC)
    */    
    virtual void packMethodResponse() = 0;
    /**
        @brief flush data
        
        End of write any data and ready to send
    */
    virtual void flush() = 0;
    /**
        @brief create marshaller object with contentType
        @param contentType is an content type 
            @li @b XML - create xml marshaller
            @li @b BINARY create binary marshaler
        @param writer is a object which write output data
        @return reference to new marshaller
    
    */  
    static Marshaller_t* create(long contentType, Writer_t& writer);
    


};

};

#endif
