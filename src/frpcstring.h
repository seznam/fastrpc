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
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCSTRING_H
#define FRPCSTRING_H

#include <string>
#include <frpcvalue.h>
#include <frpctypeerror.h>

namespace FRPC {
class Pool_t;

/**
@brief String type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT String_t : public Value_t {
    friend class Pool_t;
public:
    enum{ TYPE = TYPE_STRING };

    using value_type = std::string;

    ~String_t() override;

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
    const char* getTypeName() const override {return "string";}

    /**
        @brief Get data itself. Data are not "\0"-terminated.
        @return Pointer to the binary data.
    */
    std::string::size_type size() const;

    /**
        @brief Get data itself. Data are not "\0"-terminated.
        @return Pointer to the binary data.
    */
    const std::string::value_type* data() const;

    /**
        @brief Get binary data as STL string.
        @return Binary data as string.
    */
    //FIXME: const std::string&
    std::string getString() const;

    /**
        @brief Get binary data as STL string.
        @return Binary data as string.
    */
    const std::string& getValue() const;

    /**
        @brief Get binary data as C string.
        @return Binary data as C string.
    */
    const char* c_str() const;

    /**
        @brief Method to clone/copy Binary_t
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    Value_t& clone(Pool_t &newPool) const override;

    /**
        @brief operator const std::string
    */
    operator const std::string& () const {return value;}

    /**
        @brief operator const std::wstring
    */
    operator std::wstring () const;
    ///static members
    static const String_t &FRPC_EMPTY;

    /**
        @brief Validates data, must be valid utf-8 byte sequence in range http://www.w3.org/TR/xml/#NT-Char
        @param pData - is a data pointer
        @param dataSize - is a size of data in bytes
    */
    static void validateBytes(const std::string::value_type *pData, std::string::size_type dataSize);

protected:
    /**
        @brief Default constructor is disabled
    */
    String_t();
    /**
        @brief Constructor  from pointer to data and data size
        @param pool  -  is a reference to Pool_t used for allocating
        @param pData - is a unsigned char pointer to data
        @param dataSize - is a size of data in bytes
    */
    String_t(std::string::value_type *pData, std::string::size_type dataSize);

    /**
        @brief Constructor  from pointer to data and data size
        @param pool  -  is a reference to Pool_t used for allocating
        @param pData - is a unsigned char pointer to data
        @param dataSize - is a size of data in bytes
    */
    String_t(const std::string::value_type *pData, std::string::size_type dataSize);
    /**
       @brief Constructor from std::string value
       @param pool  -  is a reference to Pool_t used for allocating
       @param value  - is a std::string of data
    */
    explicit String_t(const std::string &value);
    /**
       @brief Constructor from std::wstring value
       @param pool  -  is a reference to Pool_t used for allocating
       @param value  - is a std::wstring of data
    */
    explicit String_t(const std::wstring &value);

    std::string value;///internal storage
};
/**
    @brief Inline method

    Used to retype Value_t to String_t
    @return  If Value_t  can  retype to String_t return reference to String_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t

*/
inline FRPC_DLLEXPORT String_t& String(Value_t &value)
{
    auto *string_v = dynamic_cast<String_t*>(&value);

    if(!string_v)
        throw TypeError_t::format("Type is %s but not string",
                                  value.getTypeName());

    return *string_v;
}

/**
    @brief Inline method

    Used to retype Value_t to String_t
    @return  If Value_t  can  retype to String_t return reference to String_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t

*/
inline FRPC_DLLEXPORT const String_t& String(const Value_t &value)
{
    const auto *string_v = dynamic_cast<const String_t*>(&value);

    if(!string_v)
        throw TypeError_t::format("Type is %s but not string",
                                  value.getTypeName());

    return *string_v;
}

} // namespace FRPC

#endif
