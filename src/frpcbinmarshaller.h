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
 * FILE          $Id: frpcbinmarshaller.h,v 1.4 2007-04-02 15:28:21 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCBINMARSHALLER_H
#define FRPCFRPCBINMARSHALLER_H

#include <frpcmarshaller.h>
#include <vector>
#include <frpcinternals.h>
#include <frpcwriter.h>

namespace FRPC
{




/**
@brief Binary Marshaller (FastRPC)
@author Miroslav Talasek
*/
class BinMarshaller_t : public Marshaller_t
{
public:
    BinMarshaller_t(Writer_t &writer):writer(writer)
    {}

    virtual ~BinMarshaller_t();

    virtual void packArray(long numOfItems);
    virtual void packBinary(const char* value, long size);
    virtual void packBool(bool value);
    virtual void packDateTime(short year, char month, char day, char hour,
                              char minute, char sec,
                              char weekDay, time_t unixTime, char timeZone);
    virtual void packDouble(double value);
    virtual void packFault(long errNumber, const char* errMsg, long size);
    virtual void packInt(long value);
    virtual void packMethodCall(const char* methodName, long size);
    virtual void packString(const char* value, long size);
    virtual void packStruct(long numOfMembers);
    virtual void packStructMember(const char* memberName, long size);
    virtual void packMethodResponse();
    virtual void flush();

private:

    BinMarshaller_t();  

    inline unsigned char getNumberSize(unsigned long number)
    {
        
        if(!(number & 0xffffff00))
            return CHAR8;

        if(!(number & 0xffff0000))
            return SHORT16;

        if(!(number & 0xff000000))
            return LONG24;

        return LONG32;
    }
    
    void packMagic();

    //vector<TypeStorage_t> vectEntity; //not used
    Writer_t  &writer;
};

}

#endif
