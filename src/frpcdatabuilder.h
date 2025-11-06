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
 * FILE          $Id: frpcdatabuilder.h,v 1.6 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#ifndef FRPCFRPCDATABUILDER_H
#define FRPCFRPCDATABUILDER_H

#include <frpcplatform.h>
#include <frpcint.h>
#include <time.h>
#include <string>

namespace FRPC {

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT DataBuilder_t
{
public:
    DataBuilder_t();
    virtual ~DataBuilder_t();

    virtual void buildMethodResponse() = 0;
    virtual void buildBinary(const char* data, unsigned int size) = 0;
    virtual void buildBinary(const std::string &data) = 0;
    virtual void buildBool(bool value) = 0;
    virtual void buildDateTime(short year, char month, char day,char hour,
                               char minute, char sec, char weekDay,
                               time_t unixTime, int timeZone) = 0;
    virtual void buildDouble(double value) = 0;
    virtual void buildFault(int errNumber, const char* errMsg,
                            unsigned int size ) = 0;
    void buildFault(int errNumber, const char* errMsg);
    virtual void buildFault(int errNumber, const std::string &errMsg) = 0;
    virtual void buildInt(Int_t::value_type value) = 0;
    virtual void buildMethodCall(const char* methodName, unsigned int size) = 0;
    void buildMethodCall(const char* methodName);
    virtual void buildMethodCall(const std::string &methodName) = 0;
    virtual void buildString(const char* data, unsigned int size) = 0;
    void buildString(const char* data);
    virtual void buildString(const std::string &data) = 0;
    virtual void buildStructMember(const char *memberName,
                                   unsigned int size) = 0;
    void buildStructMember(const char *memberName);
    virtual void buildStructMember(const std::string &memberName) = 0;
    virtual void closeArray() = 0;
    virtual void closeStruct() = 0;
    virtual void openArray(unsigned int numOfItems) = 0;
    virtual void openStruct(unsigned int numOfMembers) = 0;

};

class FRPC_DLLEXPORT DataBuilderWithNull_t : public DataBuilder_t
{
public:
    DataBuilderWithNull_t();
    virtual ~DataBuilderWithNull_t();
    virtual void buildMethodResponse() = 0;
    virtual void buildBinary(const char* data, unsigned int size) = 0;
    virtual void buildBinary(const std::string &data) = 0;
    virtual void buildBool(bool value) = 0;
    virtual void buildDateTime(short year, char month, char day,char hour,
                               char minute, char sec, char weekDay,
                               time_t unixTime, int timeZone) = 0;
    virtual void buildDouble(double value) = 0;
    virtual void buildFault(int errNumber, const char* errMsg,
                            unsigned int size ) = 0;
    virtual void buildFault(int errNumber, const std::string &errMsg) = 0;
    virtual void buildInt(Int_t::value_type value) = 0;
    virtual void buildMethodCall(const char* methodName, unsigned int size) = 0;
    virtual void buildMethodCall(const std::string &methodName) = 0;
    virtual void buildString(const char* data, unsigned int size) = 0;
    virtual void buildString(const std::string &data) = 0;
    virtual void buildStructMember(const char *memberName,
                                   unsigned int size) = 0;
    virtual void buildStructMember(const std::string &memberName) = 0;
    virtual void closeArray() = 0;
    virtual void closeStruct() = 0;
    virtual void openArray(unsigned int numOfItems) = 0;
    virtual void openStruct(unsigned int numOfMembers) = 0;
    virtual void buildNull() = 0;
};

}

#endif
