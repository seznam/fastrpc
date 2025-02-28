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
 * FILE          $Id: frpcvalue.cc,v 1.3 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#include "frpcvalue.h"
#include "frpcnull.h"

namespace FRPC
{

Value_t& Value_t::operator[](std::vector<Value_t*>::size_type) {
    throw TypeError_t::format("Type of object is %s but array operator used.",
                              this->getTypeName());
}

const Value_t& Value_t::operator[](
    std::vector<Value_t*>::size_type) const
{
    throw TypeError_t::format("Type of object is %s but array operator used.",
                              this->getTypeName());
}

Value_t& Value_t::operator[](
    const std::map<std::string, Value_t*>::key_type&)
{
    throw TypeError_t::format("Type of object is %s but struct operator used.",
                              this->getTypeName());
}

const Value_t& Value_t::operator[](
    const std::map<std::string, Value_t*>::key_type&) const
{
    throw TypeError_t::format("Type of object is %s but struct operator used.",
                              this->getTypeName());
}

Value_t::~Value_t()
{}

bool Value_t::isNull() const
{
    return FRPC::isNull(*this);
}

}
