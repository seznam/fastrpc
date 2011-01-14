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
 * FILE             $Id: frpcjsonmarshaller.cc,v 1.2 2011-01-14 08:23:06 burlog Exp $
 *
 * DESCRIPTION      JSON marshaller
 *
 * PROJECT          FastRPC library.
 *
 * AUTHOR           Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2011
 * All Rights Reserved
 *
 * HISTORY
 *       2011-01-06 (bukovsky)
 *                  First draft.
 */

#include <limits>
#include <sstream>
#include <iomanip>

#include "frpc.h"
#include "frpcwriter.h"
#include "frpcinternals.h"
#include "frpcxmlmarshaller.h"
#include "frpcjsonmarshaller.h"

#ifdef _DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

namespace FRPC {
namespace {

inline void write(Writer_t &writer, const std::string &value) {
    if (!value.empty())
        writer.write(value.data(), value.size());
}

template <typename Context_t>
inline bool dec(Context_t &ctx, Writer_t &writer) {
    if (ctx.empty()) return true;
    switch (ctx.back().second) {
    case 0:
        writer.write(reinterpret_cast<char *>(&ctx.back().first), 1);
        ctx.resize(ctx.size() - 1);
        return dec(ctx, writer);
    default:
        writer.write(",", 1);
        --ctx.back().second;
        break;
    case 1:
        --ctx.back().second;
        return dec(ctx, writer);
    }
    return ctx.empty();
}

std::string escape(unsigned int ch) {
    std::ostringstream os;
    os << "\\u"
       << std::hex
       << std::setfill('0')
       << std::setw(4)
       << ch;
    return os.str();
}

void quote(Writer_t &writer, const char *ipos, unsigned int size) {
    writer.write("\"", 1);
    for (const char *epos = ipos + size; ipos != epos; ++ipos) {
        switch (*ipos) {
        case '"':
            writer.write("\\\"", 2);
            break;
        case '\\':
            writer.write("\\\\", 2);
            break;
        case '\r':
            writer.write("\\r", 2);
            break;
        case '\n':
            writer.write("\\n", 2);
            break;
        case '\t':
            writer.write("\\t", 2);
            break;
        default:
            if (::iscntrl(*ipos)) {
                writer.write(escape(*ipos).c_str(), 6);
            } else {
                writer.write(ipos, 1);
            }
        }
    }
    writer.write("\"", 1);
}

} // namespace

JSONMarshaller_t::JSONMarshaller_t(Writer_t &writer,
                                   const ProtocolVersion_t &protocolVersion)
        : writer(writer), ctx()
{
    if (protocolVersion.versionMajor > FRPC_MAJOR_VERSION)
        throw Error_t("Not supported protocol version");
}

void JSONMarshaller_t::flush() {
    while (!dec(ctx, writer)) {}
    writer.flush();
}

void JSONMarshaller_t::packMethodCall(const char *methodName,
                                      unsigned int size)
{
    throw Error_t("JSON marshaller don't support method calls");
}

void JSONMarshaller_t::packMethodResponse() {}

void JSONMarshaller_t::packFault(int errNumber, const char *errMsg,
                                 unsigned int size)
{
    std::ostringstream os;
    os << "{ \"status\": " << errNumber
       << ", \"statusMessage\": ";
    write(writer, os.str());
    quote(writer, errMsg, size);
    writer.write(" }", 2);
}

void JSONMarshaller_t::packArray(unsigned int numOfItems) {
    DBG("array: #%d\n", numOfItems);
    if (!numOfItems) {
        writer.write("[]", 2);
        dec(ctx, writer);

    } else {
        ctx.push_back(std::make_pair(ARRAY, numOfItems));
        writer.write("[", 1);
    }
}

void JSONMarshaller_t::packStruct(unsigned int numOfMembers) {
    DBG("struct: #%d\n", numOfMembers);
    if (!numOfMembers) {
        writer.write("{}", 2);
        dec(ctx, writer);

    } else {
        ctx.push_back(std::make_pair(STRUCT, numOfMembers));
        writer.write("{", 1);
    }
}

void JSONMarshaller_t::packStructMember(const char *memberName,
                                        unsigned int size)
{
    DBG("member: %.*s\n", size, memberName);
    quote(writer, memberName, size);
    writer.write(":", 1);
}

void JSONMarshaller_t::packBinary(const char *value, unsigned int size) {
    DBG("binary: %.*s\n", size, value);
    writer.write("\"", 1);
    XmlMarshaller_t::writeEncodeBase64(writer, value, size);
    writer.write("\"", 1);
    dec(ctx, writer);
}

void JSONMarshaller_t::packBool(bool value) {
    DBG("bool: %d\n", value);
    write(writer, value? "true": "false");
    dec(ctx, writer);
}

void JSONMarshaller_t::packDouble(double value) {
    DBG("double: %f\n", value);
    std::ostringstream os;
    os << std::setprecision(std::numeric_limits<double>::digits10) << value;
    write(writer, os.str());
    dec(ctx, writer);
}

void JSONMarshaller_t::packInt(Int_t::value_type value) {
    DBG("int: %li\n", value);
    std::ostringstream os;
    os << value;
    write(writer, os.str());
    dec(ctx, writer);
}

void JSONMarshaller_t::packString(const char *value, unsigned int size) {
    DBG("string: %.*s\n", size, value);
    quote(writer, value, size);
    dec(ctx, writer);
}

void JSONMarshaller_t::packDateTime(short, char, char, char, char, char, char,
                                    time_t unixTime, int)
{
    DBG("datetime: %lu\n", unixTime);
    std::ostringstream os;
    os << unixTime;
    write(writer, os.str());
    dec(ctx, writer);
}

void JSONMarshaller_t::packNull() {
    DBG("null: \n");
    writer.write("null", 4);
    dec(ctx, writer);
}

} // namespace FRPC

