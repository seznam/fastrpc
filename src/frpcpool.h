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
 * FILE          $Id: frpcpool.h,v 1.8 2011-02-11 08:56:17 burlog Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCPOOL_H
#define FRPCFRPCPOOL_H

#include <frpcplatform.h>

#include <frpcvalue.h>
#include <vector>
#include <string>
#include <frpcint.h>

namespace FRPC
{


class Bool_t;
class Double_t;
class Binary_t;
class BinaryRef_t;
struct BinaryRefFeeder_t;
class String_t;
class Array_t;
class DateTime_t;
class Struct_t;
class Null_t;

/**
@author Miroslav Talasek
@brief Memory pool
 
Thic obbject has completely control of pointers to all Value_t
*/
class FRPC_DLLEXPORT Pool_t
{
public:
    /**
        @brief Constructor of memory pool
    */
    Pool_t();

    /** Allow move. */
    Pool_t(Pool_t &&) = default;
    Pool_t &operator=(Pool_t &&) = default;

    /**
        @brief Destructor of memory pool
    */
    ~Pool_t();

    void  free();
    /**
        @brief Create new Int_t object from long number
        @param value is a long number
        @return reference to Int_t
    */
    Int_t&  Int(const Int_t::value_type &value);
    /**
        @brief Create new Bool_t object from bool value
        @param value is a bool value
        @return reference to Bool_t
    */
    Bool_t& Bool(const bool &value);
    /**
        @brief Create new Double_t object from double number
        @param value is a double number
        @return reference to Double_t
    */
    Double_t& Double(const double &value);
    /**
        @brief Create new Binary_t object from pointer and size of data
        @param data is a  pointer to binary data
        @param dataSize is a size of binary data
        @return reference to Binary_t
    */
    Binary_t& Binary(std::string::value_type *data,
                     std::string::size_type dataSize);
    /**
        @brief Create new Binary_t object from pointer and size of data
        @param data is a  pointer to binary data
        @param dataSize is a size of binary data
        @return reference to Binary_t
    */
    Binary_t& Binary(const std::string::value_type *data,
                     std::string::size_type dataSize);
    /**
        @brief Create new Binary_t object from pointer and size of data
        @param data is a  pointer to binary data
        @param dataSize is a size of binary data
        @return reference to Binary_t
    */
    Binary_t& Binary(const uint8_t *data, std::size_t dataSize);
    /**
        @brief Create new Binary_t object from pointer and size of data
        @param value is a std::string
        @return reference to Binary_t
    */
    Binary_t& Binary(const std::string &value);
    /**
        @brief Create new BinaryRef_t object from chunks feeder.
    */
    BinaryRef_t& BinaryRef(BinaryRefFeeder_t feeder);
    /*
        @brief Create new DateTime_t object from unix tm structure
        You should specify your timezone!

        @param year - Year is offset (0 - 2047) to zero-year 1600 
        (0=1600, ... 370 = 1970, ...)
        @param month - Month is 1 - 12
        @param day -   Day is 1 - 31
        @param hour -  Hour is 0 - 23
        @param min -   Minute is 0 - 59
        @param sec -   Second is 0 - 59
        @param weekDay - dey in week.
        @param unixTime - second from epoch.
        @param timeZone in in quarter-hours
        @return reference to DateTime_t
    */
    DateTime_t&  DateTime(short year, char month, char day,
                          char hour, char min, char sec, char weekDay,
                          time_t unixTime, int timeZone);
    /**
        @brief Create new DateTime_t object from unix timestamp
        You should specify your timezone!

        @param timestamp unix timestamp
        @param timeZone difference between UTC and localtime in seconds
        @return reference to DateTime_t
    */
    DateTime_t&  DateTime(time_t timestamp, int timeZone);
    /**
        @brief Create new DateTime_t object from iso string
        You should specify your timezone!

        @param isoFormat ISO datetime string
        @return reference to DateTime_t
    */
    DateTime_t&  DateTime(const std::string &isoFormat);
    /**
        @brief Create new DateTime_t object from data and local timezone
        @param year - Year is offset (0 - 2047) to zero-year 1600 
        (0=1600, ... 370 = 1970, ...)
        @param month - Month is 1 - 12
        @param day -   Day is 1 - 31
        @param hour -  Hour is 0 - 23
        @param min -   Minute is 0 - 59
        @param sec -   Second is 0 - 59
        @return reference to DateTime_t
    */
    DateTime_t&  LocalTime(short year, char month, char day,
                           char hour = 0, char min = 0, char sec = 0);
    /**
        @brief Create new DateTime_t object from timestamp and local timezone
        @param timestamp unix timestamp
        @return reference to DateTime_t
    */
    DateTime_t&  LocalTime(const time_t &timestamp);
    /**
        @brief Create new DateTime_t object from now and local timezone
        @return reference to DateTime_t
    */
    DateTime_t&  LocalTime();
    /**
        @brief Create new DateTime_t object from data and UTC timezone
        @param year - Year is offset (0 - 2047) to zero-year 1600 
        (0=1600, ... 370 = 1970, ...)
        @param month - Month is 1 - 12
        @param day -   Day is 1 - 31
        @param hour -  Hour is 0 - 23
        @param min -   Minute is 0 - 59
        @param sec -   Second is 0 - 59
        @return reference to DateTime_t
    */
    DateTime_t&  UTCTime(short year, char month, char day,
                         char hour = 0, char min = 0, char sec = 0);
    /**
        @brief Create new DateTime_t object from timestamp and UTC timezone
        @param timestamp unix timestamp
        @return reference to DateTime_t
    */
    DateTime_t&  UTCTime(const time_t &timestamp);
    /**
        @brief Create new DateTime_t object from now and UTC timezone
        @return reference to DateTime_t
    */
    DateTime_t&  UTCTime();

