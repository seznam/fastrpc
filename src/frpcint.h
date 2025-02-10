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
 * FILE          $Id: frpcint.h,v 1.9 2008-05-05 12:52:00 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCINT_H
#define FRPCINT_H

#include <cstdint>
#include <frpcvalue.h>
#include <frpctypeerror.h>

namespace FRPC {
class Pool_t;
/**
@brief Int type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Int_t : public Value_t {
    friend class Pool_t;
public:
    enum{ TYPE = 0x01 };

    using value_type = int64_t;

    /**
    @brief Destructor
    */
    ~Int_t() override = default;

    /**
    @brief Getting type of value
    @return  @b unsigned @b short always
    @li @b Int_t::type - identificator of inteeger value
    */
    TypeTag_t getType() const override {return TYPE;}

    /**
        @brief Getting typename of value
        @return @b const @b char  always
        @li @b "Int" - typename of Int_t
    */
    const char* getTypeName() const override {return "int";}

    /**
    @brief Getting internal integer value
    @return   @b long - internal value
    */
    value_type getValue() const {return value;}

    /**
    @brief Operator long
    */
    operator value_type () const {return value;}

    /**
        @brief Method for clone/copy Double_t
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    Value_t& clone(Pool_t &newPool) const override;

    /// static
    static const Int_t &FRPC_ZERO;
    static const Int_t &FRPC_MINUS_ONE;

private:
    /**
    @brief default constructor  is disabled
    */
    Int_t();
    /**
    @brief Costructor from long number
    @param pool is a reference to Pool_t used for allocating
    @param number  is an  long value
    */
    explicit Int_t(const value_type &number)
        :value(number)
    {}

    value_type value; /**  Internal long value */
};
/**
    @brief Inline method

    Used to retype Value_t to Int_t
    @return  If Value_t  can  retype to Int_t return reference to  Int_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t

*/
inline FRPC_DLLEXPORT Int_t& Int(Value_t &value) {
    auto *integer = dynamic_cast<Int_t*>(&value);

    if(!integer)
        throw TypeError_t::format("Type is %s but not int",
                                  value.getTypeName());
    return *integer;
}

/**
    @brief Inline method

    Used to retype Value_t to Int_t
    @return  If Value_t  can  retype to Int_t return reference to  Int_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t

*/
inline FRPC_DLLEXPORT const Int_t& Int(const Value_t &value) {
    const auto *integer = dynamic_cast<const Int_t*>(&value);

    if(!integer)
        throw TypeError_t::format("Type is %s but not int",
                                  value.getTypeName());
    return *integer;
}

} // namespace FRPC

#endif
