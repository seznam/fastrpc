/*
 * FILE          $Id: frpcbinmarshaller.h,v 1.1 2005-07-19 13:02:53 vasek Exp $
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
    virtual void packDateTime(short year, char month, char day, char hour, char min, char sec,
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