    /**
        @brief Create new DateTime_t object without clearly from given data
               no additional check will be performed
        @param year  - 0 ... 2012 ... (I think doc above lies about year range)
        @param month - Month is 1 - 12
        @param day -   Day is 1 - 31
        @param hour -  Hour is 0 - 23
        @param min -   Minute is 0 - 59
        @param sec -   Second is 0 - 59
        @return reference to DateTime_t
    */
    DateTime_t&  ForceUTCTime(short year, char month, char day,
                              char hour, char min, char sec,
                              time_t unixTime);

    /**
        @brief Create new String_t object from std::string
        @param value is a std::string 
        @return reference to String_t
    */
    String_t&  String(const std::string &value);
    /**
        @brief Create new String_t object from std::wstring
        @param value is a std::wstring 
        @return reference to String_t
    */
    String_t&  String(const std::wstring &value);
    /**
        @brief Create new String_t object from pointer and size of data
        @param data is a  pointer to string data
        @param dataSize is a size of string data
        @return reference to String_t
    */
    String_t& String(std::string::value_type *data,
                     std::string::size_type dataSize);

    /**
        @brief Create new String_t object from pointer and size of data
        @param data is a  pointer to string data
        @param dataSize is a size of string data
        @return reference to String_t
    */
    String_t& String(const std::string::value_type *data,
                     std::string::size_type dataSize);

    /**
        @brief Create new empty Array_t 
        @return reference to Array_t
    */
    Array_t& Array();
    /**
        @brief Create new  Array_t with one item 
        @param item1 is a Value_t reference
        @return reference to Array_t
    */
    Array_t& Array(const Value_t &item1);
    /**
        @brief Create new  Array_t with two items 
        @param item1 is a Value_t reference
        @param item2 is a Value_t reference
        @return reference to Array_t
    */
    Array_t& Array(const Value_t &item1, const Value_t &item2);
    /**
        @brief Create new  Array_t with three items 
        @param item1 is a Value_t reference
        @param item2 is a Value_t reference
        @param item3 is a Value_t reference
        @return reference to Array_t
    */
    Array_t& Array(const Value_t &item1, const Value_t &item2,
                   const Value_t &item3);
    /**
        @brief Create new  Array_t with four items 
        @param item1 is a Value_t reference
        @param item2 is a Value_t reference
        @param item3 is a Value_t reference
        @param item4 is a Value_t reference
        @return reference to Array_t
    */
    Array_t& Array(const Value_t &item1, const Value_t &item2,
                   const Value_t &item3, const Value_t &item4);
    /**
        @brief Create new  Array_t with five items 
        @param item1 is a Value_t reference
        @param item2 is a Value_t reference
        @param item3 is a Value_t reference
        @param item4 is a Value_t reference
        @param item5 is a Value_t reference
        @return reference to Array_t
    */
    Array_t& Array(const Value_t &item1, const Value_t &item2,
                   const Value_t &item3, const Value_t &item4,
                   const Value_t &item5);
    /**
    @brief Create new empty Struct_t
    @return reference to Struct_t
    */

