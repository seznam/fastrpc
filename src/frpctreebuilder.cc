/*
 * FILE          $Id: frpctreebuilder.cc,v 1.1 2005-07-19 13:02:54 vasek Exp $
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

void TreeBuilder_t::buildDateTime(short year, char month, char day, char hour, char min, char sec, char weekDay, time_t unixTime, char timeZone)
{
    Value_t &dateTime = pool.DateTime(year, month, day, hour, min, sec,
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
