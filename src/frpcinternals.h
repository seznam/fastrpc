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
 * FILE          $Id: frpcinternals.h,v 1.7 2007-05-22 13:03:23 mirecta Exp $
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
#include <sstream>
#include <frpcint.h>

#define FRPC_MAJOR_VERSION 2
#define FRPC_MINOR_VERSION 0
#define FRPC_LITTLE_ENDIAN(data)
namespace FRPC
{

enum{NONE=0, INT=1,BOOL,DOUBLE,STRING,DATETIME,BINARY,INTP8,INTN8,
     STRUCT=10,ARRAY,METHOD_CALL=13,METHOD_RESPONSE,FAULT,
     MEMBER_NAME = 100,METHOD_NAME,METHOD_NAME_LEN,MAGIC,MAIN };

enum{CHAR8=0,SHORT16=1,LONG24,LONG32,LONG40,LONG48,LONG56,LONG64};
enum{DATA,LENGTH};

const Int_t::value_type ZERO = 0;
const Int_t::value_type ALLONES = ~ZERO;
const Int_t::value_type INT8_MASK = ALLONES << 8;
const Int_t::value_type INT16_MASK = ALLONES << 16;
const Int_t::value_type INT24_MASK = ALLONES << 24;
const Int_t::value_type INT31_MASK = ALLONES << 31;
const Int_t::value_type INT32_MASK = ALLONES << 32;
const Int_t::value_type INT40_MASK = ALLONES << 40;
const Int_t::value_type INT48_MASK = ALLONES << 48;
const Int_t::value_type INT56_MASK = ALLONES << 56;

const int32_t OLD_ZERO = 0;
const int32_t OLD_ALLONES = ~ZERO;
const int32_t OLD_INT8_MASK = OLD_ALLONES << 8;
const int32_t OLD_INT16_MASK = OLD_ALLONES << 16;
const int32_t OLD_INT24_MASK = OLD_ALLONES << 24;
const int32_t OLD_INT32_MASK = OLD_ZERO;

union Number_t
{
    Number_t(Int_t::value_type number):number(number)
    {
        FRPC_LITTLE_ENDIAN(data);
    }
    Number_t(const char *number, char size)
    {
        memset(data, 0, 8);
        memcpy(data, number, size);

        FRPC_LITTLE_ENDIAN(data);
    }
    char data[8];
    Int_t::value_type number;
};

union Number32_t
{
    Number32_t(long number):number(number)
    {
        FRPC_LITTLE_ENDIAN(data);
    }
    Number32_t(const char *number, char size)
    {
        memset(data, 0, 4);
        memcpy(data, number, size);

        FRPC_LITTLE_ENDIAN(data);
    }
    char data[4];
    int32_t number;
};



struct StreamHolder_t
{
    StreamHolder_t()
            : os()
    {}

    ~StreamHolder_t()
    {
        
    }

    std::ostringstream os;
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
    TypeStorage_t(char type, 
                  unsigned int numOfItems)
             :type(type),numOfItems(numOfItems),member(false)
    {}
    /**
        @brief Data Type (STRUCT or ARRAY)
    */
    char type;
    /**
        @brief  says how many items(ARRAY) or members(STRUCT) has
    */
    unsigned int numOfItems;
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
    time_t unixTime;
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

const unsigned int  BUFFER_SIZE = 1 << 16;
const size_t MAX_LEN = 20;

}

#endif
