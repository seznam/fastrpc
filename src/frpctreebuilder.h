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
 * FILE          $Id: frpctreebuilder.h,v 1.5 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCTREEBUILDER_H
#define FRPCFRPCTREEBUILDER_H

#include <frpcplatform.h>

#include <frpcdatabuilder.h>
#include <frpc.h>
#include <frpcfault.h>
#include <memory>

namespace FRPC {

struct ValueTypeStorage_t {
    ValueTypeStorage_t(Value_t *container, char type)
        :type(type), container(container)
    {}
    ~ValueTypeStorage_t() = default;
    char type;
    Value_t* container;
};

/**
@author Miroslav Talasek
*/
class Pool_t;

class FRPC_DLLEXPORT TreeBuilder_t : public DataBuilder_t {
public:
    TreeBuilder_t(Pool_t &pool)
        : pool(pool), first(true), retValue(nullptr), errNum(-500)
    {}
    enum{ARRAY=0,STRUCT};
    ~TreeBuilder_t() override;

    void buildBinary(const char* data, unsigned int size) override;
    void buildBinary(const std::string& data) override;
    void buildBool(bool value) override;
    void buildDateTime(short year, char month, char day,
                               char hour, char min, char sec, char weekDay,
                                time_t unixTime, int timeZone) override;
    void buildDouble(double value) override;
    void buildFault(int errNumber, const char* errMsg, unsigned int size) override;
    void buildFault(int errNumber, const std::string& errMsg) override;
    void buildInt(Int_t::value_type value) override;
    void buildMethodCall(const char* methodName, unsigned int size) override;
    void buildMethodCall(const std::string& methodName) override;
    void buildMethodResponse() override;
    void buildString(const char* data, unsigned int size) override;
    void buildString(const std::string& data) override;
    void buildStructMember(const char* memberName, unsigned int size) override;
    void buildStructMember(const std::string& memberName) override;
    void closeArray() override;
    void closeStruct() override;
    void openArray(unsigned int numOfItems) override;
    void openStruct(unsigned int numOfMembers) override;
    void buildNull();
    bool isFirst(Value_t  &value) {
        if (first) {
            retValue = &value;
            first = false;
            return true;
        }
        return false;
    }
    bool isMember(Value_t &value ) {
        if (entityStorage.size() < 1)
            return false;
        switch(entityStorage.back().type) {
        case ARRAY:
            dynamic_cast<Array_t*>(entityStorage.back().container)->append(value);
            break;
        case STRUCT:
            dynamic_cast<Struct_t*>(entityStorage.back().container)->
                append(memberName ,value);
            break;
        default:
            //OOPS
            break;

        }
        return true;
    }

    Value_t& getUnMarshaledData() {
        if (!retValue)
            throw Fault_t(static_cast<int>(getUnMarshaledErrorNumber()),
                          getUnMarshaledErrorMessage());
        return *retValue;
    }

    Value_t* getUnMarshaledDataPtr() {return retValue;}

    const std::string getUnMarshaledMethodName() {return methodName;}

    const std::string getUnMarshaledErrorMessage() {
        if (errMsg.size() != 0)
            return errMsg;
        return "No data unmarshalled";
    }

    long getUnMarshaledErrorNumber() {
        return errNum;
    }

protected:
    Pool_t &pool;
    bool first;
    Value_t *retValue;
    std::string memberName;
    std::string methodName;
    int errNum;
    std::string errMsg;
    std::vector<ValueTypeStorage_t> entityStorage;
};

class FRPC_DLLEXPORT ExtTreeBuilder_t : public DataBuilder_t {
public:
    ExtTreeBuilder_t(Pool_t &pool, const std::vector<std::string> &secrets);

    ~ExtTreeBuilder_t() override;

    void buildNull();
    void buildInt(Int_t::value_type value) override;
    void buildBool(bool value) override;
    void buildDouble(double value) override;
    void buildString(const char *data, unsigned int size) override;
    void buildString(const std::string &data) override {
        buildString(data.data(), static_cast<unsigned int>(data.size()));
    }
    void buildBinary(const char *data, unsigned int size) override;
    void buildBinary(const std::string &data) override {
        buildBinary(data.data(), static_cast<unsigned int>(data.size()));
    }
    void buildDateTime(short year, char month, char day, char hour, char min, char sec, char weekDay, time_t unixTime, int timeZone) override;

    void openArray(unsigned int numOfItems) override;
    void closeArray() override;

    void openStruct(unsigned int numOfMembers) override;
    void closeStruct() override;

    void buildStructMember(const char *name, unsigned int size) override;
    void buildStructMember(const std::string &name) override {
        buildStructMember(name.c_str(), static_cast<unsigned int>(name.size()));
    }

    void buildFault(int code, const char *msg, unsigned int size) override;
    void buildFault(int code, const std::string &msg) override {
        buildFault(code, msg.c_str(), static_cast<unsigned int>(msg.size()));
    }

    void buildMethodCall(const char *name, unsigned int size) override;
    void buildMethodCall(const std::string &name) override {
        buildMethodCall(name.c_str(), static_cast<unsigned int>(name.size()));
    }

    void buildMethodResponse() override;

    std::pair<std::string, Array_t *> getMethodCall() const;
    Value_t *getMethodResponse() const;
    Fault_t *getFault() const;

protected:
    struct Pimpl_t;
    std::unique_ptr<Pimpl_t> pimpl;
};

} // namespace FRPC

#endif
