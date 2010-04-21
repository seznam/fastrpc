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
 * FILE          $Id: frpcnull.h,v 1.1 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCNULL_H
#define FRPCNULL_H

#include <frpcvalue.h>
#include <typeinfo>

namespace FRPC
{
class Pool_t;

/**
@brief Null type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT  Null_t : public Value_t
{
    friend class Pool_t;
public:
    enum{TYPE = 0x0C};
    /**
        @brief Destructor
    */
    virtual ~Null_t();
    /**
        @brief Getting type of value
        @return @b unsigned @b short always
        @li @b Null_t::TYPE - identificator of null value
    */
    virtual unsigned short getType() const
    {
        return TYPE;
    }
    /**
        @brief Getting typename of value
        @return @b const @b char* always
        @li @b "Null" - typename of Null_t
    */
    virtual const char* getTypeName() const
    {
        return "null";
    }

    /**
        @brief Method to clone/copy Null_t
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    virtual Value_t& clone(Pool_t &newPool) const;

private :
    /**
        @brief Default constructor
    */
    Null_t() {}

    static Null_t staticValue;
};

inline FRPC_DLLEXPORT const bool isNull(const Value_t &value)
{
    if (dynamic_cast<const Null_t*>(&value)) {
        return true;
    } else {
        return false;
    }
}

};

#endif
