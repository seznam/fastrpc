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
 * FILE             $Id: frpcurlunmarshaller.cc,v 1.1 2011-01-10 22:27:04 burlog Exp $
 *
 * DESCRIPTION      Url encoded unmarshaller.
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

#include <map>
#include <cerrno>
#include <utility>
#include <algorithm>
#include <memory.h>

#include "frpcstreamerror.h"
#include "frpctreebuilder.h"
#include "frpcurlunmarshaller.h"

namespace FRPC {
namespace {

/** Returns true if string str ends with suffix.
 */
bool endsWith(const std::string &str, const std::string &suffix) {
    if (suffix.size() > str.size()) return false;
    return std::equal(str.begin() + str.size() - suffix.size(), str.end(),
                      suffix.begin());
}

/** Returns member name (splits array type info).
 */
std::string extractMember(const std::string &member) {
    return endsWith(member, "[]")?
              std::string(member.begin(), member.end() - 2):
              member;
}

/** 
 * @short Reads from given iterator next two bytes and convert its as hexa encoded
 * pair to one byte.
 */
char unhex(const std::string::const_iterator &it) {
    int ch = '\0';

    for (char i = 0; i < 2; ++i) {
        switch (*(it + i)) {
        case '0' ... '9': ch |=  0 + *(it + i) - '0'; break;
        case 'a' ... 'f': ch |= 10 + *(it + i) - 'a'; break;
        case 'A' ... 'F': ch |= 10 + *(it + i) - 'A'; break;
        default: break;
        }
        if (i == 0) ch <<= 4;
    }

    return static_cast<char>(ch);
}

/**
 * @short Decodes given string from url encoding.
 */
std::string urldecode(const std::string &str) {
    std::string res;
    res.reserve(2 * str.size());

    for (std::string::const_iterator
            it = str.begin(), et = str.end();
            it != et; ++it)
    {
        switch (*it) {
        case '%':
            if (std::distance(++it, et) < 2) break;
            res.append(1, unhex(it++));
            break;

        case '+':
            res.append(1, ' ');
            break;

        default:
            res.append(1, *it);
        }
    }

    return res;
}

/** Unqoute quote mark and backslash.
 */
std::string unquote(const std::string &str) {
    std::string res;
    res.reserve(str.size());

    for (std::string::const_iterator
            it = str.begin(), et = str.end();
            it != et; ++it)
    {
        switch (*it) {
        case '\\': {
            std::string::const_iterator inext = it + 1;
            if (inext != et) {
                switch (*inext) {
                case '\\':
                case '"':
                    it = inext;
                default: break;
                }
            }
        }
        default:
            res.append(1, *it);
        }
    }

    return res;
}

/**
 * @short Values agregator. We need collect all values before supplying it to
 * DataBuilder_t so that we can process correctly arrays.
 */
class Values_t {
public:
    /// build method type
    typedef void (Values_t::*Builder_t)(DataBuilder_t &, const std::string &);

    /**
     * @short C'tor.
     * @param buffer url encoded method call attributes.
     */
    explicit Values_t(const std::string &buffer)
        : values()
    {
        std::string::size_type iamp = -1;
        do {
            // find '&' separator
            std::string::size_type ibegin = ++iamp;
            iamp = buffer.find('&', iamp);

            // split to (name, value) pair
            std::string::size_type iequal = buffer.find('=', ibegin);
            if (iequal < iamp) {
                std::string name = buffer.substr(ibegin, iequal - ibegin);
                std::string::size_type
                    num = (iamp == std::string::npos)?
                        iamp:
                        iamp - ibegin - name.size() - 1;
                std::string
                    value = unquote(urldecode(buffer.substr(iequal + 1, num)));

                // extract type info and push value
                Builder_t type = resolveType(value);
                values[name].push_back(std::make_pair(type, value));
#ifndef _DEBUG
                printf("> %s=%s\n", name.c_str(), value.c_str());
#endif
            }
        } while (iamp != std::string::npos);
    }

    /**
     * @short Dump all collected values to builder.
     * @param builder some data builder.
     */
    void dump(DataBuilder_t &builder) {
        // fill global struct
        for (Values_t::Struct_t::const_iterator
                ientry = values.begin(),
                eentry = values.end();
                ientry != eentry; ++ientry)
        {
            // build struct member
            std::string member = extractMember(ientry->first);
            builder.buildStructMember(member);

            // member which ends with [] is array entry
            if (member.size() != ientry->first.size()) {
                builder.openArray(ientry->second.size());
                for (Value_t::const_iterator
                        iarray = ientry->second.begin(),
                        earray = ientry->second.end();
                        iarray != earray; ++iarray)
                {
                    (this->*iarray->first)(builder, iarray->second);
                }
                builder.closeArray();

            } else {
                (this->*ientry->second.front().first)
                    (builder, ientry->second.front().second);
            }
        }
    }

