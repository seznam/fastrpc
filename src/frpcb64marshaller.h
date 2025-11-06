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
 * FILE          $Id: frpcb64marshaller.h,v 1.1 2011-02-25 09:21:07 volca Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Filip Volejnik <filip.volejnik@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#ifndef FRPCB64MARSHALLER_H
#define FRPCB64MARSHALLER_H

#include <memory>

#include <frpcbinmarshaller.h>
#include <frpcb64writer.h>

namespace FRPC {

class Base64Marshaller_t : public BinMarshaller_t {
public:
    Base64Marshaller_t(std::unique_ptr<Base64Writer_t> &b64writer,
                    const ProtocolVersion_t &protocolVersion)
        : BinMarshaller_t(*b64writer, protocolVersion), b64writer(std::move(b64writer))
    {}

protected:
    std::unique_ptr<Base64Writer_t> b64writer;
};

} // namespace FRPC

#endif // FRPCB64MARSHALLER_H
