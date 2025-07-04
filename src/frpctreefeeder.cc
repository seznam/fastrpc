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

#include <variant>

#include "frpcsecret.h"
#include "frpcstring_view.h"
#include "frpcbinmarshaller.h"
#include "frpcxmlmarshaller.h"
#include "frpcjsonmarshaller.h"
#include "frpcerror.h"
#include "frpctreefeeder.h"

namespace FRPC {
namespace {

template <typename Type_t>
using when =  std::enable_if_t<
    Type_t::value,
    bool
>;

template <typename Type_t>
using neg =  std::conditional_t<
    Type_t::value,
    std::false_type,
    std::true_type
>;

template <typename MarshallerT>
struct has_non_virtual_api: public std::false_type {};

template <>
struct has_non_virtual_api<BinMarshaller_t>: public std::true_type {};

template <>
struct has_non_virtual_api<XmlMarshaller_t>: public std::true_type {};

template <>
struct has_non_virtual_api<JSONMarshaller_t>: public std::true_type {};

/** Returns escaped pointer segment according to JSON Pointer RFC 6901 with
 * extension for commas - they are escaped as well using same schema with
 * '~2' value.
 */
std::string escape_segment(std::string_view value) {
    std::string result;
    for (auto ch: value) {
        switch (ch) {
        case ',':
            result += "~2";
            break;
        case '/':
            result += "~1";
            break;
        case '~':
            result += "~0";
            break;
        default:
            result += ch;
            break;
        }
    }
    return result;
}

/** Secret manager without any secrets.
 */
struct VoidSecrets_t {
    template <typename KeyT>
    void push(const KeyT &) {}
    void pop() {}
    void secret_found() {}
};

/** Secret manager that collects secrets in a vector.
 */
struct Secrets_t {
    /** Push a new key to the pointer stack.
     */
    template <typename KeyT>
    void push(const KeyT &key) {pointer.emplace_back(key);}

    /** Pop the last key from the pointer stack.
     */
    void pop() {pointer.pop_back();}

    /** Mark the current pointer as a secret.
     */
    void secret_found() {secrets.push_back(str());}

    /** Get the current pointer as a string.
     */
    std::string str() const {
        std::string result;
        for (const auto &item: pointer) {
            result += '/';
            std::visit(
                [&] (auto &&item) {
                    using ItemTypeT = std::decay_t<decltype(item)>;
                    if constexpr (std::is_same_v<ItemTypeT, std::string>) {
                        result += escape_segment(item);
                    } else if constexpr (std::is_integral_v<ItemTypeT>) {
                        result += std::to_string(item);
                    }
                },
                item
            );
        }
        return !result.empty()? result: "/";
    }

    std::vector<std::string> &secrets;
    std::vector<std::variant<std::string, std::size_t>> pointer;
};

/** The RAII object that extends the pointer stack with a new key and
 * pops it on destruction.
 */
template <typename SecretsT>
struct PointerExtender_t {
    /** C'tor.
     */
    template <typename KeyT>
    PointerExtender_t(SecretsT &secrets, const KeyT &key)
        : secrets(secrets)
    {
        secrets.push(key);
    }

    // no copy or move
    PointerExtender_t(const PointerExtender_t &) = delete;
    PointerExtender_t &operator=(const PointerExtender_t &) = delete;
    PointerExtender_t(PointerExtender_t &&) = delete;
    PointerExtender_t &operator=(PointerExtender_t &&) = delete;

    /** D'tor.
     */
    ~PointerExtender_t() {secrets.pop();}

