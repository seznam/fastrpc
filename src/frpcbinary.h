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
 * FILE          $Id: frpcbinary.h,v 1.4 2007-04-02 15:28:21 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#ifndef FRPCBINARY_H
#define FRPCBINARY_H

#include <string>
#include <frpcvalue.h>


namespace FRPC
{
class Pool_t;

/**
@brief Binary type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT  Binary_t : public Value_t
{
    friend class Pool_t;
public:
    enum{ TYPE = 0x06 };

    virtual ~Binary_t();

    /**
        @brief Getting type of value
        @return  @b unsigned @b short always 
        @li @b Binary_t::TYPE - identificator of binary value
    */
    virtual unsigned short getType() const
    {
        return TYPE;
    }

    /**
        @brief Getting typename of value
        @return @b const @b char* always
        @li @b "Binary" - typename of Binary_t
    */
    virtual const char* getTypeName() const
    {
        return "binary";
    }
    
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
    std::string getString() const;

    /**
        @brief Method to clone/copy Binary_t 
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    virtual Value_t& clone(Pool_t &newPool) const;

    /**
        @brief operator const std::string
    */
    inline operator const std::string& () const
    {
        return value;
    }

private:
    /**
        @brief Default constructor is disabled
    */
    Binary_t();
    /**
        @brief Constructor  from pointer to data and data size
        @param pool  -  is a reference to Pool_t used for allocating
        @param pData - is a unsigned char pointer to data 
        @param dataSize - is a size of data in bytes
    */
    Binary_t(Pool_t &pool, std::string::value_type *pData, std::string::size_type dataSize);
    /**
       @brief Constructor from std::string value
       @param pool  -  is a reference to Pool_t used for allocating
       @param value  - is a std::string of data
    */
    Binary_t(Pool_t &pool, const std::string &value);

    std::string value;///internal storage

};
/**
    @brief Inline method
    
    Used to retype Value_t to Binary_t 
    @return  If Value_t  can  retype to Binary_t return reference to Binary_t
    @n If Value_t can't retype to Binary_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT Binary_t& Binary(Value_t &value)
{
    Binary_t *binary = dynamic_cast<Binary_t*>(&value);

    if(!binary)
        throw TypeError_t("Type is %s but not binary",value.getTypeName());
    return *binary;
}

/**
    @brief Inline method
    
    Used to retype Value_t to Binary_t 
    @return  If Value_t  can  retype to Binary_t return reference to Binary_t
    @n If Value_t can't retype to Binary_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT const Binary_t& Binary(const Value_t &value)
{
    const Binary_t *binary = dynamic_cast<const Binary_t*>(&value);

    if(!binary)
        throw TypeError_t("Type is %s but not binary",value.getTypeName());
    return *binary;
}

}

#endif
