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
 * FILE          $Id: frpctreebuilder.cc,v 1.3 2007-04-02 15:28:20 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#include "frpctreebuilder.h"
#include <frpcpool.h>
namespace FRPC
{




TreeBuilder_t::~TreeBuilder_t()
{
    
}


void TreeBuilder_t::buildBinary(const char* data, long size)
{
    Value_t &binary = pool.Binary(const_cast<char*>(data),size);
    if(!isMember(binary))
        isFirst(binary);
}

void TreeBuilder_t::buildBinary(const std::string& data)
{
    Value_t &binary = pool.Binary(data);
    if(!isMember(binary))
        isFirst(binary);

}

void TreeBuilder_t::buildBool(bool value)
{
    Value_t &boolean = pool.Bool(value);
    if(!isMember(boolean))
        isFirst(boolean);

}

void TreeBuilder_t::buildDateTime(short year, char month, char day, char hour, char minute, char sec, char weekDay, time_t unixTime, char timeZone)
{
    Value_t &dateTime = pool.DateTime(year, month, day, hour, minute, sec,
                                      weekDay, unixTime, timeZone);
    if(!isMember(dateTime))
        isFirst(dateTime);

}

void TreeBuilder_t::buildDouble(double value)
{
    Value_t &doubleVal = pool.Double(value);

    if(!isMember(doubleVal))
        isFirst(doubleVal);
}

void TreeBuilder_t::buildFault(long errNumber, const char* errMsg, long size)
{
    this->errNum = errNumber;
    this->errMsg.erase();
    this->errMsg.append(errMsg, size);
    retValue = 0;
    first = true;

}

void TreeBuilder_t::buildFault(long errNumber, const std::string& errMsg)
{
    this->errNum = errNumber;
    this->errMsg = errMsg;
    retValue = 0;
    first = true;

}

void TreeBuilder_t::buildInt(long value)
{
    Value_t &integer = pool.Int(value);

    if(!isMember(integer))
        isFirst(integer);

}

void TreeBuilder_t::buildMethodCall(const char* methodName, long size)
{
    this->methodName.erase();
    this->methodName.append(methodName, size);

    Value_t &array = pool.Array();

    retValue = &array;
    first = true;
    entityStorage.push_back(ValueTypeStorage_t(&array,ARRAY));

}

void TreeBuilder_t::buildMethodCall(const std::string& methodName)
{

    this->methodName = methodName;

    Value_t &array = pool.Array();

    retValue = &array;
    first = true;
    entityStorage.push_back(ValueTypeStorage_t(&array,ARRAY));

}

void TreeBuilder_t::buildMethodResponse()
{}

void TreeBuilder_t::buildString(const char* data, long size)
{
    Value_t &stringVal = pool.String(const_cast<char*>(data), size);

    if(!isMember(stringVal))
        isFirst(stringVal);
}

void TreeBuilder_t::buildString(const std::string& data)
{

    Value_t &stringVal = pool.String(data);

    if(!isMember(stringVal))
        isFirst(stringVal);
}

void TreeBuilder_t::buildStructMember(const char* memberName, long size)
{
    this->memberName.erase();
    this->memberName.append(memberName,size);
}

void TreeBuilder_t::buildStructMember(const std::string& memberName)
{
    this->memberName = memberName;

}

void TreeBuilder_t::closeArray()
{
    entityStorage.pop_back();
}

void TreeBuilder_t::closeStruct()
{
    entityStorage.pop_back();
}

void TreeBuilder_t::openArray(long numOfItems)
{
    Value_t &array = pool.Array();

    if(!isMember(array))
        isFirst(array);

    entityStorage.push_back(ValueTypeStorage_t(&array,ARRAY));

}

void TreeBuilder_t::openStruct(long numOfMembers)
{
    Value_t &structVal = pool.Struct();
    
    if(!isMember(structVal))
        isFirst(structVal);
        
    entityStorage.push_back(ValueTypeStorage_t(&structVal,STRUCT));

}

}
;
