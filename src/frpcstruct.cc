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
 * FILE          $Id: frpcstruct.cc,v 1.9 2008-05-05 12:52:00 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#include "frpcstruct.h"
#include "frpcpool.h"
#include "frpckeyerror.h"
#include "frpclenerror.h"

namespace FRPC {

Struct_t::Struct_t() {}


Struct_t::~Struct_t() {}

Struct_t::Struct_t(const Struct_t::pair &value) {
    structData.insert(value);
}

Struct_t::Struct_t(const std::string &key, const Value_t &value) {
    if (key.size() > 255) {
        throw LenError_t("Size of member name must be max 255 not %d.",
                         key.size() );
    }
    structData.insert(value_type(key,const_cast<Value_t *>(&value)));
}

Value_t& Struct_t::clone(Pool_t& newPool) const {
    Struct_t *newStruct = &newPool.Struct();

    for (std::map<std::string,Value_t*>::const_iterator
            istructData = structData.begin();
            istructData != structData.end(); ++istructData) {
        newStruct->insert(value_type(istructData->first,
                                     &(istructData->second)->clone(newPool)));
    }

    return  *newStruct;
}

bool Struct_t::has_key(const Struct_t::key_type &key) const {
    if (structData.find(key) != structData.end())
        return true;
    return false;
}

std::pair<Struct_t::iterator, bool> Struct_t::insert(
    const Struct_t::key_type &key, const Value_t &value) {

    return  structData.insert(value_type(key,const_cast<Value_t *>(&value)));
}

std::pair<Struct_t::iterator, bool>
Struct_t::insert(const Struct_t::pair &value) {
    return structData.insert(value);
}

Struct_t::iterator Struct_t::insert(Struct_t::iterator iter, const pair &value)
{
    return structData.insert(iter, value);
}

Struct_t::iterator Struct_t::begin() {
    return structData.begin();
}

Struct_t::iterator Struct_t::end() {
    return structData.end();
}

Struct_t::const_iterator Struct_t::begin() const {
    return structData.begin();
}

Struct_t::const_iterator Struct_t::end() const {
    return structData.end();
}

bool Struct_t::empty() const {
    return structData.empty();
}

Struct_t::size_type Struct_t::size() const {
    return structData.size();
}

Struct_t& Struct_t::append(const Struct_t::pair &value) {
    std::pair<Struct_t::iterator, bool> res = structData.insert(value);
    if (!res.second) {
        res.first->second = value.second;
    }
    return *this;
}

Struct_t& Struct_t::append(const Struct_t::key_type &key,
                           const Value_t &value) {
    if (key.size() > 255) {
        throw LenError_t("Size of member name must be max 255 not %d.",
                         key.size() );
    }

    std::pair<Struct_t::iterator, bool>
        res = structData.insert(value_type(key, const_cast<Value_t *>(&value)));
    if (!res.second) {
        res.first->second = const_cast<Value_t *>(&value);
    }
    return *this;
}

const Value_t* Struct_t::get(const key_type &key) const {
    const_iterator istructData;

    if ((istructData = structData.find(key)) == structData.end())
        return 0;
    return istructData->second;

}

Value_t* Struct_t::get(const key_type &key) {
    iterator istructData;

    if ((istructData = structData.find(key)) == structData.end())
        return 0;
    return istructData->second;

}


Value_t& Struct_t::get(const key_type &key, Value_t &defaultValue){
    iterator istructData;

    if ((istructData = structData.find(key)) == structData.end())
        return defaultValue;

    return *(istructData->second);

}

const Value_t& Struct_t::get(const key_type &key,
                             const Value_t &defaultValue) const{
    const_iterator istructData;

    if ((istructData = structData.find(key)) == structData.end())
        return defaultValue;

    return *(istructData->second);

}

Value_t& Struct_t::operator[] (const Struct_t::key_type &key) {
    iterator istructData;

    if ((istructData = structData.find(key)) == structData.end())
        throw KeyError_t("Key \"%s\" does not exist.",key.c_str());

    return *(istructData->second);
}

const Value_t& Struct_t::operator[] (const Struct_t::key_type &key) const {
    const_iterator istructData;

    if ((istructData = structData.find(key)) == structData.end())
        throw KeyError_t("Key \"%s\" does not exist.",key.c_str());

    return *(istructData->second);
}


void Struct_t::clear() {
    structData.clear();

}

}
