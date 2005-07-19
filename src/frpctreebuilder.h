/*
 * FILE          $Id: frpctreebuilder.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
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
#ifndef FRPCFRPCTREEBUILDER_H
#define FRPCFRPCTREEBUILDER_H

#include <frpcplatform.h>

#include <frpcdatabuilder.h>
#include <frpc.h>


namespace FRPC
{
struct ValueTypeStorage_t
{

    ValueTypeStorage_t(Value_t *container, char type):type(type),container(container)
    {}
    ~ValueTypeStorage_t()
    {}
    char type;
    Value_t* container;
};

/**
@author Miroslav Talasek
*/
class Pool_t;

class FRPC_DLLEXPORT TreeBuilder_t : public DataBuilder_t
{
public:
    TreeBuilder_t(Pool_t &pool):DataBuilder_t(), pool(pool),first(true),retValue(0),errNum(-500)
    {}
    enum{ARRAY=0,STRUCT};
    virtual ~TreeBuilder_t();

    virtual void buildBinary(const char* data, long size);
    virtual void buildBinary(const std::string& data);
    virtual void buildBool(bool value);
    virtual void buildDateTime(short year, char month, char day, char hour, char min, char sec, char weekDay, time_t unixTime, char timeZone);
    virtual void buildDouble(double value);
    virtual void buildFault(long errNumber, const char* errMsg, long size);
    virtual void buildFault(long errNumber, const std::string& errMsg);
    virtual void buildInt(long value);
    virtual void buildMethodCall(const char* methodName, long size);
    virtual void buildMethodCall(const std::string& methodName);
    virtual void buildMethodResponse();
    virtual void buildString(const char* data, long size);
    virtual void buildString(const std::string& data);
    virtual void buildStructMember(const char* memberName, long size);
    virtual void buildStructMember(const std::string& memberName);
    virtual void closeArray();
    virtual void closeStruct();
    virtual void openArray(long numOfItems);
    virtual void openStruct(long numOfMembers);
    inline bool isFirst( Value_t  &value )
    {
        if(first)
        {
            retValue = &value;
            first = false;
            return true;
        }
        return false;
    }
    inline bool isMember(Value_t &value )
    {
        if(entityStorage.size() < 1)
            return false;
        switch(entityStorage.back().type)
        {
        case ARRAY:
            {

                dynamic_cast<Array_t*>(entityStorage.back().container)->append(value);

                //entityStorage.back().numOfItems--;
            }
            break;
        case STRUCT:
            {
                dynamic_cast<Struct_t*>(entityStorage.back().container)->append(memberName ,value);


                //entityStorage.back().numOfItems--;

            }
            break;
        default:
            //OOPS
            break;

        }
        /*if(entityStorage.back().numOfItems == 0)
        entityStorage.pop_back();*/
        return true;
    }
    inline Value_t& getUnMarshaledData()
    {
        return *retValue;
    }
    inline const std::string getUnMarshaledMethodName()
    {
        return methodName;
    }
    inline const std::string getUnMarshaledErrorMessage()
    {
        if(errMsg.size() != 0)
            return errMsg;
        else
            return "No data unmarshalled";
    }
    inline long getUnMarshaledErrorNumber()
    {
        return errNum;
    }

private:
    Pool_t &pool;
    bool first;
    Value_t *retValue;
    std::string memberName;
    std::string methodName;
    long errNum;
    std::string errMsg;
    std::vector<ValueTypeStorage_t> entityStorage;
};

};

#endif
