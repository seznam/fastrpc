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
 * FILE          $Id: frpcarray.h,v 1.7 2008-05-05 12:52:00 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2005
 * All Rights Reserved
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCARRAY_H
#define FRPCFRPCARRAY_H

#include <frpcvalue.h>
#include <frpctypeerror.h>
#include <vector>

namespace FRPC {
class Pool_t;

/**
@brief Array type can storage any type of Value_t
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Array_t : public Value_t {
    friend class Pool_t;
public:
    enum{TYPE = TYPE_ARRAY};

    /**
       @brief Array_t iterator
    */
    using iterator = std::vector<Value_t *>::iterator;
    /**
       @brief Array_t const_iterator
    */
    using const_iterator = std::vector<Value_t *>::const_iterator;
    /**
      @brief Array_t size_type
    */
    using size_type = std::vector<Value_t *>::size_type;

    // value types
    using reference = Value_t &;
    using const_reference = const Value_t &;
    using value_type = Value_t;

    /**
        @brief  Default destructor
    */
    ~Array_t() override;

    /**
       @brief Method to clone/copy Array_t
       @param newPool is reference of Pool_t which is used for allocate objects
     * @return reference to new Array_t as Value_t
    */

    Value_t& clone(Pool_t &newPool) const override;
    /**
        @brief Getting type of value
        @return  @b unsigned @b short always
        @li @b Array_t::TYPE - identificator of array value
    */
    TypeTag_t getType() const override {return TYPE;}
    /**
        @brief Getting typename of value
        @return @b const @b char * always
        @li @b "Array" - typename of Array_t
    */
    const char* getTypeName() const override {return "array";}

    /**
        @brief operator []
        @return reference to Value_t or exception IndexError_t if index is out of range
    */
    Value_t &operator[] (size_type index) override;

    /**
        @brief operator []
        @return reference to Value_t or exception IndexError_t if index is out of range
    */
    const Value_t &operator[] (size_type index) const override;

    /**
        @brief getting iterator to first item
        @return Array_t::iterator - position to first item
    */
    iterator begin();
    /**
        @brief getting iterator to last item
        @return Array_t::iterator - position to last item
    */
    iterator end();

    /**
        @brief getting iterator to first item
        @return Array_t::iterator - position to first item
    */
    const_iterator begin() const;
    /**
        @brief getting iterator to last item
        @return Array_t::iterator - position to last item
    */
    const_iterator end() const;

    /**
        @brief getting number of items in Array_t
        @return  Array_t::size_type -  number of items in Array_t
    */
    size_type size() const;
    /**
        @brief checking if Array_t is empty
        @return bool
        @li @b TRUE if the Array_t is empty @n
        @li @b FALSE if the Array_t isn't empty

    */
    bool empty() const;
    /**
        @brief delete all items in Array_t
    */
    void clear();

    /**
        @brief reserve memory for Array_t
        @param size is new size
    */
    void reserve(size_type size);

    /**
       @brief return cacacity of Array_t
       @return size_type capacity of Array_t
    */

    size_type capacity();

    /**
        @brief append Value_t to end of Array_t
        @param value is new Value_t
    */
    void push_back(const Value_t &value);

    /**
        @brief append Value_t to end of Array_t
        @param value is new Value_t
        @return Array_t& reference with apended value
    */
    Array_t& append(const Value_t &value);
    /**
        @brief check array for items
        @param items is std::string contains  signatures as
        @li @b  s - string
        @li @b  i - int
        @li @b  b - bool
        @li @b  d - double
        @li @b  B - binary
        @li @b  D - dateTime
        @li @b  A - array
        @li @b  S - struct

        If items not correct function throw TypeError_t.
      @n This method is using for checking input parameters in methods.
    */
    void checkItems(const std::string &items) const;

    ///static member
    static const Array_t &FRPC_EMPTY;

protected:
    using Value_t::operator[];

    /**
        @brief Costructor empty Array_t
    */
    Array_t();

    /**
        @brief Costructor Array_t with reserved size
        @param size to reserve
    */
    explicit Array_t(size_type size);

    /**
        @brief Costructor Array_t with one Value_t item
        @param item  is a new item
    */
    explicit Array_t(const Value_t &item);

    std::vector<Value_t*> arrayData;///Internal array data

};
/**
    @brief Inline method

    Used to retype Value_t to Array_t
    @param value is reference to Value_t
    @return  If Value_t  can  retype to Array_t return reference to Array_t
    @n If Value_t can't retype to Array_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT Array_t& Array(Value_t &value) {
    auto *array = dynamic_cast<Array_t*>(&value);

    if(!array)
        throw TypeError_t::format("Type is %s but not array",
                                  value.getTypeName());
    return *array;
}

/**
    @brief Inline method

    Used to retype Value_t to Array_t
    @param value is reference to Value_t
    @return  If Value_t  can  retype to Array_t return reference to Array_t
    @n If Value_t can't retype to Array_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT const Array_t& Array(const Value_t &value) {
    const auto *array = dynamic_cast<const Array_t*>(&value);

    if(!array)
        throw TypeError_t::format("Type is %s but not array",
                                  value.getTypeName());
    return *array;
}

} // namespace FRPC

#endif
