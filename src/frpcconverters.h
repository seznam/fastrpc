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
 * FILE             $Id: frpcconverters.h,v 1.5 2008-11-19 08:37:34 burlog Exp $
 *
 * DESCRIPTION      Templated converters.
 *
 * PROJECT          FastRPC library.
 *
 * AUTHOR           Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2008
 * All Rights Reserved
 *
 * HISTORY
 *       2008-05-02 (bukovsky)
 *                  First draft.
 */

#ifndef FRPC_FRPCCONVERTERS_H
#define FRPC_FRPCCONVERTERS_H

#include <iterator>
#include <algorithm>

#include <frpcpool.h>
#include <frpcbool.h>
#include <frpcint.h>
#include <frpcdouble.h>
#include <frpcstring.h>
#include <frpcbinary.h>
#include <frpcdatetime.h>
#include <frpcstruct.h>
#include <frpcarray.h>

namespace FRPC {

/**
 * @short Base converter class.
 */
template <class Type_t, class FRPCType_t>
class base_cnvt {
public:
    /// Conversion fuction type.
    using ConverterCall_t = FRPCType_t &(Pool_t::*)(const Type_t &);

    /**
     * @short Create new converter.
     * @param pool FastRPC pool.
     * @param call conversion function.
     */
    explicit base_cnvt(Pool_t &pool, ConverterCall_t call)
        : pool(&pool), call(call)
    {}

    /**
     * @short Convert value to FastRPC value.
     * @return FastRPC value.
     */
    Value_t &operator()(const Type_t &value) const {
        return (*pool.*call)(value);
    }

    /**
     * @short Return FastRPC pool.
     * @return FastRPC pool.
     */
    Pool_t *allocator() const {return pool;}

private:
    Pool_t *pool;          //!< FastRPC pool.
    ConverterCall_t call;  //!< conversion function
};

/**
 * @short Convert int to FastRPC value.
 */
template <class Type_t>
class base_int_cnvt: public base_cnvt<Type_t, Int_t> {
public:
    /**
     * @short Create new converter.
     * @param pool FastRPC pool.
     */
    explicit base_int_cnvt(Pool_t &pool)
        : base_cnvt<Type_t, Int_t>(pool, &Pool_t::Int)
    {}
};

/**
 * @short Convert double to FastRPC value.
 */
template <class Type_t>
class base_double_cnvt: public base_cnvt<Type_t, Double_t> {
public:
    /**
     * @short Create new converter.
     * @param pool FastRPC pool.
     */
    explicit base_double_cnvt(Pool_t &pool)
        : base_cnvt<Type_t, Double_t>(pool, &Pool_t::Double)
    {}
};

/**
 * @short Convert string to FastRPC value.
 */
template <class Type_t>
class base_string_cnvt: public base_cnvt<Type_t, String_t> {
public:
    /**
     * @short Create new converter.
     * @param pool FastRPC pool.
     */
    explicit base_string_cnvt(Pool_t &pool)
        : base_cnvt<Type_t, String_t>(pool, &Pool_t::String)
    {}
};

/**
 * @short Convert bool to FastRPC value.
 */
template <class Type_t>
class base_bool_cnvt: public base_cnvt<Type_t, Bool_t> {
public:
    /**
     * @short Create new converter.
     * @param pool FastRPC pool.
     */
    explicit base_bool_cnvt(Pool_t &pool)
        : base_cnvt<Type_t, Bool_t>(pool, &Pool_t::Bool)
    {}
};

/**
 * @short Convert binary to FastRPC value.
 */
template <class Type_t>
class base_binary_cnvt: public base_cnvt<Type_t, Binary_t> {
public:
    /**
     * @short Create new converter.
     * @param pool FastRPC pool.
     */
    explicit base_binary_cnvt(Pool_t &pool)
        : base_cnvt<Type_t, Binary_t>(pool, &Pool_t::Binary)
    {}
};

/**
 * @short Convert datetime to FastRPC value.
 */
template <class Type_t>
class base_datetime_cnvt: public base_cnvt<Type_t, DateTime_t> {
public:
    /**
     * @short Create new converter.
     * @param pool FastRPC pool.
     */
    explicit base_datetime_cnvt(Pool_t &pool)
        : base_cnvt<Type_t, DateTime_t>(pool, &Pool_t::DateTime)
    {}
};

/**
 * @short Convert localtime to FastRPC value.
 */
template <class Type_t>
class base_localtime_cnvt: public base_cnvt<Type_t, DateTime_t> {
public:
    /**
     * @short Create new converter.
     * @param pool FastRPC pool.
     */
    explicit base_localtime_cnvt(Pool_t &pool)
        : base_cnvt<Type_t, DateTime_t>(pool, &Pool_t::LocalTime)
    {}
};

/**
 * @short Convert utctime to FastRPC value.
 */
template <class Type_t>
class base_utctime_cnvt: public base_cnvt<Type_t, DateTime_t> {
public:
    /**
     * @short Create new converter.
     * @param pool FastRPC pool.
     */
    explicit base_utctime_cnvt(Pool_t &pool)
        : base_cnvt<Type_t, DateTime_t>(pool, &Pool_t::UTCTime)
    {}
};

/**
 * @short Convert array - each iterable object to FastRPC value.
 */
template <class Converter_t>
class base_array_cnvt {
public:
    /**
     * @short Create new converter.
     * @param converter value converter.
     */
    explicit base_array_cnvt(const Converter_t &converter)
        : converter(converter)
    {}

