/*
 * FILE          $Id: frpcarray.cc,v 1.4 2006-03-15 15:42:37 mirecta Exp $
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


const long ARRAY_RESERVED_SPACE = 32;

namespace FRPC
{


Array_t::~Array_t()
{}


Value_t& Array_t::clone(Pool_t& newPool) const
{
    Array_t *newArray =&newPool.Array();

    for(std::vector<Value_t*>::const_iterator iarrayData = arrayData.begin();
            iarrayData != arrayData.end(); ++iarrayData)
    {
		newArray->push_back((*iarrayData)->clone( newPool ));

    }
    return *newArray;
}

Array_t::Array_t(Pool_t &pool):Value_t(pool)
{
    arrayData.reserve(ARRAY_RESERVED_SPACE);
}


Array_t::Array_t(Pool_t &pool, Value_t  &item ):Value_t(pool)
{
    arrayData.reserve(ARRAY_RESERVED_SPACE);
    arrayData.push_back(&item);
}

Array_t::iterator Array_t::begin() const
{
    return arrayData.begin();
}

Array_t::iterator Array_t::end() const
{
    return arrayData.end()
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



void Array_t::push_back(Value_t &value)
{
    arrayData.push_back(&value);
}

Array_t& Array_t::append(Value_t &value)
{

    arrayData.push_back(&value);
    return *this;
}

bool Array_t::empty() const
{
    return arrayData.empty();
}

Value_t& Array_t::operator[] (Array_t::size_type index)
{
    if(index >= arrayData.size())
        throw(IndexError_t("index %d is out of range 0 - %d.", index, arrayData.size()));

    return *(arrayData[index]);
}

const Value_t& Array_t::operator[] (Array_t::size_type index) const
{
    if(index >= arrayData.size())
        throw(IndexError_t("index %d is out of range 0 - %d.", index, arrayData.size()));

    return *(arrayData[index]);
}

void Array_t::checkItems(const std::string &items) const
{

    if(arrayData.size() !=  items.size())
    {
        throw LenError_t("Array must have %d parameters.",items.size() );
    }

    long itemNum = 0;

    for( std::vector<Value_t*>::const_iterator i = arrayData.begin(); i != arrayData.end(); ++i)
    {
        switch(items[itemNum])
        {
        case 'i':
            if((*i)->getType() != Int_t::TYPE)
                throw TypeError_t("Parameter %d must be int not %s.",itemNum + 1,
                                  (*i)->getTypeName());
            break;

        case 's':
            if((*i)->getType() != String_t::TYPE)
                throw TypeError_t("Parameter %d must be string not %s.",itemNum + 1,
                                  (*i)->getTypeName());
            break;

        case 'd':
            if((*i)->getType() != Double_t::TYPE)
                throw TypeError_t("Parameter %d must be double not %s.",itemNum + 1,
                                  (*i)->getTypeName());
            break;

        case 'b':
            if((*i)->getType() != Bool_t::TYPE)
                throw TypeError_t("Parameter %d must be bool not %s.",itemNum + 1,
                                  (*i)->getTypeName());
            break;

        case 'D':
            if((*i)->getType() != DateTime_t::TYPE)
                throw TypeError_t("Parameter %d must be dateTime not %s.",itemNum + 1,
                                  (*i)->getTypeName());
            break;

        case 'B':
            if((*i)->getType() != Binary_t::TYPE)
                throw TypeError_t("Parameter %d must be binary not %s.",itemNum + 1,
                                  (*i)->getTypeName());
            break;

        case 'S':
            if((*i)->getType() != Struct_t::TYPE)
                throw TypeError_t("Parameter %d must be struct not %s.",itemNum + 1,
                                  (*i)->getTypeName());
            break;

        case 'A':
            if((*i)->getType() != Array_t::TYPE)
                throw TypeError_t("Parameter %d must be array not %s.",itemNum + 1,
                                  (*i)->getTypeName());
            break;

        }

        itemNum ++;
    }


}

}
