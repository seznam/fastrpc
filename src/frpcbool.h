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
 * FILE          $Id: frpcbool.h,v 1.7 2008-05-05 12:52:00 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCBOOL_H
#define FRPCBOOL_H

#include <frpcvalue.h>
#include <frpctypeerror.h>

namespace FRPC {
class Pool_t;

/**
@brief Bool type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Bool_t: public Value_t {
    friend class Pool_t;
public:
    enum{TYPE = 0x02};
    using value_type = bool;

    /**
        @brief Destructor
    */
    ~Bool_t() override;

    /**
        @brief Getting type of value
        @return @b unsigned @b short always
        @li @b Bool_t::TYPE - identificator of boolean value
    */
    unsigned short getType() const override
    {
        return TYPE;
    }
    /**
        @brief Getting typename of value
        @return @b const @b char* always
        @li @b "Bool" - typename of Bool_t
    */
    const char* getTypeName() const override
    {
        return "bool";
    }
    /**
        @brief Getting internal boolean value
        @return  @b bool - internal value
    */
    bool getValue() const
    {
        return value;
    }

    /**
        @brief Operator bool const
    */
    operator bool() const
    {
        return value;
    }

    /**
        @brief Method to clone/copy Bool_t
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    Value_t& clone(Pool_t &newPool) const override;

    ///static members
    static const Bool_t &FRPC_TRUE;
    static const Bool_t &FRPC_FALSE;

private :
    /**
        @brief Default constructor
    */
    Bool_t() = default;

    /**
        @brief Costructor from bool value
        @param pool is a reference to Pool_t used for allocating
        @param boolean  is a bool value
    */
    explicit Bool_t(const bool &boolean)
        :value(boolean)
    {}

    bool value{}; /**  Internal bool value */
};

/**
    @brief Inline method

    Used to retype Value_t to Bool_t
    @return  If Value_t  can  retype to Bool_t return reference to Bool_t
    @n If Value_t can't retype to Bool_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT Bool_t& Bool(Value_t &value)
{
    auto *boolean = dynamic_cast<Bool_t*>(&value);

    if(!boolean)
        throw TypeError_t::format("Type is %s but not bool",
                                  value.getTypeName());

    return *boolean;
}

inline FRPC_DLLEXPORT const Bool_t& Bool(const Value_t &value)
{
    const auto *boolean = dynamic_cast<const Bool_t*>(&value);

    if(!boolean)
        throw TypeError_t::format("Type is %s but not bool",
                                  value.getTypeName());

    return *boolean;
}

} // namespace FRPC

#endif
