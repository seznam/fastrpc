/*
 * FILE          $Id: frpcdatabuilder.h,v 1.2 2005-07-25 06:10:47 vasek Exp $
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
#ifndef FRPCFRPCDATABUILDER_H
#define FRPCFRPCDATABUILDER_H

#include <frpcplatform.h>

#include <time.h>
#include <string>
namespace FRPC
{

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT DataBuilder_t
{
public:
    DataBuilder_t();
    virtual ~DataBuilder_t();

    virtual void buildMethodResponse() = 0;
    virtual void buildBinary(const char* data, long size) = 0;
    virtual void buildBinary(const std::string &data) = 0;
    virtual void buildBool(bool value) = 0;
    virtual void buildDateTime(short year, char month, char day,char hour, char minute, char sec,
                               char weekDay, time_t unixTime, char timeZone) = 0;
    virtual void buildDouble(double value) = 0;
    virtual void buildFault(long errNumber, const char* errMsg, long size = -1) = 0;
    virtual void buildFault(long errNumber, const std::string &errMsg) = 0;
    virtual void buildInt(long value) = 0;
    virtual void buildMethodCall(const char* methodName, long size = -1) = 0;
    virtual void buildMethodCall(const std::string &methodName) = 0;
    virtual void buildString(const char* data, long size = -1) = 0;
    virtual void buildString(const std::string &data) = 0;
    virtual void buildStructMember(const char *memberName, long size = -1) = 0;
    virtual void buildStructMember(const std::string &memberName) = 0;
    virtual void closeArray() = 0;
    virtual void closeStruct() = 0;
    virtual void openArray(long numOfItems) = 0;
    virtual void openStruct(long numOfMembers) = 0;



};

};

#endif
