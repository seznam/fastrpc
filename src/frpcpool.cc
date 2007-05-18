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
 * FILE          $Id: frpcpool.cc,v 1.4 2007-05-18 15:29:45 mirecta Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#include <frpc.h>
//remove
#include <stdio.h>

namespace FRPC
{

Pool_t::Pool_t()
{
    pointerStorage.reserve(1024);
}


Pool_t::~Pool_t()
{

    //deleting all pointers
    
    for(std::vector< Value_t* >::iterator ipointerStorage = pointerStorage.begin();
            ipointerStorage != pointerStorage.end(); ++ipointerStorage)
    {
       // printf(" mazem pointer %p\n",*ipointerStorage);
       delete *ipointerStorage ;
    }
    
    pointerStorage.clear();
    //pointerStorage.reserve(0);
    
}

void Pool_t::free()
{
    //deleting all pointers
    for(std::vector< Value_t* >::iterator ipointerStorage = pointerStorage.begin();
            ipointerStorage != pointerStorage.end(); ++ipointerStorage)
    {
        delete  *ipointerStorage ;
    }
    pointerStorage.clear();

}

Int_t&  Pool_t::Int(Int_t::value_type value)
{
    Int_t* newValue =  new Int_t(*this, value);

    //printf("Alokujem %p\n",newValue);
    pointerStorage.push_back(newValue);

    return *newValue;
}

Bool_t& Pool_t::Bool(bool value)
{
    Bool_t* newValue =  new Bool_t(*this, value);

    pointerStorage.push_back(newValue);

    return *newValue;

}



Double_t& Pool_t::Double(double value)
{
    Double_t* newValue =  new Double_t(*this, value);

    pointerStorage.push_back(newValue);

    return *newValue;
}


Binary_t& Pool_t::Binary(std::string::value_type *data, std::string::size_type dataSize)
{
    Binary_t *newValue = new Binary_t(*this, data, dataSize);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Binary_t& Pool_t::Binary(const std::string &value)
{
    Binary_t *newValue =  new Binary_t(*this, value);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::DateTime(short year, char month, char day,
                              char hour, char minute, char sec, char weekDay, time_t unixTime,
                              char timeZone)
{
    DateTime_t *newValue =  new DateTime_t(*this, year, month, day, hour, minute, sec, weekDay, unixTime,
                                           timeZone);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::DateTime(short year, char month, char day,
                              char hour, char minute, char sec)
{
    DateTime_t *newValue =  new DateTime_t(*this, year, month, day, hour, minute, sec,-1,-1,0);

    pointerStorage.push_back(newValue);

    return *newValue;

}

DateTime_t&  Pool_t::DateTime(time_t timestamp)
{
    DateTime_t *newValue =  new DateTime_t(*this, timestamp);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::DateTime(const std::string &isoFormat)
{
    DateTime_t *newValue =  new DateTime_t(*this, isoFormat);

    pointerStorage.push_back(newValue);

    return *newValue;
}

DateTime_t&  Pool_t::DateTime()
{
    DateTime_t *newValue =  new DateTime_t(*this);

    pointerStorage.push_back(newValue);

    return *newValue;
}



String_t&  Pool_t::String(const std::string &value)
{
    String_t *newValue =  new String_t(*this, value);

    pointerStorage.push_back(newValue);

    return *newValue;
}

String_t&  Pool_t::String(const std::wstring &value)
{
    String_t *newValue =  new String_t(*this, value);

    pointerStorage.push_back(newValue);

    return *newValue;
}


String_t& Pool_t::String(std::string::value_type *data, std::string::size_type dataSize)
{
    String_t *newValue = new String_t(*this, data, dataSize);

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array()
{
    Array_t *newValue =  new Array_t(*this);

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(Value_t & item1)
{
    Array_t *newValue =  new Array_t(*this, item1);

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(Value_t& item1, Value_t& item2)
{
    Array_t *newValue =  new Array_t(*this, item1);

    newValue->push_back(item2);


    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(Value_t& item1, Value_t& item2, Value_t& item3)
{
    Array_t *newValue =  new Array_t(*this,item1);

    newValue->push_back(item2);
    newValue->push_back(item3);

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(Value_t& item1, Value_t& item2, Value_t& item3,Value_t& item4)
{
    Array_t *newValue =  new Array_t(*this,item1);

    newValue->push_back(item2);
    newValue->push_back(item3);
    newValue->push_back(item4);

    pointerStorage.push_back(newValue);

    return *newValue;
}



Array_t& Pool_t::Array(Value_t& item1, Value_t& item2, Value_t& item3,Value_t& item4, Value_t& item5)
{
    Array_t *newValue =  new Array_t(*this,item1);

    newValue->push_back(item2);
    newValue->push_back(item3);
    newValue->push_back(item4);
    newValue->push_back(item5);

    pointerStorage.push_back(newValue);

    return *newValue;
}
Struct_t& Pool_t::Struct()
{
    Struct_t *newValue = new Struct_t(*this);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, Value_t &item1)
{
    Struct_t *newValue = new Struct_t(*this);

    newValue->insert(key1,item1);
    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, Value_t &item1, const std::string &key2,
                         Value_t &item2)
{
    Struct_t *newValue = new Struct_t(*this);

    newValue->insert(key1,item1);
    newValue->insert(key2,item2);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, Value_t &item1, const std::string &key2,
                         Value_t &item2, const std::string &key3, Value_t &item3)
{
    Struct_t *newValue = new Struct_t(*this);

    newValue->insert(key1,item1);
    newValue->insert(key2,item2);
    newValue->insert(key3,item3);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, Value_t &item1, const std::string &key2,
                         Value_t &item2,
                         const std::string &key3, Value_t &item3,
                         const std::string &key4, Value_t &item4)
{
    Struct_t *newValue = new Struct_t(*this);

    newValue->insert(key1,item1);
    newValue->insert(key2,item2);
    newValue->insert(key3,item3);
    newValue->insert(key4,item4);

    pointerStorage.push_back(newValue);

    return *newValue;
}

Struct_t& Pool_t::Struct(const std::string &key1, Value_t &item1, const std::string &key2,
                         Value_t &item2,
                         const std::string &key3, Value_t &item3,
                         const std::string &key4, Value_t &item4,
                         const std::string &key5, Value_t &item5)
{
    Struct_t *newValue = new Struct_t(*this);

    newValue->insert(key1,item1);
    newValue->insert(key2,item2);
    newValue->insert(key3,item3);
    newValue->insert(key4,item4);
    newValue->insert(key5,item5);

    pointerStorage.push_back(newValue);

    return *newValue;
}

}
