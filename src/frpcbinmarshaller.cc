/*
 * FILE          $Id: frpcbinmarshaller.cc,v 1.1 2005-07-19 13:02:53 vasek Exp $
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
#include "frpcbinmarshaller.h"
#include <string.h>
#include <frpclenerror.h>
#include <frpcinternals.h>


#define FRPC_DATA_TYPE(type,info) (((type & 0x1f)<<3)|(info & 0x07))


namespace FRPC
{


BinMarshaller_t::~BinMarshaller_t()
{}


void BinMarshaller_t::packArray(long numOfItems)
{


    //inr type 8,16,24 or 32
    unsigned int numType = getNumberSize(numOfItems);

    //union long and buffer
    Number_t number(numOfItems);

    //pack dataType
    char type = FRPC_DATA_TYPE(ARRAY,numType);

    //add DATATYPE to buffer
    writer.write(&type,1);
    //add number (current length)
    writer.write((char*)number.data, numType);

}

void BinMarshaller_t::packBinary(const char* value, long size)
{


    //inr type 8,16,24 or 32
    unsigned int sizeNumType = getNumberSize(size);

    Number_t dataSize(size);

    //pack dataType
    char type = FRPC_DATA_TYPE(BINARY,sizeNumType);

    //add DATATYPE to buffer
    writer.write(&type,1);

    writer.write((char*)dataSize.data,sizeNumType);

    writer.write(value,size);

}

void BinMarshaller_t::packBool(bool value)
{
    //buffer

    unsigned char boolean = (value==true)?1:0;

    //pack dataType
    char type = FRPC_DATA_TYPE(BOOL,boolean);

    //add DATATYPE to buffer
    writer.write(&type,1);


}

void BinMarshaller_t::packDateTime(short year, char month, char day, char hour, char min, char sec, char weekDay, time_t unixTime, char timeZone)
{
    DateTimeData_t dateTime;
    //pack type
    char type = FRPC_DATA_TYPE(DATETIME,0);

    //pack DateTimeData
    if ( year <= 1600 )
    {
        dateTime.dateTime.year = 0;
    }
    else
    {
        dateTime.dateTime.year = (year - 1600) & 0x7ff;
    }
    dateTime.dateTime.month = month & 0x0f;
    dateTime.dateTime.day = day & 0x1f;
    dateTime.dateTime.hour = hour & 0x1f;
    dateTime.dateTime.min = min & 0x3f;
    dateTime.dateTime.sec = sec & 0x3f;

    dateTime.dateTime.weekDay = weekDay & 0x07;
    dateTime.dateTime.unixTime = unixTime;
    dateTime.dateTime.timeZone = timeZone;
    dateTime.pack();
    //write type
    writer.write(&type,1);
    //write data
    writer.write((dateTime.data),10);
}

void BinMarshaller_t::packDouble(double value)
{
    //pack type
    char type = FRPC_DATA_TYPE(DOUBLE, 0);
    //write type
    writer.write(&type,1);
    //write data
    writer.write((char*)&value, 8);



}

void BinMarshaller_t::packFault(long errNumber, const char* errMsg, long size)
{
    //pact type
    char type = FRPC_DATA_TYPE(FAULT, 0);

    //magic
    packMagic();
    //write type
    writer.write(&type,1);

    //pack and write errNumber
    packInt(errNumber);
    //pack error msg
    packString(errMsg, size);

}

void BinMarshaller_t::packInt(long value)
{
    //obtain int size for compress
    int numSize = getNumberSize(value);
    //pack type
    char type = FRPC_DATA_TYPE(INT,numSize);
    //pack number value
    Number_t  number(value);

    //write type
    writer.write(&type,1);
    writer.write(number.data,numSize);
}

void BinMarshaller_t::packMethodCall(const char* methodName, long size)
{
    long nameSize;
    //obtain a right lenght of string
    nameSize = (size == -1)? strlen(methodName):size;
    //pack type
    char type = FRPC_DATA_TYPE(METHOD_CALL,0);
    //check conditions
    if( size > 255 || nameSize == 0)
        throw LenError_t("Lenght of method name is %d not in interval (1-255)",nameSize);

    //magic
    packMagic();
    //write type
    writer.write(&type, 1);
    //write  nameSize
    writer.write(reinterpret_cast<char*>(&nameSize), 1);
    //write method name
    writer.write(methodName, nameSize);

}

void BinMarshaller_t::packString(const char* value, long size)
{
    long strSize;
    //obtain a right lenght of string
    strSize = (size == -1)? strlen(value):size;
    //obtain size of number
    int numSize = getNumberSize(strSize);
    //pack type
    char type = FRPC_DATA_TYPE(STRING,numSize);
    //pack strSize
    Number_t number(strSize);

    //write type
    writer.write(&type, 1);
    //write packed strSize
    writer.write(number.data, numSize);
    //write whole string
    writer.write(value, strSize);
}

void BinMarshaller_t::packStruct(long numOfMembers)
{
    //obtain size of number
    int numSize = getNumberSize(numOfMembers);
    //pack type
    char type = FRPC_DATA_TYPE(STRUCT, numSize);
    //pack numOfMembers
    Number_t number(numOfMembers);

    //write type
    writer.write(&type, 1);
    //write packed numOfMembers
    writer.write(number.data, numSize);


}

void BinMarshaller_t::packStructMember(const char* memberName, long size)
{
    long nameSize;
    //obtain a right lenght of string
    nameSize = (size == -1)? strlen(memberName):size;

    if(size > 255 || nameSize == 0)
        throw LenError_t("Lenght of member name is %d not in interval (1-255)",nameSize);

    //write member name
    writer.write(reinterpret_cast<char *>(&nameSize), 1);
    //write whole memberName
    writer.write(memberName, nameSize);

}

void BinMarshaller_t::packMethodResponse()
{
    //pack type
    char type = FRPC_DATA_TYPE(METHOD_RESPONSE, 0);
    //magic
    packMagic();

    //write type
    writer.write(&type, 1);

}

void BinMarshaller_t::packMagic()
{
    char magic[]={0xCA,0x11,FRPC_MAJOR_VERSION,FRPC_MINOR_VERSION};
    writer.write(magic,4);

}

void BinMarshaller_t::flush()
{
    writer.flush();
}



};
