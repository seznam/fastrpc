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
 * FILE          $Id: frpcinternals.h,v 1.13 2010-05-10 08:39:33 mirecta Exp $
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
#include <frpcstreamerror.h>
#include <frpclogging.h>

#include <sys/param.h>

#ifdef __BYTE_ORDER
   #if __BYTE_ORDER == __BIG_ENDIAN
      #define FRPC_BIG_ENDIAN
      //#warning "BIG ENDIAN detected! Using it."
   #endif
#elif defined __BYTE_ORDER__
   #if __BYTE_ORDER__ == __BIG_ENDIAN__
      #define FRPC_BIG_ENDIAN
      //#warning "BIG ENDIAN detected! Using it."
   #endif
#else
  #warning "ENDIAN not defined ! Using default LITTLE ENDIAN"
#endif /* __BYTE_ORDER */


#define FRPC_MAJOR_VERSION 3
#define FRPC_MINOR_VERSION 1

// We use this to intentionally downgrade the emitted frpc stream
// so that we can have a comfortable 2 phase transition to new protocol version.
#define FRPC_MAJOR_VERSION_DEFAULT 2
#define FRPC_MINOR_VERSION_DEFAULT 1

#define SWAP_BYTE(byte1,byte2)  \
         byte1 = byte1 ^ byte2; \
         byte2 = byte1 ^ byte2; \
         byte1 = byte1 ^ byte2;

namespace FRPC {

enum {
    NONE            = 0,
    INT             = 1,
    BOOL            = 2,
    DOUBLE          = 3,
    STRING          = 4,
    DATETIME        = 5,
    BINARY          = 6,
    INTP8           = 7,
    INTN8           = 8,
    STRUCT          = 10,
    ARRAY           = 11,
    NULLTYPE        = 12,
    METHOD_CALL     = 13,
    METHOD_RESPONSE = 14,
    FAULT           = 15,
    METHOD_NAME     = 100,
    MEMBER_NAME     = 101
};

enum {
    CHAR8   = 0,
    SHORT16 = 1,
    LONG24  = 2,
    LONG32  = 3,
    LONG40  = 4,
    LONG48  = 5,
    LONG56  = 6,
    LONG64  = 7
};

const Int_t::value_type ZERO = 0;
const Int_t::value_type ALLONES = ~ZERO; // NOLINT
const Int_t::value_type INT8_MASK =  (int64_t)((uint64_t)ALLONES <<  8u);
const Int_t::value_type INT16_MASK = (int64_t)((uint64_t)ALLONES << 16u);
const Int_t::value_type INT24_MASK = (int64_t)((uint64_t)ALLONES << 24u);
const Int_t::value_type INT31_MASK = (int64_t)((uint64_t)ALLONES << 31u);
const Int_t::value_type INT32_MASK = (int64_t)((uint64_t)ALLONES << 32u);
const Int_t::value_type INT40_MASK = (int64_t)((uint64_t)ALLONES << 40u);
const Int_t::value_type INT48_MASK = (int64_t)((uint64_t)ALLONES << 48u);
const Int_t::value_type INT56_MASK = (int64_t)((uint64_t)ALLONES << 56u);

const int32_t OLD_ZERO = 0;
const int32_t OLD_ALLONES = ~ZERO; // NOLINT
const int32_t OLD_INT8_MASK =  (int32_t)((uint32_t)OLD_ALLONES <<  8u);
const int32_t OLD_INT16_MASK = (int32_t)((uint32_t)OLD_ALLONES << 16u);
const int32_t OLD_INT24_MASK = (int32_t)((uint32_t)OLD_ALLONES << 24u);
const int32_t OLD_INT32_MASK = OLD_ZERO;

union Number_t {
    Number_t(Int_t::value_type number)
        :number(number)
    {
#ifdef FRPC_BIG_ENDIAN
        //swap it
        SWAP_BYTE(data[7],data[0]);
        SWAP_BYTE(data[6],data[1]);
        SWAP_BYTE(data[5],data[2]);
        SWAP_BYTE(data[4],data[3]);
#endif
    }
    char data[8]; // NOLINT
    Int_t::value_type number;
};

union Number32_t {
    Number32_t(long number)
        :number(static_cast<int32_t>(number))
    {
#ifdef FRPC_BIG_ENDIAN
        //swap it
        SWAP_BYTE(data[3],data[0]);
        SWAP_BYTE(data[2],data[1]);
#endif
    }

