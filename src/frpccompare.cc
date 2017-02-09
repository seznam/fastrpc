/*
 * FastRPC -- Fast RPC library compatible with XML-RPC
 * Copyright (C) 2005-8  Seznam.cz, a.s.
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
 * FILE             $Id: frpccompare.h,v 1.4 2011-02-16 09:23:09 burlog Exp $
 *
 * DESCRIPTION      Compare generic values - implementation.
 *
 * PROJECT          FastRPC library.
 *
 * AUTHOR           Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2010
 * All Rights Reserved
 *
 * HISTORY
 *       2017-02-07 (volejnik)
 *                  First draft.
 */

#include <stdexcept>
#include "frpccompare.h"

namespace FRPC {

bool operator==(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) == 0;
}

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if values is not same.
 */
bool operator!=(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) != 0;
}

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is less then rhs.
 */
bool operator<(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) < 0;
}

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is greater then rhs.
 */
bool operator>(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) > 0;
}

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is less or equal then rhs.
 */
bool operator<=(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) <= 0;
}

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is greater or equal then rhs.
 */
bool operator>=(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) >= 0;
}

template <typename Value_T>
static int compareValue(const Value_T &lhs, const Value_T &rhs) {
    return (lhs < rhs)? -1
        : ((rhs < lhs)? 1: 0);
}

template <typename Value_T>
static int compareValue(const Value_t &lhs, const Value_t &rhs) {
    const Value_T &lhsc = static_cast<const Value_T&>(lhs);
    const Value_T &rhsc = static_cast<const Value_T&>(rhs);
    return (lhsc.getValue() < rhsc.getValue())? -1
        : ((rhsc.getValue() < lhsc.getValue())? 1: 0);
}

template <>
int compareValue<FRPC::DateTime_t>(const FRPC::Value_t &lhs,
                                   const FRPC::Value_t &rhs)
{
    const DateTime_t &lhsc = static_cast<const DateTime_t&>(lhs);
    const DateTime_t &rhsc = static_cast<const DateTime_t&>(rhs);
    return (lhsc.getUnixTime() < rhsc.getUnixTime())? -1
        : ((rhsc.getUnixTime() < lhsc.getUnixTime())? 1: 0);
}

static int compare(const FRPC::Struct_t &lhs, const FRPC::Struct_t &rhs) {
    FRPC::Struct_t::const_iterator ilhs = lhs.begin();
    FRPC::Struct_t::const_iterator irhs = rhs.begin();
    FRPC::Struct_t::const_iterator elhs = lhs.end();
    FRPC::Struct_t::const_iterator erhs = rhs.end();

    for (; (ilhs != elhs) && (irhs != erhs); ++ilhs, ++irhs) {
        // compare the key
        if (int res = ilhs->first.compare(irhs->first)) return res;
        // compare the value
        if (int res = compare(*ilhs->second, *irhs->second)) return res;
    }

    // check ends of arrays
    if (ilhs != elhs) return 1;
    if (irhs != erhs) return -1;
    return 0;
}

static int compare(const FRPC::Array_t &lhs, const FRPC::Array_t &rhs) {
    FRPC::Array_t::const_iterator ilhs = lhs.begin();
    FRPC::Array_t::const_iterator irhs = rhs.begin();
    FRPC::Array_t::const_iterator elhs = lhs.end();
    FRPC::Array_t::const_iterator erhs = rhs.end();

    for (; (ilhs != elhs) && (irhs != erhs); ++ilhs, ++irhs)
        // compare the value
        if (int res = compare(**ilhs, **irhs)) return res;

    // check ends of arrays
    if (ilhs != elhs) return 1;
    if (irhs != erhs) return -1;
    return 0;
}

int compare(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    // compare the type
    if (int res = compareValue(lhs.getType(), rhs.getType())) return res;

    // compare the value
    switch (lhs.getType()) {
    case FRPC::Bool_t::TYPE:
        return compareValue<FRPC::Bool_t>(lhs, rhs);
    case FRPC::Int_t::TYPE:
        return compareValue<FRPC::Int_t>(lhs, rhs);
    case FRPC::Double_t::TYPE:
        return compareValue<FRPC::Double_t>(lhs, rhs);
    case FRPC::String_t::TYPE:
        return compareValue<FRPC::String_t>(lhs, rhs);
    case FRPC::Binary_t::TYPE:
        return compareValue<FRPC::Binary_t>(lhs, rhs);
    case FRPC::DateTime_t::TYPE:
        return compareValue<FRPC::DateTime_t>(lhs, rhs);
    case FRPC::Struct_t::TYPE:
        return compare(FRPC::Struct(lhs), FRPC::Struct(rhs));
    case FRPC::Array_t::TYPE:
        return compare(FRPC::Array(lhs), FRPC::Array(rhs));
    case FRPC::Null_t::TYPE:
        return 0;
    default:
        break;
    }
    throw std::runtime_error("FRPC::compare(lhs, rhs)");
}

} // namespace FRPC
