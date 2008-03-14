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
 * FILE          $Id: frpcbinmarshaller.cc,v 1.9 2008-03-14 10:29:14 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#include "frpcbinmarshaller.h"
#include <string.h>
#include <frpclenerror.h>
#include "frpcinternals.h"
#include <stdlib.h>

#define FRPC_DATA_TYPE(type,info) (((type & 0x1f)<<3)|(info & 0x07))


namespace FRPC {

BinMarshaller_t::BinMarshaller_t(Writer_t &writer,
                                 const ProtocolVersion_t &protocolVersion)
        :writer(writer),protocolVersion(protocolVersion) {
    if (protocolVersion.versionMajor > FRPC_MAJOR_VERSION) {
        throw Error_t("Not supported protocol version");
    }
}


BinMarshaller_t::~BinMarshaller_t() {}


void BinMarshaller_t::packArray(unsigned int numOfItems) {


    //inr type 8,16,24 or 32
    unsigned int numType = getNumberType(numOfItems);

    //union long and buffer
    Number_t number(numOfItems);

    //pack dataType
    char type = FRPC_DATA_TYPE(ARRAY,numType);

    //add DATATYPE to buffer
    writer.write(&type,1);
    //add number (current length)
    writer.write((char*)number.data, getNumberSize(numType));

}

void BinMarshaller_t::packBinary(const char* value, unsigned int size) {


    //inr type 8,16,24 or 32
    unsigned int numType = getNumberType(size);

    Number_t dataSize(size);

    //pack dataType
    char type = FRPC_DATA_TYPE(BINARY,numType);

    //add DATATYPE to buffer
    writer.write(&type,1);

    writer.write((char*)dataSize.data,getNumberSize(numType));

    writer.write(value,size);

}

void BinMarshaller_t::packBool(bool value) {
    //buffer

    unsigned char boolean = (value ? 1 : 0);

    //pack dataType
    char type = FRPC_DATA_TYPE(BOOL, boolean);

    //add DATATYPE to buffer
    writer.write(&type,1);


}

void BinMarshaller_t::packDateTime(short year, char month, char day, char hour, char minute, char sec, char weekDay, time_t unixTime, char timeZone) {
    DateTimeData_t dateTime;
    //pack type
    char type = FRPC_DATA_TYPE(DATETIME,0);

    //pack DateTimeData
    if ( year <= 1600 ) {
        dateTime.dateTime.year = 0;
    } else {
        dateTime.dateTime.year = (year - 1600) & 0x7ff;
    }
    dateTime.dateTime.month = month & 0x0f;
    dateTime.dateTime.day = day & 0x1f;
    dateTime.dateTime.hour = hour & 0x1f;
    dateTime.dateTime.minute = minute & 0x3f;
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

void BinMarshaller_t::packDouble(double value) {
    //pack type
    char type = FRPC_DATA_TYPE(DOUBLE, 0);
    //write type
    writer.write(&type,1);
    //write data
    char data[8];
    memset(data, 0, 8);
    memcpy(data, (char*)&value, 8);

#ifdef FRPC_BIG_ENDIAN
    //swap it
    SWAP_BYTE(data[7],data[0]);
    SWAP_BYTE(data[6],data[1]);
    SWAP_BYTE(data[5],data[2]);
    SWAP_BYTE(data[4],data[3]);
#endif


    writer.write(data, 8);



}

void BinMarshaller_t::packFault(int errNumber, const char* errMsg, unsigned int size) {
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

void BinMarshaller_t::packInt(Int_t::value_type value) {
    

    if (protocolVersion.versionMajor > 1) {

        if (value < 0) { // negative int8
            //obtain int size for compress
            unsigned int numType = getNumberType(-value);
            char type = FRPC_DATA_TYPE(INTN8,numType);
            Number_t  number(-value);
            //write type
            writer.write(&type,1);
            writer.write(number.data,getNumberSize(numType));

        } else { //positive int8
            unsigned int numType = getNumberType(value);
            char type = FRPC_DATA_TYPE(INTP8,numType);
            Number_t  number(value);
            //write type
            writer.write(&type,1);
            writer.write(number.data,getNumberSize(numType));

        }
        
    } else {
        unsigned int numType = getNumberType(value);
        //pack type
        char type = FRPC_DATA_TYPE(INT,numType);
        //pack number value
        Number32_t  number(value);

        //write type
        writer.write(&type,1);
        writer.write(number.data,getNumberSize(numType));
    }


}

void BinMarshaller_t::packMethodCall(const char* methodName, unsigned int  size) {

    //pack type
    char type = FRPC_DATA_TYPE(METHOD_CALL,0);
    //check conditions
    if ( size > 255 || size == 0)
        throw LenError_t("Lenght of method name is %d not in interval (1-255)",size);
    int8_t realSize = int8_t(size);
    //magic
    packMagic();
    //write type
    writer.write(&type, 1);
    //write  nameSize
    writer.write(reinterpret_cast<char*>(&realSize), 1);
    //write method name
    writer.write(methodName, realSize);

}

void BinMarshaller_t::packString(const char* value, unsigned int size) {

    //obtain size of number
    int numType = getNumberType(size);
    //pack type
    char type = FRPC_DATA_TYPE(STRING,numType);
    //pack strSize
    Number_t number(size);

    //write type
    writer.write(&type, 1);
    //write packed strSize
    writer.write(number.data, getNumberSize(numType));
    //write whole string
    writer.write(value, size);
}

void BinMarshaller_t::packStruct(unsigned int  numOfMembers) {
    //obtain size of number
    unsigned int numType = getNumberType(numOfMembers);
    //pack type
    char type = FRPC_DATA_TYPE(STRUCT, numType);
    //pack numOfMembers
    Number_t number(numOfMembers);

    //write type
    writer.write(&type, 1);
    //write packed numOfMembers
    writer.write(number.data, getNumberSize(numType));


}

void BinMarshaller_t::packStructMember(const char* memberName, unsigned int size) {


    if (size > 255 || size == 0)
        throw LenError_t("Lenght of member name is %d not in interval (1-255)",size);

    //write member name
    int8_t realSize = int8_t(size);
    writer.write(reinterpret_cast<char *>(&realSize), 1);
    //write whole memberName
    writer.write(memberName, realSize);

}

void BinMarshaller_t::packMethodResponse() {
    //pack type
    char type = FRPC_DATA_TYPE(METHOD_RESPONSE, 0);
    //magic
    packMagic();

    //write type
    writer.write(&type, 1);

}

void BinMarshaller_t::packMagic() {
    
    char magic[]={0xCA,0x11,0x00,0x00};
    magic[2] = protocolVersion.versionMajor;
    magic[3] = protocolVersion.versionMinor;
    writer.write(magic,4);

}

void BinMarshaller_t::flush() {
    writer.flush();
}



};
