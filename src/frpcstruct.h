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
 * FILE          $Id: frpcstruct.h,v 1.8 2011-02-11 08:56:17 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCSTRUCT_H
#define FRPCFRPCSTRUCT_H

#include <frpcvalue.h>
#include "frpctypeerror.h"
#include <map>
#include <string>


namespace FRPC
{
class Pool_t;

/**
@brief Srtruct type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Struct_t : public Value_t
{
    friend class Pool_t;
public:
    /**
        @brief Struct_t iterator
    */
    typedef std::map<std::string,Value_t*>::iterator        iterator;
    /**
         @brief Struct_t const_iterator
    */
    typedef std::map<std::string,Value_t*>::const_iterator  const_iterator;
    /**
         @brief Struct_t size_type
    */
    typedef std::map<std::string,Value_t*>::size_type       size_type;
    /**
        @brief Struct_t key_type
    */
    typedef std::map<std::string,Value_t*>::key_type        key_type;
    /**
         @brief Struct_t value_type
    */
    typedef std::map<std::string,Value_t*>::value_type      value_type;
    /**
        @brief Struct_t pair
    */
    typedef std::pair<std::string, Value_t*>                 pair;

    // value types
    typedef const value_type &const_reference;
    typedef value_type &reference;


    enum{TYPE = 0x0A};
    /**
    `   @brief Default destructor
    */
    virtual ~Struct_t();
    /**
        @brief Method to clone/copy Struct_t
        @param newPool is pointer of Pool_t which is used for allocate objects
    */
    virtual Value_t& clone(Pool_t& newPool) const;
    /**
        @brief Getting type of value
        @return  @b unsigned @b short always
        @li @b Struct_t::TYPE - identificator of struct value
    */
    virtual unsigned short getType() const
    {
        return TYPE;
    }
    /**
        @brief Getting typename of value
        @return @b const @b char* always
        @li @b "Struct" - typename of Struct_t
    */
    virtual const char* getTypeName() const
    {
        return "struct";
    }
    /**
        @brief Getting info if Struct_t has key
        @param key searched key
        @return @b bool
        @li @b TRUE - if struct has key 'key'
        @li @b FALSE - if struct hasn't key 'key'
    */
    bool has_key(const key_type &key) const;
    /**
        @brief Inserting a new item with key
        @param key is reference to Struct_t::key_type
        @param value is reference to new Value_t
        @return  std::pair<iterator, bool> as std::map<>::insert(..)
    */
    std::pair<iterator, bool> insert(const key_type &key, const Value_t &value);
    /**
        @brief Inserting a new item with key
        @param value is new pair Struct_t::pair(std::string key, Value_t* value)
        @return  std::pair <iterator, bool> as std::map<>::insert(..)
    */
    std::pair<iterator, bool> insert(const pair &value);
    /**
        @brief Inserting a new item with key
        @param iterator where value should go.
        @param value is new pair Struct_t::pair(std::string key, Value_t* value)
        @return  std::pair <iterator, bool> as std::map<>::insert(..)
    */
    iterator insert(iterator iter, const pair &value);
    /**
        @brief Delete all items in Struct_t
    */
    void clear();
    /**
        @brief Checking if Struct_t is empty
        @return bool
        @li @b TRUE if the Struct_t is empty @n
        @li @b FALSE if the Struct_t isn't empty
    */
    bool empty() const;
    /**
        @brief getting number of items in Struct_t
        @return  Struct_t::size_type -  number of items in Struct_t
    */
    size_type size() const;

    /**
        @brief Getting iterator to first item
        @return Struct_t::iterator - position to first item
    */
    iterator begin();
    /**
        @brief Getting iterator to last item
        @return Struct_t::iterator - position to last item
    */
    iterator end();

    /**
        @brief Getting iterator to first item
        @return Struct_t::iterator - position to first item
    */
    const_iterator begin() const;
    /**
        @brief Getting iterator to last item
        @return Struct_t::iterator - position to last item
    */
    const_iterator end() const;

    /**
        @brief Insert Value_t to Struct_t with key
        @param value is is new pair Struct_t::pair(std::string key, Value_t* value)
        @return Struct_t& reference with apended value
    */
    Struct_t& append(const pair &value);
    /**
        @brief Insert Value_t to Struct_t with key
        @param key is reference to Struct_t::key_type
        @param value is reference to new Value_t
        @return Struct_t& reference with apended value
    */
    Struct_t& append(const key_type &key, const Value_t &value);
    /**
        @brief Get poiter to value or zero if not exists
        @param key is reference to Struct_t::key_type
        @return Value_t* pointer or zero
    */
    const Value_t* get(const key_type &key) const;
    Value_t* get(const key_type &key);
    /**
        @brief Get reference to value or defaultValue if not exists
        @param key is reference to Struct_t::key_type
        @param defaultValue reference to fefault value
        @return Value_t* pointer or zero
    */
    const Value_t& get(const key_type &key, const Value_t &defaultValue) const;
    Value_t& get(const key_type &key, Value_t &defaultValue);
    /**
        @brief operator []
        @return reference to Value_t or exeption KeyError_t if key isn't exist
    */
    Value_t& operator[] (const key_type &key);

    /**
        @brief operator []
        @return reference to Value_t or exeption KeyError_t if key isn't exist
    */
    const Value_t& operator[] (const key_type &key) const;

    /**
         @brief Returns iterator to value or end()
    */
    const_iterator find(const key_type &key) const {
         return structData.find(key);
    }

    /**
         @brief Returns iterator to value or end(). Mutable version
    */
    iterator find(const key_type &key) {
         return structData.find(key);
    }

    /**
        @brief Remove Value_t from Struct_t with key
        @param key is reference to Struct_t::key_type
        @return Struct_t& reference with apended value
    */
    size_type erase(const key_type &key);

    /// static member
    static const Struct_t &FRPC_EMPTY;

private:
    /**
        @brief Costructor empty Struct_t
        @param pool is a reference to Pool_t used for allocating
    */
    Struct_t();
    /**          1
        @brief Costructor of Struct_t with one item
        @param pool is a reference to Pool_t used for allocating
        @param key is reference to Struct_t::key_type
        @param value is reference to new Value_t
    */
    Struct_t(const std::string &key, const Value_t &value);
    /**
        @brief Costructor of Struct_t  with one item
        @param pool is a reference to Pool_t used for allocating
        @param value is new pair Struct_t::pair(std::string key, Value_t* value)
    */
    explicit Struct_t(const pair &value);



    std::map<std::string,Value_t*> structData; ///internal Struct_t data


};
/**
    @brief Inline method

    Used to retype Value_t to Struct_t
    @return  If Value_t  can  retype to Struct_t return reference to  Struct_t
    @n If Value_t can't retype to Struct_t: throw exception TypeError_t

*/
inline FRPC_DLLEXPORT Struct_t& Struct(Value_t &value)
{
    Struct_t *struct_v = dynamic_cast<Struct_t*>(&value);

    if(!struct_v)
        throw TypeError_t::format("Type is %s but not struct",
                                  value.getTypeName());
    return *struct_v;

}

/**
    @brief Inline method

    Used to retype Value_t to Struct_t
    @return  If Value_t  can  retype to Struct_t return reference to  Struct_t
    @n If Value_t can't retype to Struct_t: throw exception TypeError_t

*/
inline FRPC_DLLEXPORT const Struct_t& Struct(const Value_t &value)
{
    const Struct_t *struct_v = dynamic_cast<const Struct_t*>(&value);

    if(!struct_v)
        throw TypeError_t::format("Type is %s but not struct",
                                  value.getTypeName());
    return *struct_v;

}
}

#endif
