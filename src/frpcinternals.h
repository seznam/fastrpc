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
 * FILE          $Id: frpcinternals.h,v 1.4 2007-04-02 15:28:20 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#ifndef FRPCINTERNALS_H
#define FRPCINTERNALS_H
#include <memory.h>
#include <strstream>

#define FRPC_MAJOR_VERSION 1
#define FRPC_MINOR_VERSION 0
#define FRPC_LITTLE_ENDIAN(data)
namespace FRPC
{

enum{NONE=0, INT=1,BOOL,DOUBLE,STRING,DATETIME,BINARY,
     STRUCT=10,ARRAY,METHOD_CALL=13,METHOD_RESPONSE,FAULT,
     MEMBER_NAME = 100,METHOD_NAME,METHOD_NAME_LEN,MAGIC,MAIN };

enum{CHAR8=1,SHORT16,LONG24,LONG32};
enum{DATA,LENGTH};

union Number_t
{
    Number_t(long number):number(number)
    {
        FRPC_LITTLE_ENDIAN(data);
    }
    Number_t(const char *number, char size)
    {
        memset(data, 0, 4);
        memcpy(data, number, size);

        FRPC_LITTLE_ENDIAN(data);
    }
    char data[4];
    long number;
};




struct StreamHolder_t
{
    StreamHolder_t()
            : os()
    {}

    ~StreamHolder_t()
    {
        os.freeze(false);
    }

    std::ostrstream os;
};



/**
@brief Structure to store data type and member count 
 
requiered in marshall and unmarshall
*/
struct TypeStorage_t
{
    /**
        @brief Simple constructor 
        @param type is data type  STRUCT or ARRAY
        @param numOfItems says how many items(ARRAY) or members(STRUCT) has
    */
    TypeStorage_t(char type, long numOfItems):type(type),numOfItems(numOfItems),member(false)
    {}
    /**
        @brief Data Type (STRUCT or ARRAY)
    */
    char type;
    /**
        @brief  says how many items(ARRAY) or members(STRUCT) has
    */
    long numOfItems;
    /**
        @brief says if member name has built or set only in STRUCT
    */
    bool member;
};


struct DateTimeInternal_t
{
    DateTimeInternal_t()
    :timeZone(0),unixTime(0),weekDay(0),sec(0),
     minute(0),hour(0),day(0),month(0),year(0)
    {}
    unsigned char timeZone;
    unsigned long unixTime;
    unsigned char weekDay;
    unsigned char sec;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned short year;
};


struct DateTimeData_t
{
    DateTimeInternal_t dateTime;
    char data[10];
    DateTimeData_t()
    {
        memset(data,0,sizeof(data));
    }

    void pack()
    {
        
        data[0] = dateTime.timeZone;
        memcpy(&data[1],reinterpret_cast<char*>(&dateTime.unixTime),4) ;
        data[5] = (dateTime.sec & 0x1f) << 3 | (dateTime.weekDay & 0x07);
        data[6] = ((dateTime.minute & 0x3f) << 1) | ((dateTime.sec & 0x20) >> 5) |
                ((dateTime.hour & 0x01) << 7);
        data[7] = ((dateTime.hour & 0x1e) >> 1) | ((dateTime.day & 0x0f) << 4);
        data[8] = ((dateTime.day & 0x1f) >> 4) | ((dateTime.month & 0x0f) << 1) |
                  ((dateTime.year & 0x07) << 5);
        data[9] = ((dateTime.year & 0x07f8) >> 3);
    }
    void unpack()
    {
        dateTime.year  = (data[9] << 3) | ((data[8] & 0xe0) >> 5);
        dateTime.month = (data[8] & 0x1e) >> 1;
        dateTime.day = ((data[8] & 0x01) << 4) |(((data[7] & 0xf0) >> 4)); 
        dateTime.hour = ((data[7] & 0x0f) << 1) | ((data[6] & 0x80) >> 7);
        dateTime.minute = ((data[6] & 0x7e) >> 1);
        dateTime.sec = ((data[6] & 0x01) << 5) | ((data[5] & 0xf8) >> 3);
        dateTime.weekDay = (data[5] & 0x07);
        memcpy(reinterpret_cast<char*>(&dateTime.unixTime),&data[1], 4 );
        dateTime.timeZone = data[0];
    }
};

const long BUFFER_SIZE = 1 << 16;
const size_t MAX_LEN = 20;

}

#endif