    /**
     * @short Convert value - each iterable object to FastRPC value.
     * @return
     */
    template <class ArrayType_t>
    Value_t &operator()(const ArrayType_t &value) const {
        return to_array(value.begin(), value.end(), converter);
    }

    /**
     * @short Return FastRPC pool.
     * @return FastRPC pool.
     */
    Pool_t *allocator() const {
        return const_cast<Pool_t *>(converter.allocator());
    }

public:
    const Converter_t &converter; //!< value converter
};

/**
 * @short Shortcut for base_array_cnvt.
 * @param cnvt array value convertor.
 * @return new base_array_cnvt.
 */
template <class BaseConv_t>
base_array_cnvt<BaseConv_t> array_cnvt(const BaseConv_t &cnvt) {
    return base_array_cnvt<BaseConv_t>(cnvt);
}

/**
 * @short Convert pair - member of associative containers to FastRPC value.
 */
template <class Converter_t>
class base_pair_cnvt {
public:
    /**
     * @short Create new converter.
     * @param converter value converter.
     */
    explicit base_pair_cnvt(const Converter_t &converter)
        : converter(converter)
    {}

    /**
     * @short Convert pair - member of associative containers to FastRPC value.
     * @return FastRPC value.
     */
    template <class ValueType_t>
    Struct_t::pair operator() (const ValueType_t &pair) const {
        return Struct_t::pair(pair.first, &converter(pair.second));
    }

    /**
     * @short Return FastRPC pool.
     * @return FastRPC pool.
     */
    Pool_t *allocator() const {
        return const_cast<Pool_t *>(converter.allocator());
    }

public:
    const Converter_t &converter; //!< value converter
};

/**
 * @short Shortcut for base_pair_cnvt.
 * @param cnvt pair value convertor.
 * @return new base_pair_cnvt.
 */
template <class BaseConv_t>
base_pair_cnvt<BaseConv_t> pair_cnvt(const BaseConv_t &cnvt) {
    return base_pair_cnvt<BaseConv_t>(cnvt);
}

/**
 * @short Base class for aplication struct a class converters.
 */
class base_struct_cnvt {
public:
    /**
     * @short Create new converter.
     * @param converter value converter.
     */
    explicit base_struct_cnvt(Pool_t &pool)
        : pool(pool)
    {}

    /**
     * @short Return FastRPC pool.
     * @return FastRPC pool.
     */
    Pool_t *allocator() const {return &pool;}

protected:
    Pool_t &pool; //!< FastRPC pool
};

/**
 * @short Convert elements in interval to array of FastRPC values.
 * @param begin iterator at first element.
 * @param end iterator after last element.
 * @param converter value converter.
 * @return FastRPC array.
 */
template <class ForwardIterator_t, class Converter_t>
Value_t &to_array(const ForwardIterator_t &begin,
                         const ForwardIterator_t &end,
                         const Converter_t &converter) {

    Array_t &array = converter.allocator()->Array();
    std::transform(begin, end, std::back_inserter(array), converter);
    return array;
}

/**
 * @short Convert elements of iterable container to array of FastRPC values.
 * @param container some value container like std::vector.
 * @param converter value converter.
 * @return FastRPC array.
 */
template <class Container_t, class Converter_t>
Value_t &to_array(const Container_t &container,
                         const Converter_t &converter) {

    return to_array(container.begin(), container.end(), converter);
}

/**
 * @short Convert elements in interval to struct of FastRPC values.
 * @param begin iterator at first element.
 * @param end iterator after last element.
 * @param converter value converter.
 * @return FastRPC array.
 */
template <class ForwardIterator_t, class Converter_t>
Value_t &to_struct(const ForwardIterator_t &begin,
                   const ForwardIterator_t &end,
                   const Converter_t &converter) {

    Struct_t &structure = converter.allocator()->Struct();
    std::transform(begin, end,
                   std::inserter(structure, structure.begin()),
                   pair_cnvt(converter));
    return structure;
}

/**
 * @short Convert elements in interval to struct of FastRPC values.
 * @param container some value container like std::map.
 * @param converter value converter.
 * @return FastRPC array.
 */
template <class Container_t, class Converter_t>
Value_t &to_struct(const Container_t &container,
                   const Converter_t &converter) {
    return to_struct(container.begin(), container.end(), converter);
}


/// int64_t converter typedef
using int_cnvt = base_int_cnvt<int64_t>;

/// double converter typedef
using double_cnvt = base_double_cnvt<double>;

/// wide string converter typedef
using wstring_cnvt = base_string_cnvt<std::wstring>;
/// string converter typedef
using string_cnvt = base_string_cnvt<std::string>;

/// bool converter typedef
using bool_cnvt = base_bool_cnvt<bool>;

/// string to binary converter typedef
using binary_cnvt = base_binary_cnvt<std::string>;

/// string to datetime convert typedef
using datetime_cnvt = base_datetime_cnvt<std::string>;
/// local time_t to datetime converter typedef
using localtime_cnvt = base_localtime_cnvt<time_t>;
/// utc time_t to datetime converter typedef
using utctime_cnvt = base_utctime_cnvt<time_t>;

} // namespace FRPC

#endif /* FRPC_FRPCCONVERTERS_H */
