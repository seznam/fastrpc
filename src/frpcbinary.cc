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
 * FILE          $Id: frpcbinary.cc,v 1.4 2007-05-24 11:28:28 mirecta Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#include "frpcbinary.h"
#include "frpcpool.h"

namespace FRPC
{




Binary_t::~Binary_t()
{}

Binary_t::Binary_t(std::string::value_type *pData, std::string::size_type dataSize)
        :value(pData,dataSize)
{}

Binary_t::Binary_t(const std::string &value)
        :value(value)
{}

std::string::size_type Binary_t::size() const
{
    return value.size();
}


const std::string::value_type*  Binary_t::data() const
{
    return  value.data();
}


std::string Binary_t::getString() const
{
    return value;
}

const std::string& Binary_t::getValue() const
{
    return value;
}


Value_t& Binary_t::clone(Pool_t &newPool) const
{
    return newPool.Binary(value);
}
}