    SecretsT &secrets; //!< reference to the secrets manager
};

/** Feed value implementation that packs the value into the marshaller.
 */
template <typename SecretsT, typename MarshallerT>
void feedValueImpl(MarshallerT &marshaller, const Value_t &value, SecretsT &secrets);

/** Pack Null_t into the marshaller.
 */
template <
    typename MarshallerT,
    when<neg<has_non_virtual_api<MarshallerT>>> = true
> void packNull(MarshallerT &, const Value_t &) {
    throw Error_t("Unknown marshaller, we don't known how to pack null");
}

/** Pack Null_t into the marshaller.
 */
template <
    typename MarshallerT,
    when<has_non_virtual_api<MarshallerT>> = true
> void packNull(MarshallerT &marshaller, const Value_t &) {
    marshaller.packNull();
}

/** Pack String_t into the marshaller.
 */
template <typename StringT>
void packString(Marshaller_t &marshaller, const StringT &str) {
    marshaller.packString(str.data(), static_cast<uint32_t>(str.size()));
}

/** Pack String_t into the marshaller.
 */
template <
    typename MarshallerT,
    when<neg<has_non_virtual_api<MarshallerT>>> = true
> void packBinaryRef(MarshallerT &, const Value_t &) {
    throw Error_t("Unknown marshaller, we don't known how to pack binaryref");
}

/** Pack BinaryRef_t into the marshaller.
 */
template <
    typename MarshallerT,
    when<has_non_virtual_api<MarshallerT>> = true
> void packBinaryRef(MarshallerT &marshaller, const BinaryRef_t &bin) {
    marshaller.packBinaryRef(bin.getFeeder());
}

/** Pack Binary_t into the marshaller.
 */
void packBinary(Marshaller_t &marshaller, const Binary_t &bin) {
    marshaller.packBinary(bin.data(), static_cast<uint32_t>(bin.size()));
}

/** Pack DateTime into the marshaller.
 */
void packDateTime(Marshaller_t &marshaller, const DateTime_t &dt) {
    marshaller.packDateTime(
        dt.getYear(),
        static_cast<char>(dt.getMonth()),
        static_cast<char>(dt.getDay()),
        static_cast<char>(dt.getHour()),
        static_cast<char>(dt.getMin()),
        static_cast<char>(dt.getSec()),
        static_cast<char>(dt.getDayOfWeek()),
        dt.getUnixTime(),
        dt.getTimeZone()
    );
}

/** Feed value implementation that packs the struct into the marshaller.
 */
template <typename SecretsT, typename MarshallerT>
void packStruct( // NOLINT(misc-no-recursion)
    MarshallerT &marshaller,
    const Struct_t &value,
    SecretsT &secrets
) {
    marshaller.packStruct(static_cast<uint32_t>(value.size()));
    for (const auto &[key, value]: value) {
        PointerExtender_t<SecretsT> ext(secrets, key);
        marshaller.packStructMember(key.data(), static_cast<uint32_t>(key.size()));
        feedValueImpl(marshaller, *value, secrets);
    }
}

/** Feed value implementation that packs the array into the marshaller.
 */
template <typename SecretsT, typename MarshallerT>
void packArray( // NOLINT(misc-no-recursion)
    MarshallerT &marshaller,
    const Array_t &array,
    SecretsT &secrets
) {
    marshaller.packArray(static_cast<uint32_t>(array.size()));
    for (auto i = 0u; i < array.size(); ++i) {
        PointerExtender_t<SecretsT> ext(secrets, i);
        feedValueImpl(marshaller, array[i], secrets);
    }
}

/** Feed value implementation that packs the value into the marshaller.
 */
template <typename SecretsT, typename MarshallerT>
void feedValueImpl( // NOLINT(misc-no-recursion)
    MarshallerT &marshaller,
    const Value_t &value,
    SecretsT &secrets
) {
    switch(value.getType()) {
    case Int_t::TYPE:
        marshaller.packInt(Int(value).getValue());
        return;

    case Bool_t::TYPE:
        marshaller.packBool(Bool(value).getValue());
        return;

    case Null_t::TYPE:
        packNull(marshaller, value);
        return;

    case Double_t::TYPE:
        marshaller.packDouble(Double(value).getValue());
        return;

    case String_t::TYPE:
        packString(marshaller, String(value));
        return;

    case StringView_t::TYPE:
        packString(marshaller, StringView(value));
        return;

    case BinaryRef_t::TYPE:
        packBinaryRef(marshaller, BinaryRef(value));
        return;

    case Binary_t::TYPE:
        packBinary(marshaller, Binary(value));
        return;

    case DateTime_t::TYPE:
        packDateTime(marshaller, DateTime(value));
        return;

    case Struct_t::TYPE:
        packStruct(marshaller, Struct(value), secrets);
        return;

    case Array_t::TYPE:
        packArray(marshaller, Array(value), secrets);
        return;

    case SecretValue_t::TYPE: {
        secrets.secret_found();
        feedValueImpl(marshaller, SecretValue(value).getValue(), secrets);
        return;
    }

    default:
        break;
    }
    throw Error_t(
        "Unknown value type: type=" + std::to_string(value.getType())
    );
}

/** Feed value implementation that packs the value into the marshaller
 * and returns the result.
 */
template <typename ResultT = void, typename MarshallerT>
ResultT feedValueImpl(MarshallerT &marshaller, const Value_t &value) {
    if constexpr (std::is_same_v<ResultT, void>) {
        VoidSecrets_t secrets;
        return feedValueImpl(marshaller, value, secrets);

    } else {
        ResultT result;
        Secrets_t secrets = {result, {}};
        feedValueImpl(marshaller, value, secrets);
        return result;
    }
}

} // namespace

void TreeFeeder_t::feedValue(const Value_t &value){
    if (auto *m = dynamic_cast<BinMarshaller_t *>(&marshaller))
        return feedValueImpl(*m, value);
    if (auto *m = dynamic_cast<XmlMarshaller_t *>(&marshaller))
        return feedValueImpl(*m, value);
    if (auto *m = dynamic_cast<JSONMarshaller_t *>(&marshaller))
        return feedValueImpl(*m, value);
    return feedValueImpl(marshaller, value);
}

std::vector<std::string>
TreeFeeder_t::feedValueAndGatherSecrets(const Value_t &value) {
    if (auto *m = dynamic_cast<BinMarshaller_t *>(&marshaller))
        return feedValueImpl<std::vector<std::string>>(*m, value);
    if (auto *m = dynamic_cast<XmlMarshaller_t *>(&marshaller))
        return feedValueImpl<std::vector<std::string>>(*m, value);
    if (auto *m = dynamic_cast<JSONMarshaller_t *>(&marshaller))
        return feedValueImpl<std::vector<std::string>>(*m, value);
    return feedValueImpl<std::vector<std::string>>(marshaller, value);
}

TreeFeeder_t::~TreeFeeder_t() = default;

} // namespace FRPC
