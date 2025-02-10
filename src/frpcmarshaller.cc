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
 * FILE          $Id: frpcmarshaller.cc,v 1.6 2011-02-25 09:21:07 volca Exp $
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

#include "frpcmarshaller.h"
#include "frpcwriter.h"
#include "frpcbinmarshaller.h"
#include "frpcxmlmarshaller.h"
#include "frpcjsonmarshaller.h"
#include "frpcb64marshaller.h"
#include "frpcerror.h"

namespace FRPC {

Marshaller_t::Marshaller_t() = default;

Marshaller_t::~Marshaller_t() = default;

Marshaller_t* Marshaller_t::create(unsigned int contentType, Writer_t& writer,
                                  const ProtocolVersion_t &protocolVersion) {
    Marshaller_t *marshaller = nullptr;

    switch (contentType) {
    case BINARY_RPC:
        marshaller = new BinMarshaller_t(writer,protocolVersion);
        break;

    case XML_RPC:
        marshaller = new XmlMarshaller_t(writer,protocolVersion);
        break;

    case JSON:
        marshaller = new JSONMarshaller_t(writer,protocolVersion);
        break;

    case BASE64_RPC: {
            std::unique_ptr<Base64Writer_t> b64writer(new Base64Writer_t(writer));
            marshaller = new Base64Marshaller_t(b64writer, protocolVersion);
            break;
        }

    default:
        throw Error_t("This marshaller not exists");
        break;
    }

    return marshaller;
}

void Marshaller_t::packStructMember(const char* memberName) {
    auto size = static_cast<uint32_t>(strlen(memberName));
    packStructMember(memberName, size);
}

void  Marshaller_t::packString(const char* value){
    auto size = static_cast<uint32_t>(strlen(value));
    packString(value,size);
}

void Marshaller_t::packFault(int errNumber, const char* errMsg){
    auto size = static_cast<uint32_t>(strlen(errMsg));
    packFault(errNumber,errMsg,size);
}

void Marshaller_t::packMethodCall(const char* methodName){
    auto size = static_cast<uint32_t>(strlen(methodName));
    packMethodCall(methodName,size);
}

} // namespace FRPC
