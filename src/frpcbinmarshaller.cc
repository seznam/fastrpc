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
 * FILE          $Id: frpcbinmarshaller.cc,v 1.12 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#include <cstring>
#include <cstdlib>
#include <iostream>

#include "frpcbinmarshaller.h"
#include "frpclenerror.h"
#include "frpcinternals.h"

namespace FRPC {
namespace {

template <typename T>
 std::make_unsigned_t<T> safe_abs(T val) {
    using U =  std::make_unsigned_t<T>;
    if (val < 0)
        return static_cast<U>(~val) + 1; // two's complement
    return static_cast<U>(val);
}

uint32_t getNumberSize(const ProtocolVersion_t &protocolVersion, uint32_t size) {
    if (protocolVersion.versionMajor < 2)
        return size;
    return size + 1;
}

uint32_t getNumberType(const ProtocolVersion_t &protocolVersion, Int_t::value_type number) {
    if (protocolVersion.versionMajor < 2) {
        auto oldNumber = int32_t(number);
        // + 1 => old marking
        if (!(oldNumber & OLD_INT8_MASK))
            return CHAR8 + 1;

        if (!(oldNumber & OLD_INT16_MASK))
            return SHORT16 + 1;

        if (!(oldNumber & OLD_INT24_MASK))
            return LONG24 + 1;

        if (!(oldNumber & OLD_INT32_MASK))
            return LONG32 + 1;

        throw StreamError_t("Number is too big for protocol version 1.0");
    } else {
        if (!(number & INT8_MASK))
            return CHAR8;

        if (!(number & INT16_MASK))
            return SHORT16;

        if (!(number & INT24_MASK))
            return LONG24;

        if (!(number & INT32_MASK))
            return LONG32;

        if (!(number & INT40_MASK))
            return LONG40;

        if (!(number & INT48_MASK))
            return LONG48;

        if (!(number & INT56_MASK))
            return LONG56;

        return LONG64;
    }
}

/** Encodes type and info into type tag.
 */
char data_type(TypeTag_t type, uint32_t info) {
    return static_cast<char>(((type & 0x1fu) << 3u) | (info & 0x07u));
}

/** Encodes signed integer as unsigned,
 * with positive values even and negative values odd
 * starting around zero.
 * This saves transfer space and unifies integer encoding.
 * 0 -> 0
 * -1 -> 1
 * 1 -> 2
 * -2 -> 3
 * 2 -> 4
 * ...
 */
uint64_t zigzagEncode(int64_t n) {
    // the right shift has to be arithmetic
    // negative numbers become all binary 1s
    // positive numbers become all binary 0s
    // effectively inverting bits of the result in
    // case of negative number
    // NOLINTNEXTLINE
    return (static_cast<uint64_t>(n) << 1u) ^ static_cast<uint64_t>(n >> 63u);
}

} // namespace

BinMarshaller_t::BinMarshaller_t(Writer_t &writer,
                                 const ProtocolVersion_t &protocolVersion)
        :writer(writer),protocolVersion(protocolVersion) {
    if (protocolVersion.versionMajor > FRPC_MAJOR_VERSION) {
        throw Error_t("Not supported protocol version");
    }
}


BinMarshaller_t::~BinMarshaller_t() = default;


void BinMarshaller_t::packArray(unsigned int numOfItems) {


    //inr type 8,16,24 or 32
    unsigned int numType = getNumberType(protocolVersion, numOfItems);

    //union long and buffer
    Number_t number(numOfItems);

    //pack dataType
    char type = data_type(ARRAY, numType);

    //add DATATYPE to buffer
    writer.write(&type,1);
    //add number (current length)
    writer.write((char*)number.data, getNumberSize(protocolVersion, numType));

}

void BinMarshaller_t::packBinary(const char* value, unsigned int size) {


    //inr type 8,16,24 or 32
    unsigned int numType = getNumberType(protocolVersion, size);

    Number_t dataSize(size);

    //pack dataType
    char type = data_type(BINARY, numType);

    //add DATATYPE to buffer
    writer.write(&type,1);

    writer.write((char*)dataSize.data, getNumberSize(protocolVersion, numType));

    writer.write(value, size);

}

void BinMarshaller_t::packBool(bool value) {
    //buffer

    unsigned char boolean = (value ? 1 : 0);

    //pack dataType
    char type = data_type(BOOL, boolean);

    //add DATATYPE to buffer
    writer.write(&type,1);

}

void BinMarshaller_t::packDateTime(short year, char month, char day, char hour,
                                   char minute, char sec, char weekDay,
                                   time_t unixTime, int timeZone)
{
    // since protocol version 3, we use full timestamp time_t in the packed data
    if (protocolVersion.versionMajor > 2) {
        DateTimeDataV3_t dateTime;
        //pack type
        char type = data_type(DATETIME,0);

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
        dateTime.dateTime.timeZone = static_cast<char>(timeZone / 60 / 15);
        dateTime.pack();
        //write type
        writer.write(&type,1);
        //write data
        writer.write((dateTime.data), sizeof(dateTime.data));
    } else {
        DateTimeData_t dateTime;
        //pack type
        char type = data_type(DATETIME,0);

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
        dateTime.dateTime.timeZone = static_cast<char>(timeZone / 60 / 15);
        dateTime.pack();
        //write type
        writer.write(&type,1);
        //write data
        writer.write((dateTime.data),10);
    }
}

void BinMarshaller_t::packDouble(double value) {
    //pack type
    char type = data_type(DOUBLE, 0);
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

void BinMarshaller_t::packFault(int errNumber, const char* errMsg,
                                unsigned int size) {
    //pact type
    char type = data_type(FAULT, 0);

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
    if (protocolVersion.versionMajor > 2) {
        // pack via zigzag encoding.
        uint64_t zig = zigzagEncode(value);
        unsigned int numType = getNumberType(protocolVersion, static_cast<Int_t::value_type>(zig));
        //pack type
        char type = data_type(INT, numType);
        //pack number value
        Number_t number(zig);

        //write type
        writer.write(&type, 1);
        writer.write(number.data, getNumberSize(protocolVersion, numType));

    } else if (protocolVersion.versionMajor > 1) {

        if (value < 0) { // negative int8
            //obtain int size for compress
            unsigned int numType = getNumberType(protocolVersion, safe_abs(value));
            char type = data_type(INTN8,numType);
            Number_t  number(safe_abs(value));
            //write type
            writer.write(&type,1);
            writer.write(number.data, getNumberSize(protocolVersion, numType));

        } else { //positive int8
            unsigned int numType = getNumberType(protocolVersion, value);
            char type = data_type(INTP8,numType);
            Number_t  number(value);
            //write type
            writer.write(&type,1);
            writer.write(number.data, getNumberSize(protocolVersion, numType));

        }

    } else {
        unsigned int numType = getNumberType(protocolVersion, value);
        //pack type
        char type = data_type(INT,numType);
        //pack number value
        Number32_t  number(value);

        //write type
        writer.write(&type,1);
        writer.write(number.data, getNumberSize(protocolVersion, numType));
    }


}

void BinMarshaller_t::packMethodCall(const char* methodName,
                                     unsigned int  size) {

    //pack type
    char type = data_type(METHOD_CALL,0);

    //check conditions
    if ( size > 255 || size == 0)
        throw LenError_t::format(
            "Lenght of method name is %d not in interval (1-255)", size);

    int8_t realSize = int8_t(size);
    //magic
    packMagic();
    //write type
    writer.write(&type, 1);
    //write  nameSize
    writer.write(reinterpret_cast<char*>(&realSize), 1);
    //write method name
    writer.write(methodName, size);

}

void BinMarshaller_t::packString(const char* value, unsigned int size) {

    // validate bytes
    String_t::validateBytes(value, size);

    //obtain size of number
    int numType = getNumberType(protocolVersion, size);
    //pack type
    char type = data_type(STRING,numType);
    //pack strSize
    Number_t number(size);

    //write type
    writer.write(&type, 1);
    //write packed strSize
    writer.write(number.data, getNumberSize(protocolVersion, numType));
    //write whole string
    writer.write(value, size);
}

void BinMarshaller_t::packStruct(unsigned int  numOfMembers) {
    //obtain size of number
    unsigned int numType = getNumberType(protocolVersion, numOfMembers);
    //pack type
    char type = data_type(STRUCT, numType);
    //pack numOfMembers
    Number_t number(numOfMembers);

    //write type
    writer.write(&type, 1);
    //write packed numOfMembers
    writer.write(number.data, getNumberSize(protocolVersion, numType));
}

void BinMarshaller_t::packStructMember(const char* memberName,
                                       unsigned int size) {


    if (size > 255 || size == 0)
        throw LenError_t::format(
            "Lenght of member name is %d not in interval (1-255)", size);

    //write member name
    int8_t realSize = int8_t(size);
    writer.write(reinterpret_cast<char *>(&realSize), 1);
    //write whole memberName
    writer.write(memberName, size);

}

void BinMarshaller_t::packMethodResponse() {
    //pack type
    char type = data_type(METHOD_RESPONSE, 0);
    //magic
    packMagic();

    //write type
    writer.write(&type, 1);

}

void BinMarshaller_t::packMagic() {

    unsigned char magic[]={0xCA,0x11,0x00,0x00};
    magic[2] = protocolVersion.versionMajor;
    magic[3] = protocolVersion.versionMinor;
    writer.write(reinterpret_cast<const char*>(magic),4);
}

void BinMarshaller_t::flush() {
    writer.flush();
}

void BinMarshaller_t::packNull() {

    if (protocolVersion.versionMajor < 2
        || ( protocolVersion.versionMajor == 2 && protocolVersion.versionMinor < 1) ) {

        throw StreamError_t("Null is not supported by protocol version lower than 2.1");
    }

    char type = data_type(NULLTYPE, 0);

    writer.write(&type, 1);

}

void BinMarshaller_t::packBinaryRef(BinaryRefFeeder_t feeder) {
    auto size = feeder.size();
    unsigned int numType = getNumberType(protocolVersion, size);
    Number_t dataSize(size);
    char type = data_type(BINARY, numType);
    writer.write(&type, 1);
    writer.write((char*)dataSize.data, getNumberSize(protocolVersion, numType));
    while (auto chunk = feeder.next())
        writer.write(reinterpret_cast<const char *>(chunk.data), static_cast<uint32_t>(chunk.size));
}

} // namespace FRPC

