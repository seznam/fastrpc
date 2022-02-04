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
 * FILE          $Id: frpctreefeeder.cc,v 1.6 2011-01-10 22:25:15 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include "frpcstring_view.h"
#include "frpcbinmarshaller.h"
#include "frpcxmlmarshaller.h"
#include "frpcjsonmarshaller.h"
#include "frpcerror.h"
#include "frpctreefeeder.h"

namespace FRPC {
namespace {

template <typename Type_t>
using when = typename std::enable_if<
    Type_t::value,
    bool
>::type;

template <typename Type_t>
using neg = typename std::conditional<
    Type_t::value,
    std::false_type,
    std::true_type
>::type;

template <typename Marshaller_t>
struct has_non_virtual_api: public std::false_type {};

template <>
struct has_non_virtual_api<BinMarshaller_t>: public std::true_type {};

template <>
struct has_non_virtual_api<XmlMarshaller_t>: public std::true_type {};

template <>
struct has_non_virtual_api<JSONMarshaller_t>: public std::true_type {};

template <typename Marshaller_t>
void feedValueImpl(Marshaller_t &marshaller, const Value_t &value);

template <
    typename Marshaller_t,
    when<neg<has_non_virtual_api<Marshaller_t>>> = true
> void packNull(Marshaller_t &, const Value_t &) {
    throw Error_t("Unknown marshaller, we don't known how to pack null");
}

template <
    typename Marshaller_t,
    when<has_non_virtual_api<Marshaller_t>> = true
> void packNull(Marshaller_t &marshaller, const Value_t &) {
    marshaller.packNull();
}

template <typename StringT>
void packString(Marshaller_t &marshaller, StringT &&str) {
    marshaller.packString(str.data(), str.size());
}

template <
    typename Marshaller_t,
    when<neg<has_non_virtual_api<Marshaller_t>>> = true
> void packBinaryRef(Marshaller_t &, const Value_t &) {
    throw Error_t("Unknown marshaller, we don't known how to pack binaryref");
}

template <
    typename Marshaller_t,
    when<has_non_virtual_api<Marshaller_t>> = true
> void packBinaryRef(Marshaller_t &marshaller, const BinaryRef_t &bin) {
    marshaller.packBinaryRef(bin.getFeeder());
}

void packBinary(Marshaller_t &marshaller, const Binary_t &bin) {
    marshaller.packBinary(bin.data(), bin.size());
}

void packDateTime(Marshaller_t &marshaller, const DateTime_t &dt) {
    marshaller.packDateTime(dt.getYear(), dt.getMonth(), dt.getDay(),
                            dt.getHour(), dt.getMin(), dt.getSec(),
                            dt.getDayOfWeek(), dt.getUnixTime(),
                            dt.getTimeZone());
}

template <typename Marshaller_t>
void packStruct(Marshaller_t &marshaller, const Struct_t &value) {
    marshaller.packStruct(value.size());
    for (auto &item: value) {
        marshaller.packStructMember(item.first.data(), item.first.size());
        feedValueImpl(marshaller, *item.second);
    }
}

template <typename Marshaller_t>
void packArray(Marshaller_t &marshaller, const Array_t &array) {
    marshaller.packArray(array.size());
    for (auto &item: array) feedValueImpl(marshaller, *item);
}

template <typename Marshaller_t>
void feedValueImpl(Marshaller_t &marshaller, const Value_t &value) {
    switch(value.getType()) {
    case Int_t::TYPE:
        marshaller.packInt(Int(value).getValue());
        break;

    case Bool_t::TYPE:
        marshaller.packBool(Bool(value).getValue());
        break;

    case Null_t::TYPE:
        packNull(marshaller, value);
        break;

    case Double_t::TYPE:
        marshaller.packDouble(Double(value).getValue());
        break;

    case String_t::TYPE:
        packString(marshaller, String(value));
        break;

    case StringView_t::TYPE:
        packString(marshaller, StringView(value));
        break;

    case BinaryRef_t::TYPE:
       packBinaryRef(marshaller, BinaryRef(value));
       break;

    case Binary_t::TYPE:
        packBinary(marshaller, Binary(value));
        break;

    case DateTime_t::TYPE:
        packDateTime(marshaller, DateTime(value));
        break;

    case Struct_t::TYPE:
        packStruct(marshaller, Struct(value));
        break;

    case Array_t::TYPE:
        packArray(marshaller, Array(value));
        break;

    default:
        throw Error_t("Unknown value type: type="
                      + std::to_string(value.getType()));
    }
}

} // namespace

void TreeFeeder_t::feedValue(const Value_t &value){
    if (auto *m = dynamic_cast<BinMarshaller_t *>(&marshaller))
        return feedValueImpl(*m, value);
    else if (auto *m = dynamic_cast<XmlMarshaller_t *>(&marshaller))
        return feedValueImpl(*m, value);
    else if (auto *m = dynamic_cast<JSONMarshaller_t *>(&marshaller))
        return feedValueImpl(*m, value);
    return feedValueImpl(marshaller, value);
}

TreeFeeder_t::~TreeFeeder_t() {}

} // namespace FRPC

