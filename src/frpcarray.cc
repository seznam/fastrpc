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
 * FILE          $Id: frpcarray.cc,v 1.12 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2005
 * All Rights Reserved
 *
 * HISTORY
 *
 */

#include "frpcarray.h"

#include "frpcindexerror.h"
#include "frpclenerror.h"
#include "frpctypeerror.h"
#include "frpc.h"
#include "frpcconfig.h"

namespace FRPC
{


Array_t::~Array_t()
{}


Value_t& Array_t::clone(Pool_t& newPool) const
{
    Array_t *newArray =&newPool.Array();
    newArray->reserve(size());

    for(std::vector<Value_t*>::const_iterator iarrayData = arrayData.begin();
            iarrayData != arrayData.end(); ++iarrayData)
    {
        newArray->push_back((*iarrayData)->clone( newPool ));
    }

    return *newArray;
}

Array_t::Array_t()
{
    arrayData.reserve(LibConfig_t::getInstance()->getDefaultArraySize());
}


Array_t::Array_t(const Value_t &item)
{
    arrayData.reserve(LibConfig_t::getInstance()->getDefaultArraySize());
    arrayData.push_back(const_cast<Value_t*>(&item));
}

Array_t::const_iterator Array_t::begin() const
{
    return arrayData.begin();
}

Array_t::const_iterator Array_t::end() const
{
    return arrayData.end();
}

Array_t::iterator Array_t::begin()
{
    return arrayData.begin();
}

Array_t::iterator Array_t::end()
{
    return  arrayData.end();
}

Array_t::size_type Array_t::size() const
{
    return arrayData.size();
}

void Array_t::clear()
{
    arrayData.clear();
}

void Array_t::reserve(Array_t::size_type size)
{
   arrayData.reserve(size);
}

Array_t::size_type Array_t::capacity()
{
   return arrayData.capacity();
}

void Array_t::push_back(const Value_t &value)
{
    arrayData.push_back(const_cast<Value_t *>(&value));
}

Array_t& Array_t::append(const Value_t &value)
{

    arrayData.push_back(const_cast<Value_t *>(&value));
    return *this;
}

bool Array_t::empty() const
{
    return arrayData.empty();
}

Value_t& Array_t::operator[] (Array_t::size_type index)
{
    if(index >= arrayData.size())
        throw(IndexError_t::format("index %zd is out of range 0 - %zd.", index,
                                   arrayData.size()));

    return *(arrayData[index]);
}

const Value_t& Array_t::operator[] (Array_t::size_type index) const
{
    if(index >= arrayData.size())
        throw(IndexError_t::format("index %zd is out of range 0 - %zd.", index,
                                   arrayData.size()));

    return *(arrayData[index]);
}

void Array_t::checkItems(const std::string &items) const
{
    size_t itemsSize(0);
    for (size_t i(0) ; i < items.size() ; ++i) {
        if (items[i] != '?') {
            ++itemsSize;
        }
    }

    if(arrayData.size() != itemsSize) {
        throw LenError_t::format("Array must have %zd parameters.",
                                 items.size());
    }

    unsigned int itemNum = 0;

    for (std::vector<Value_t *>::const_iterator i = arrayData.begin();
         i != arrayData.end(); ++i)
    {
        bool canBeNull(itemNum < items.size()-1 && items[itemNum+1] == '?');

        if (canBeNull && (*i)->getType() == Null_t::TYPE) {
            itemNum += 2;
            continue;
        }

        switch(items[itemNum])
        {
        case 'i':
            if((*i)->getType() != Int_t::TYPE)
                throw TypeError_t::format("Parameter %d must be int not %s.",
                                          itemNum + 1, (*i)->getTypeName());
            break;

        case 's':
            if((*i)->getType() != String_t::TYPE)
                throw TypeError_t::format("Parameter %d must be string not %s.",
                                          itemNum + 1, (*i)->getTypeName());
            break;

        case 'd':
            if((*i)->getType() != Double_t::TYPE)
                throw TypeError_t::format("Parameter %d must be double not %s.",
                                          itemNum + 1, (*i)->getTypeName());
            break;

        case 'b':
            if((*i)->getType() != Bool_t::TYPE)
                throw TypeError_t::format("Parameter %d must be bool not %s.",
                                          itemNum + 1, (*i)->getTypeName());
            break;

        case 'D':
            if((*i)->getType() != DateTime_t::TYPE)
                throw TypeError_t::format(
                    "Parameter %d must be dateTime not %s.", itemNum + 1,
                    (*i)->getTypeName());
            break;

        case 'B':
            if((*i)->getType() != Binary_t::TYPE)
                throw TypeError_t::format("Parameter %d must be binary not %s.",
                                          itemNum + 1, (*i)->getTypeName());
            break;

        case 'S':
            if((*i)->getType() != Struct_t::TYPE)
                throw TypeError_t::format("Parameter %d must be struct not %s.",
                                          itemNum + 1, (*i)->getTypeName());
            break;

        case 'A':
            if((*i)->getType() != Array_t::TYPE)
                throw TypeError_t::format("Parameter %d must be array not %s.",
                                          itemNum + 1, (*i)->getTypeName());
            break;

        }

        itemNum += canBeNull ? 2 : 1;
    }
}

}
