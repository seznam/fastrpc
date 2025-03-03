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
 * FILE          $Id: frpctreebuilder.cc,v 1.6 2010-04-21 08:48:03 edois Exp $
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

namespace FRPC {

TreeBuilder_t::~TreeBuilder_t()
{}

void TreeBuilder_t::buildBinary(const char* data, unsigned int size)
{
    Value_t &binary = pool.Binary(const_cast<char*>(data),size);
    if(!isMember(binary))
        if (!isFirst(binary))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildBinary(const std::string& data)
{
    Value_t &binary = pool.Binary(data);
    if(!isMember(binary))
        if (!isFirst(binary))
            throw StreamError_t("Unexpected value after end");

}

void TreeBuilder_t::buildBool(bool value)
{
    Value_t &boolean = pool.Bool(value);
    if(!isMember(boolean))
        if (!isFirst(boolean))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildNull()
{
    Value_t &nullValue = pool.Null();
    if (!isMember(nullValue))
        if (!isFirst(nullValue))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildDateTime(short year, char month, char day, char hour,
                                  char minute, char sec, char weekDay,
                                  time_t unixTime, int timeZone)
{
    Value_t &dateTime = pool.DateTime(year, month, day, hour, minute, sec,
                                      weekDay, unixTime, timeZone);
    if(!isMember(dateTime))
        if (!isFirst(dateTime))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildDouble(double value)
{
    Value_t &doubleVal = pool.Double(value);

    if(!isMember(doubleVal))
        if (!isFirst(doubleVal))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildFault(int errNumber, const char* errMsg,
                               unsigned int size)
{
    this->errNum = errNumber;
    this->errMsg.erase();
    this->errMsg.append(errMsg, size);
    retValue = nullptr;
    first = true;

}

void TreeBuilder_t::buildFault(int errNumber, const std::string& errMsg)
{
    this->errNum = errNumber;
    this->errMsg = errMsg;
    retValue = nullptr;
    first = true;

}

void TreeBuilder_t::buildInt(Int_t::value_type value)
{
    Value_t &integer = pool.Int(value);

    if(!isMember(integer))
        if (!isFirst(integer))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildMethodCall(const char* methodName, unsigned int size)
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

void TreeBuilder_t::buildString(const char* data, unsigned int size)
{
    Value_t &stringVal = pool.String(const_cast<char*>(data), size);

    if(!isMember(stringVal))
        if (!isFirst(stringVal))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildString(const std::string& data)
{

    Value_t &stringVal = pool.String(data);

    if(!isMember(stringVal))
        if (!isFirst(stringVal))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildStructMember(const char* memberName, unsigned int size)
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

void TreeBuilder_t::openArray(unsigned int numOfItems)
{
    Array_t &array = pool.Array();
    array.reserve(numOfItems);

    if(!isMember(array))
        if (!isFirst(array))
            throw StreamError_t("Unexpected value after end");

    entityStorage.push_back(ValueTypeStorage_t(&array,ARRAY));
}

void TreeBuilder_t::openStruct(unsigned int /*numOfMembers*/)
{
    Value_t &structVal = pool.Struct();

    if(!isMember(structVal))
        if (!isFirst(structVal))
            throw StreamError_t("Unexpected value after end");

    entityStorage.push_back(ValueTypeStorage_t(&structVal,STRUCT));
}

}
