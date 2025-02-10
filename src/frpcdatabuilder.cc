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
 * FILE          $Id: frpcdatabuilder.cc,v 1.4 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#include <cstring>

#include "frpcdatabuilder.h"

namespace FRPC {

DataBuilder_t::DataBuilder_t() = default;

DataBuilder_t::~DataBuilder_t() = default;

void DataBuilder_t::buildFault(int errNumber, const char* errMsg) {
    auto size = static_cast<uint32_t>(strlen(errMsg));
    buildFault(errNumber, errMsg, size);
}
void DataBuilder_t::buildMethodCall(const char* methodName) {
    auto size = static_cast<uint32_t>(strlen(methodName));
    buildMethodCall(methodName,size);
}
void DataBuilder_t::buildString(const char* data) {

    auto size = static_cast<uint32_t>(strlen(data));
    buildString(data,size);
}
void DataBuilder_t::buildStructMember(const char *memberName) {

    auto size = static_cast<uint32_t>(strlen(memberName));
    buildStructMember(memberName,size);
}

DataBuilderWithNull_t::DataBuilderWithNull_t() = default;

DataBuilderWithNull_t::~DataBuilderWithNull_t() = default;

} // namespace FRPC