    Struct_t& Struct();
    /**
        @brief Create new  Struct_t with one item
        @param key1  is a const std::string reference
        @param item1 is a Value_t reference
        @return reference to Struct_t
    */
    Struct_t& Struct(const std::string &key1, const Value_t &item1);
    /**
        @brief Create new  Struct_t with two items
        @param key1  is a const std::string reference
        @param item1 is a Value_t reference
        @param key2  is a const std::string reference
        @param item2 is a Value_t reference
        @return reference to Struct_t
    */
    Struct_t& Struct(const std::string &key1, const Value_t &item1,
                     const std::string &key2, const Value_t &item2);
    /**
        @brief Create new  Struct_t with three items
        @param key1  is a const std::string reference
        @param item1 is a Value_t reference
        @param key2  is a const std::string reference
        @param item2 is a Value_t reference
        @param key3  is a const std::string reference
        @param item3 is a Value_t reference
        @return reference to Struct_t
    */
    Struct_t& Struct(const std::string &key1, const Value_t &item1,
                     const std::string &key2, const Value_t &item2,
                     const std::string &key3, const Value_t &item3);
    /**
        @brief Create new  Struct_t with four items
        @param key1  is a const std::string reference
        @param item1 is a Value_t reference
        @param key2  is a const std::string reference
        @param item2 is a Value_t reference
        @param key3  is a const std::string reference
        @param item3 is a Value_t reference
        @param key4  is a const std::string reference
        @param item4 is a Value_t reference
        @return reference to Struct_t
    */
    Struct_t& Struct(const std::string &key1, const Value_t &item1,
                     const std::string &key2, const Value_t &item2,
                     const std::string &key3, const Value_t &item3,
                     const std::string &key4, const Value_t &item4);
    /**
        @brief Create new  Struct_t with five items
        @param key1  is a const std::string reference
        @param item1 is a Value_t reference
        @param key2  is a const std::string reference
        @param item2 is a Value_t reference
        @param key3  is a const std::string reference
        @param item3 is a Value_t reference
        @param key4  is a const std::string reference
        @param item4 is a Value_t reference
        @param key5  is a const std::string reference
        @param item5 is a Value_t reference
        @return reference to Struct_t
    */
    Struct_t& Struct(const std::string &key1, const Value_t &item1,
                     const std::string &key2, const Value_t &item2,
                     const std::string &key3, const Value_t &item3,
                     const std::string &key4, const Value_t &item4,
                     const std::string &key5, const Value_t &item5);

    /**
        @brief Create new  Null_t
    */
    Null_t& Null();

    void steal_pointers(Pool_t &o) throw() {
        pointerStorage.reserve(pointerStorage.size() + o.pointerStorage.size());
        for (auto &ptr: o.pointerStorage)
            pointerStorage.push_back(ptr);
        o.pointerStorage.clear();
    }

    //private:
    std::vector< Value_t* > pointerStorage; ///@brief pointer storage of pool

private:
    // this is denied
    DateTime_t& DateTime(time_t);
    DateTime_t& DateTime(int);
    DateTime_t& DateTime(char);

    // disable devil c'tors
    Pool_t(const Pool_t &);
    const Pool_t &operator=(const Pool_t &);
};

}

#endif
