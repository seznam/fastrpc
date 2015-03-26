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
 * FILE          $Id: frpcvalue.h,v 1.7 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCVALUE_H
#define FRPCVALUE_H

#include <frpcplatform.h>

#include <string>
#include <vector>
#include <map>
#include "frpctypeerror.h"

namespace FRPC
{
class Pool_t;
/**
@brief Abstract Value type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Value_t
{
    friend class Pool_t;
public:
    /**
        @brief Default constructor
        @param pool is reference to Pool_t (memory pool)
    */
    Value_t()
    {}
    /**
        @brief  Destructor
    */
    virtual ~Value_t();
    /**
        @brief Abstract virtual method to getting type of value
        @return @b unsigned  @b short type of value
        @li @b  Int_t::TYPE         - inteeger value type
        @li @b  Bool_t::TYPE        - boolean value type
        @li @b  Double_t::TYPE      - double value type
        @li @b  String_t::TYPE      - string value type
        @li @b  DateTime_t::TYPE    - date time value type
        @li @b  Binary_t::TYPE      - binary value type

        @li @b  Struct_t::TYPE      - struct value type
        @li @b  Arry_t::TYPE        - array value type
    */
    virtual unsigned short getType() const = 0;

    /**
        @brief Abstract virtual method to getting typename of value
        @return @b const @b char* typename of value
        @li @b  "Int"           - inteeger value typename
        @li @b  "Bool"          - boolean value typename
        @li @b  "Double"        - double value typename
        @li @b  "String"        - string value typename
        @li @b  "DateTime"      - date time value typename
        @li @b  "Binary"        - binary value typename

        @li @b  "Struct"        - struct value typename
        @li @b  "Array"         - array value typename
    */
    virtual const char* getTypeName() const = 0;


    /**
       @brief Abstract virtual method to clone/copy Value_t
       @param newPool is pointer of Pool_t which is used for allocate objects
       */
    virtual Value_t& clone(Pool_t &newPool) const = 0;

    bool isNull() const;

    /**
        @brief Virtual method to get item from array
        @param index              index in array
        @return @b Value_t        object from index
    */

    virtual Value_t& operator[] (std::vector<Value_t*>::size_type index);
    virtual const Value_t& operator[] (std::vector<Value_t*>::size_type index) const;

    /**
        @brief Virtual method to get item from struct
        @param index              index in struct
        @return @b Value_t        object from index
    */
    virtual Value_t& operator[] (const std::map<std::string,Value_t*>::key_type &key);
    virtual const Value_t& operator[] (const std::map<std::string,Value_t*>::key_type &key) const;


private:

    /**
        @brief Default constructor is disabled
    */
    explicit Value_t(const Value_t&);

    Value_t& operator=(const Value_t&);
};

}

#endif