    char data[4]; // NOLINT
    int32_t number;
};

struct StreamHolder_t {
    StreamHolder_t() = default;
    ~StreamHolder_t() = default;
    std::ostringstream os;
};

/**
@brief Structure to store data type and member count

requiered in marshall and unmarshall
*/
struct TypeStorage_t {
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

struct DateTimeInternal_t {
    DateTimeInternal_t()
    :timeZone(0), unixTime(0), weekDay(0), sec(0),
     minute(0), hour(0), day(0), month(0), year(0)
    {}
    // NOLINTBEGIN
    char timeZone;
    time_t unixTime;
    unsigned char weekDay;
    unsigned char sec;
    unsigned char minute;
    unsigned char hour;
    unsigned char day;
    unsigned char month;
    unsigned short year;
    // NOLINTEND
};

struct DateTimeData_t {
    DateTimeData_t() {
        memset(data,0,sizeof(data));
    }

    void pack() {
        Int_t::value_type  unixTimeAbs = (dateTime.unixTime > 0)?
                                         dateTime.unixTime:
                                         -dateTime.unixTime;
        int32_t unixTime;
        data[0] = dateTime.timeZone;

        if (unixTimeAbs & INT32_MASK)
            unixTime = -1;
        else
            unixTime = static_cast<decltype(unixTime)>(dateTime.unixTime);

        memcpy(&data[1],reinterpret_cast<char*>(&unixTime),4) ;
        data[5] = static_cast<char>(
            ((dateTime.sec & 0x1f) << 3)
            | (dateTime.weekDay & 0x07)
        );
        data[6] = static_cast<char>(
            ((dateTime.minute & 0x3f) << 1)
            | ((dateTime.sec & 0x20) >> 5)
            | ((dateTime.hour & 0x01) << 7)
        );
        data[7] = static_cast<char>(
            ((dateTime.hour & 0x1e) >> 1)
            | ((dateTime.day & 0x0f) << 4)
        );
        data[8] = static_cast<char>(
            ((dateTime.day & 0x1f) >> 4)
            | ((dateTime.month & 0x0f) << 1)
            | ((dateTime.year & 0x07) << 5)
        );
        data[9] = static_cast<char>(((dateTime.year & 0x07f8) >> 3));
    }

    DateTimeInternal_t dateTime;
    char data[10];
};

// the datetime data structure used since protocol version 3
struct DateTimeDataV3_t {
    DateTimeDataV3_t() {
        memset(data,0,sizeof(data));
    }

    void pack() {
        data[0] = dateTime.timeZone;

        int64_t time64 = dateTime.unixTime;

        if (sizeof(time_t) > sizeof(time64)) {
            if (dateTime.unixTime != time64) {
                throw StreamError_t(
                        "Protocol can't transfer the timestamp value");
            }
        }

        memcpy(&data[1],reinterpret_cast<char*>(&time64), 8);
        data[ 9] = static_cast<char>(
            ((dateTime.sec & 0x1f) << 3)
            | (dateTime.weekDay & 0x07)
        );
        data[10] = static_cast<char>(
            ((dateTime.minute & 0x3f) << 1)
            | ((dateTime.sec & 0x20) >> 5)
            | ((dateTime.hour & 0x01) << 7)
        );
        data[11] = static_cast<char>(
            ((dateTime.hour & 0x1e) >> 1)
            | ((dateTime.day & 0x0f) << 4)
        );
        data[12] = static_cast<char>(
            ((dateTime.day & 0x1f) >> 4)
            | ((dateTime.month & 0x0f) << 1)
            | ((dateTime.year & 0x07) << 5)
        );
        data[13] = static_cast<char>(((dateTime.year & 0x07f8) >> 3));
    }

    DateTimeInternal_t dateTime;
    char data[14];
};

const unsigned int HTTP_BALLAST = 1 << 10;
const unsigned int  BUFFER_SIZE = (1 << 16) - HTTP_BALLAST;
const size_t MAX_LEN = 20;

/**
 * Helper function that calls logger callback function.
*/
void FRPC_DLLEXPORT callLoggerCallback(LogEvent_t event, LogEventData_t &eventData);

} // namespace FRPC

#endif
