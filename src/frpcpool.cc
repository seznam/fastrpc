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
 * FILE          $Id: frpcpool.cc,v 1.8 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include <cstdio>

#include "frpc.h"
#include "frpcpool.h"

namespace FRPC {

Pool_t::Pool_t() {
    pointerStorage.reserve(1024);
}


Pool_t::~Pool_t() {
    free();
}

void Pool_t::free() {
    for (auto *ptr: pointerStorage)
       delete ptr;
    pointerStorage.clear();
}

Int_t&  Pool_t::Int(const Int_t::value_type &value) {
    auto* newValue =  new Int_t(value);

    pointerStorage.push_back(newValue);

    return *newValue;
}


Bool_t& Pool_t::Bool(const bool &value) {
    auto* newValue =  new Bool_t(value);

    pointerStorage.push_back(newValue);

    return *newValue;

}

Double_t& Pool_t::Double(const double &value) {
    auto* newValue =  new Double_t(value);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Binary_t& Pool_t::Binary(std::string::value_type *data,
                         std::string::size_type dataSize)
{
    auto *newValue = new Binary_t(data, dataSize);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Binary_t& Pool_t::Binary(const std::string::value_type *data,
                         std::string::size_type dataSize)
{
    auto *newValue = new Binary_t(data, dataSize);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Binary_t& Pool_t::Binary(const uint8_t *data, std::size_t dataSize) {
    auto *newValue = new Binary_t(data, dataSize);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Binary_t& Pool_t::Binary(const std::string &value)
{
    auto *newValue =  new Binary_t(value);

    pointerStorage.push_back(newValue);

    return *newValue;
}

BinaryRef_t& Pool_t::BinaryRef(BinaryRefFeeder_t feeder) {
    auto *newValue = new BinaryRef_t(feeder);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::DateTime(short year, char month, char day,
                              char hour, char minute, char sec, char weekDay,
                              time_t unixTime, int timeZone)
{
    auto *newValue =  new DateTime_t(year, month, day, hour, minute, sec,
                                           weekDay, unixTime, timeZone);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::DateTime(time_t timestamp, int timeZone) {
    auto *newValue =  new DateTime_t(timestamp, timeZone);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::DateTime(const std::string &isoFormat)
{
    auto *newValue = new DateTime_t(isoFormat);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::LocalTime(short year, char month, char day,
                               char hour, char minute, char sec)
{
    auto *newValue =  new DateTime_t(year, month, day, hour, minute, sec);

    pointerStorage.push_back(newValue);

    return *newValue;

}

DateTime_t&  Pool_t::LocalTime(const time_t &timestamp) {
    auto *newValue = new DateTime_t(timestamp);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::LocalTime() {
    auto *newValue = new DateTime_t(time(nullptr));

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::UTCTime(short year, char month, char day,
                             char hour, char minute, char sec)
{
    auto *newValue =  new DateTime_t(year, month, day, hour, minute, sec,
                                           -1, -1, 0);

    pointerStorage.push_back(newValue);

    return *newValue;

}

DateTime_t&  Pool_t::UTCTime(const time_t &timestamp) {
    auto *newValue = new DateTime_t(timestamp, 0);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::ForceUTCTime(short year, char month, char day,
                                  char hour, char min, char sec,
                                  time_t unixTime)
{
    auto *newValue = new DateTime_t(year, month, day, hour, min, sec, unixTime);

    pointerStorage.push_back(newValue);

    return *newValue;
}


DateTime_t&  Pool_t::UTCTime() {
    auto *newValue = new DateTime_t(time(nullptr), 0);

    pointerStorage.push_back(newValue);

    return *newValue;
}


String_t&  Pool_t::String(const std::string &value) {
    auto *newValue =  new String_t(value);

    pointerStorage.push_back(newValue);

    return *newValue;
}

String_t&  Pool_t::String(const std::wstring &value) {
    auto *newValue =  new String_t(value);

    pointerStorage.push_back(newValue);

    return *newValue;
}


String_t& Pool_t::String(std::string::value_type *data,
                         std::string::size_type dataSize)
{
    auto *newValue = new String_t(data, dataSize);

    pointerStorage.push_back(newValue);

    return *newValue;
}

String_t& Pool_t::String(const std::string::value_type *data,
                         std::string::size_type dataSize)
{
    auto *newValue = new String_t(data, dataSize);

    pointerStorage.push_back(newValue);

    return *newValue;
}

StringView_t &Pool_t::StringView(const char *ptr, std::size_t length) {
    auto *newValue = new StringView_t(ptr, length);

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array()
{
    auto *newValue =  new Array_t();

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(const Value_t & item1)
{
    auto *newValue =  new Array_t(item1);

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(const Value_t& item1, const Value_t& item2)
{
    auto *newValue =  new Array_t(item1);

    newValue->push_back(item2);


    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(const Value_t& item1, const Value_t& item2,
                       const Value_t& item3)
{
    auto *newValue =  new Array_t(item1);

    newValue->push_back(item2);
    newValue->push_back(item3);

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(const Value_t& item1, const Value_t& item2,
                       const Value_t& item3, const Value_t& item4)
{
    auto *newValue =  new Array_t(item1);

    newValue->push_back(item2);
    newValue->push_back(item3);
    newValue->push_back(item4);

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(const Value_t& item1, const Value_t& item2,
                       const Value_t& item3, const Value_t& item4,
                       const Value_t& item5)
{
    auto *newValue =  new Array_t(item1);

    newValue->push_back(item2);
    newValue->push_back(item3);
    newValue->push_back(item4);
    newValue->push_back(item5);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct() {
    auto *newValue = new Struct_t();

    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, const Value_t &item1) {
    auto *newValue = new Struct_t();

    newValue->insert(key1,item1);
    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, const Value_t &item1,
                         const std::string &key2, const Value_t &item2)
{
    auto *newValue = new Struct_t();

    newValue->insert(key1,item1);
    newValue->insert(key2,item2);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, const Value_t &item1,
                         const std::string &key2, const Value_t &item2,
                         const std::string &key3, const Value_t &item3)
{
    auto *newValue = new Struct_t();

    newValue->insert(key1,item1);
    newValue->insert(key2,item2);
    newValue->insert(key3,item3);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, const Value_t &item1,
                         const std::string &key2, const Value_t &item2,
                         const std::string &key3, const Value_t &item3,
                         const std::string &key4, const Value_t &item4)
{
    auto *newValue = new Struct_t();

    newValue->insert(key1,item1);
    newValue->insert(key2,item2);
    newValue->insert(key3,item3);
    newValue->insert(key4,item4);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, const Value_t &item1,
                         const std::string &key2, const Value_t &item2,
                         const std::string &key3, const Value_t &item3,
                         const std::string &key4, const Value_t &item4,
                         const std::string &key5, const Value_t &item5)
{
    auto *newValue = new Struct_t();

    newValue->insert(key1,item1);
    newValue->insert(key2,item2);
    newValue->insert(key3,item3);
    newValue->insert(key4,item4);
    newValue->insert(key5,item5);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Null_t& Pool_t::Null() {
    return Null_t::staticValue;
}

} // namespace FRPC
