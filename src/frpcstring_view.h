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
 * FILE          $Id: frpcstring.h,v 1.7 2008-05-05 12:52:00 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCSTRINGVIEW_H
#define FRPCSTRINGVIEW_H

#if __cplusplus >= 201703L
#include <string_view>
#endif /* __cplusplus < 201703L */

#include <frpcvalue.h>
#include <frpcpool.h>

namespace FRPC {

#if __cplusplus < 201703L
struct string_view {
    const char *data() const {return ptr;}
    std::size_t size() const {return length;}
    const char *ptr;
    std::size_t length;
};
#endif /* __cplusplus < 201703L */

/**
@brief String view type
*/
class FRPC_DLLEXPORT StringView_t : public Value_t {
public:
    enum { TYPE = 0x0E };

#if __cplusplus < 201703L
    typedef string_view value_type;
#else
    typedef std::string_view value_type;
#endif /* __cplusplus < 201703L */

    virtual ~StringView_t() = default;

    /**
        @brief Getting type of value
        @return  @b unsigned @b short always
        @li @b Binary_t::TYPE - identificator of binary value
    */
    TypeTag_t getType() const override {return TYPE;}

    /**
        @brief Getting typename of value
        @return @b const @b char* always
        @li @b "String" - typename of String_t
    */
    const char* getTypeName() const override {return "string_view";}

    /**
        @brief Get data itself. Data are not "\0"-terminated.
        @return Pointer to the binary data.
    */
    std::size_t size() const {return length;}

    /**
        @brief Get data itself. Data are not "\0"-terminated.
        @return Pointer to the binary data.
    */
    const char *data() const {return ptr;}

    /**
        @brief Get binary data as STL string.
        @return Binary data as string.
    */
    std::string getString() const {return std::string(ptr, length);}

    /**
        @brief Get binary data as STL string.
        @return Binary data as string.
    */
    value_type getValue() const {return {ptr, length};}

    /**
        @brief Method to clone/copy StringView_t
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    Value_t &clone(Pool_t &newPool) const override;

    /**
        @brief operator const std::string_view
    */
    operator value_type() const {return {ptr, length};}

    ///static members
    static const StringView_t &FRPC_EMPTY;

    /**
        @brief Validates data, must be valid utf-8 byte sequence in range
        http://www.w3.org/TR/xml/#NT-Char
        @param ptr - is a data pointer
        @param length - is a size of data in bytes
    */
    static void validateBytes(const char *ptr, std::size_t length);

private:
    friend class Pool_t;

    /**
        @brief Default constructor is disabled
    */
    StringView_t();

    /**
        @brief Constructor  from pointer to data and data size
        @param ptr - is a unsigned char pointer to data
        @param length - is a size of data in bytes
    */
    StringView_t(const char *ptr, std::size_t length)
        : ptr(ptr), length(length)
    {
        //WARNING: Pointer to raw data, not null-terminated
        validateBytes(ptr, length);
    }

    /**
       @brief Constructor from std::string value
       @param value  - is a std::string_view of data
    */
    explicit StringView_t(const std::string &value)
        : StringView_t(value.data(), value.size())
    {}

    /**
       @brief Constructor from std::string value
       @param value  - is a std::string_view of data
    */
    explicit StringView_t(value_type value)
        : StringView_t(value.data(), value.size())
    {}

    const char *ptr;    //!< pointer to data
    std::size_t length; //!< size of data
};

/**
    @brief Inline method used to retype Value_t to StringView_t
    @return If Value_t can retype to String_t return reference to StringView_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t
*/
inline FRPC_DLLEXPORT StringView_t &StringView(Value_t &value) {
    if (auto *string_v = dynamic_cast<StringView_t *>(&value))
        return *string_v;
    throw TypeError_t::format(
        "Type is %s but not string_view",
        value.getTypeName()
    );
}

/**
    @brief Inline method used to retype Value_t to StringView_t
    @return If Value_t can retype to String_t return reference to StringView_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t
*/
inline FRPC_DLLEXPORT const StringView_t &StringView(const Value_t &value) {
    if (auto *string_v = dynamic_cast<const StringView_t *>(&value))
        return *string_v;
    throw TypeError_t::format(
        "Type is %s but not string_view",
        value.getTypeName()
    );
}

} // namespace FRPC

#endif /* FRPCSTRINGVIEW_H */