    /**
     * @short Don't builds anything.
     */
    void buildNothing(DataBuilder_t &, const std::string &) {}

    /**
     * @short Builds double value.
     */
    void buildDouble(DataBuilder_t &builder, const std::string &str) {
        // convert to number
        char *end;
        double value = strtod(str.c_str(), &end);
        if (*end)
            throw StreamError_t("Can't convert `%s' to double", str.c_str());
        if (errno == ERANGE)
            throw StreamError_t("Unsupported size of double (too small/big)");

        // and build
        builder.buildDouble(value);
    }

    /**
     * @short Builds int value.
     */
    void buildInt(DataBuilder_t &builder, const std::string &str) {
        // convert to number
        char *end;
        long long int value = strtoll(str.c_str(), &end, 10);
        if (*end)
            throw StreamError_t("Can't convert `%s' to int", str.c_str());
        if (errno == ERANGE)
            throw StreamError_t("Unsupported size of int (too small/big)");

        // and build
        builder.buildInt(value);
    }

    /**
     * @short Builds string value.
     */
    void buildString(DataBuilder_t &builder, const std::string &str) {
        builder.buildString(std::string(str.begin() + 1, str.end() - 1));
    }

    /**
     * @short Builds bool value.
     */
    void buildBool(DataBuilder_t &builder, const std::string &str) {
        builder.buildBool((str == "true")? true: false);
    }

    /**
     * @short Builds null value.
     */
    void buildNull(DataBuilder_t &builder, const std::string &) {
        // only some builders know null
        if (TreeBuilder_t *tBuilder = dynamic_cast<TreeBuilder_t *>(&builder)) {
            tBuilder->buildNull();
        } else {
            dynamic_cast<DataBuilderWithNull_t &>(builder).buildNull();
        }
    }

    /**
     * @short Builds date value.
     */
    void buildDateTime(DataBuilder_t &builder, const std::string &str) {
        // parse datetime
        short year;
        char month, day, hour, minute, sec;
        int tz;
        parseISODateTime(str.data(), str.size(),
                         year, month, day, hour, minute, sec, tz);

        // and build
        builder.buildDateTime(year, month, day, hour, minute, sec, -1, -1, tz);
    }

    /**
     * @short Returns count of global struct members.
     */
    inline std::size_t size() const { return values.size();}

    /**
     * @short Resolves type from name and value.
     * @param value global struct value.
     * @return pointer to build method.
     */
    Builder_t resolveType(const std::string &value) {
        static const char DATETIME_PATTERN[] = "YYYY-MM-DDThh:mm:ss+0000";
        static const std::size_t DATETIME_SIZE = sizeof(DATETIME_PATTERN) - 1;
        if (value.empty()) return &Values_t::buildNothing;

        // resolve type
        switch (*value.begin()) {
        case '"': // string =: "value"
            if ((value.size() == 1) || (*value.rbegin() != '"'))
                throw StreamError_t("String must ends with `\"'");
            return &Values_t::buildString;

        case 'f': // bool =: (false|true)
        case 't':
            return &Values_t::buildBool;

        case 'n': // null =: null
            return &Values_t::buildNull;

        // datetime =: YYYY-MM-DDThh:mm:ss | double =: 1.0 | int =: 1
        case '0'...'9':
            if ((value.size() != DATETIME_SIZE) || (value[10] != 'T')) {
                if (value.find('.', 0) == std::string::npos)
                    return &Values_t::buildInt;
                return &Values_t::buildDouble;
            }
            return &Values_t::buildDateTime;

        default:
            throw StreamError_t("Can't resolve type of value `%s'",
                                value.c_str());
        }
    }

private:
    typedef std::vector<std::pair<Builder_t, std::string> > Value_t;
    typedef std::map<std::string, Value_t> Struct_t;
    Values_t::Struct_t values; //!< global struct
};

} // namespace

URLUnMarshaller_t::URLUnMarshaller_t(DataBuilder_t &dataBuilder,
                                     const std::string &path)
    : dataBuilder(dataBuilder), method(path.empty()? "RPC2": path.data() + 1)
{
    std::replace(method.begin(), method.end(), '/', '.');
}

void URLUnMarshaller_t::unMarshall(const char *data,
                                   unsigned int size,
                                   char type)
{
    if (type != TYPE_METHOD_CALL)
        throw StreamError_t("Unsupported stream type");
    buffer.append(data, size);
}

void URLUnMarshaller_t::finish() {
    // it is method call
    dataBuilder.buildMethodCall(method);

    // parse values and build values map
    Values_t values(buffer);
    dataBuilder.openStruct(values.size());
    values.dump(dataBuilder);
    dataBuilder.closeStruct();
}

} // namespace FRPC

