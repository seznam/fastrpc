/*
 * FILE          $Id: frpcstruct.cc,v 1.2 2006-02-09 16:00:26 vasek Exp $
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
#include "frpcstruct.h"
#include "frpcpool.h"
#include "frpckeyerror.h"
#include "frpclenerror.h"

namespace FRPC
{

Struct_t::Struct_t(Pool_t &pool):Value_t(pool)
{}


Struct_t::~Struct_t()
{}

Struct_t::Struct_t(Pool_t &pool, Struct_t::pair value):Value_t(pool)
{
    structData.insert(value);
}

Struct_t::Struct_t(Pool_t &pool, const std::string &key, Value_t &value):Value_t(pool)
{
    if(key.size() > 255)
    {
        throw LenError_t("Size of member name must be max 255 no %d ",key.size() );
    }
    structData.insert(value_type(key,&value));
}

Value_t& Struct_t::clone(Pool_t& newPool) const
{
    Struct_t *newStruct = &newPool.Struct();

    for(std::map<std::string,Value_t*>::const_iterator istructData = structData.begin();
            istructData != structData.end(); ++istructData)
    {
        newStruct->insert(value_type(istructData->first,&(istructData->second)->clone(newPool)));
    }

    return  *newStruct;
}

bool Struct_t::has_key(const Struct_t::key_type &key) const
{
    if(structData.find(key) != structData.end())
        return true;
    return false;
}

std::pair<Struct_t::iterator, bool> Struct_t::insert(const Struct_t::key_type &key, Value_t &value)
{

    return  structData.insert(value_type(key,&value));
}

std::pair<Struct_t::iterator, bool> Struct_t::insert(Struct_t::pair value)
{
    return structData.insert(value);
}

Struct_t::iterator Struct_t::begin()
{
    return structData.begin();
}

Struct_t::iterator Struct_t::end()
{
    return structData.end();
}

bool Struct_t::empty() const
{
    return structData.empty();
}

Struct_t::size_type Struct_t::size() const
{
    return structData.size();
}

Struct_t& Struct_t::append(Struct_t::pair value)
{
    structData.insert(value);
    return *this;
}

Struct_t& Struct_t::append(const Struct_t::key_type &key, Value_t &value)
{
    if(key.size() > 255)
    {
        throw LenError_t("Size of member name must be max 255 no %d ",key.size() );
    }

    structData.insert(value_type(key,&value));
    return *this;
}

Value_t& Struct_t::operator[] (const Struct_t::key_type &key)
{
    iterator istructData;

    if ((istructData = structData.find(key)) == structData.end())
        throw KeyError_t("Key %s is not exists",key.c_str());

    return *(istructData->second);
}

const Value_t& Struct_t::operator[] (const Struct_t::key_type &key) const
{
    const_iterator istructData;

    if ((istructData = structData.find(key)) == structData.end())
        throw KeyError_t("Key %s is not exists",key.c_str());

    return *(istructData->second);
}


void Struct_t::clear()
{
    structData.clear();

}

}
