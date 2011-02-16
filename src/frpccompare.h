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
 * FILE             $Id: frpccompare.h,v 1.3 2011-02-16 09:17:05 burlog Exp $
 *
 * DESCRIPTION      Comparing generic values.
 *
 * PROJECT          FastRPC library.
 *
 * AUTHOR           Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2010
 * All Rights Reserved
 *
 * HISTORY
 *       2010-09-28 (bukovsky)
 *                  First draft.
 */

#ifndef FRPC_FRPCCOMPARE_H
#define FRPC_FRPCCOMPARE_H

#include <frpcbool.h>
#include <frpcint.h>
#include <frpcdouble.h>
#include <frpcstring.h>
#include <frpcbinary.h>
#include <frpcdatetime.h>
#include <frpcstruct.h>
#include <frpcarray.h>
#include <frpcnull.h>

namespace FRPC {

/** 
 * @short Compares two FastRPC values and returns zero if it is equals. 1 if
 * lhs is greater and -1 if lhs is less.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return 0, -1, 1 if equals, is lhs is greater or lhs is less.
 */
int compare(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs);

/** 
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if values is same.
 */
inline bool operator==(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) == 0;
}

/** 
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if values is not same.
 */
inline bool operator!=(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) != 0;
}

/** 
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is less then rhs.
 */
inline bool operator<(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) < 0;
}

/** 
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is greater then rhs.
 */
inline bool operator>(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) > 0;
}

/** 
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is less or equal then rhs.
 */
inline bool operator<=(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) <= 0;
}

/** 
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is greater or equal then rhs.
 */
inline bool operator>=(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    return compare(lhs, rhs) >= 0;
}

namespace helper {

template <typename Value_t>
inline int compare(const Value_t &lhs, const Value_t &rhs) {
    return (lhs < rhs)? -1: ((rhs < lhs)? 1: 0);
}

template <>
inline int compare(const FRPC::Bool_t &lhs, const FRPC::Bool_t &rhs) {
    return compare(lhs.getValue(), rhs.getValue());
}

template <>
inline int compare(const FRPC::Int_t &lhs, const FRPC::Int_t &rhs) {
    return compare(lhs.getValue(), rhs.getValue());
}

template <>
inline int compare(const FRPC::Double_t &lhs, const FRPC::Double_t &rhs) {
    return compare(lhs.getValue(), rhs.getValue());
}

template <>
inline int compare(const FRPC::String_t &lhs, const FRPC::String_t &rhs) {
    return compare(lhs.getString(), rhs.getString());
}

template <>
inline int compare(const FRPC::Binary_t &lhs, const FRPC::Binary_t &rhs) {
    return compare(lhs.getString(), rhs.getString());
}

template <>
inline int compare(const FRPC::DateTime_t &lhs, const FRPC::DateTime_t &rhs) {
    return compare(lhs.getUnixTime(), rhs.getUnixTime());
}

int compare(const FRPC::Struct_t &lhs, const FRPC::Struct_t &rhs) {
    FRPC::Struct_t::const_iterator ilhs = lhs.begin();
    FRPC::Struct_t::const_iterator irhs = rhs.begin();
    FRPC::Struct_t::const_iterator elhs = lhs.end();
    FRPC::Struct_t::const_iterator erhs = rhs.end();

    for (; (ilhs != elhs) && (irhs != erhs); ++ilhs, ++irhs) {
        // compare the key
        if (int res = compare(ilhs->first, irhs->first)) return res;
        // compare the value
        if (int res = compare(*ilhs->second, *irhs->second)) return res;
    }

    // check ends of arrays
    if (ilhs != elhs) return 1;
    if (irhs != erhs) return -1;
    return 0;
}

int compare(const FRPC::Array_t &lhs, const FRPC::Array_t &rhs) {
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

} // namespace helper

int compare(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs) {
    // compare the type
    if (int res = helper::compare(lhs.getType(), rhs.getType())) return res;

    // compare the value
    switch (lhs.getType()) {
    case FRPC::Bool_t::TYPE:
        return helper::compare(FRPC::Bool(lhs), FRPC::Bool(rhs));
    case FRPC::Int_t::TYPE:
        return helper::compare(FRPC::Int(lhs), FRPC::Int(rhs));
    case FRPC::Double_t::TYPE:
        return helper::compare(FRPC::Double(lhs), FRPC::Double(rhs));
    case FRPC::String_t::TYPE:
        return helper::compare(FRPC::String(lhs), FRPC::String(rhs));
    case FRPC::Binary_t::TYPE:
        return helper::compare(FRPC::Binary(lhs), FRPC::Binary(rhs));
    case FRPC::DateTime_t::TYPE:
        return helper::compare(FRPC::DateTime(lhs), FRPC::DateTime(rhs));
    case FRPC::Struct_t::TYPE:
        return helper::compare(FRPC::Struct(lhs), FRPC::Struct(rhs));
    case FRPC::Array_t::TYPE:
        return helper::compare(FRPC::Array(lhs), FRPC::Array(rhs));
    case FRPC::Null_t::TYPE:
        return true;
    default:
        break;
    }
    throw std::runtime_error("FRPC::compare(lhs, rhs)");
}

} // namespace FRPC

#endif /* FRPC_FRPCCOMPARE_H */

