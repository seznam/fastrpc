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
 * FILE             $Id: frpcb64unmarshaller.h,v 1.1 2011-02-25 09:21:07 volca Exp $
 *
 * DESCRIPTION      Url encoded unmarshaller.
 *
 * PROJECT          FastRPC library.
 *
 * AUTHOR           Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2011
 * All Rights Reserved
 *
 * HISTORY
 *       2011-01-06 (bukovsky)
 *                  First draft.
 *
 */

#ifndef FRPC_FRPCB64UNMARSHALLER_H
#define FRPC_FRPCB64UNMARSHALLER_H

#include <frpcbinunmarshaller.h>
#include <frpcdatabuilder.h>
#include "frpcinternals.h"
#include <frpc.h>
#include <vector>
#include <string>

namespace FRPC {

/**
 * @short Unmarshalling url encoded form data.
 */
class Base64UnMarshaller_t : public BinUnMarshaller_t {
public:
    /**
     * @short C'tor.
     */
    Base64UnMarshaller_t(DataBuilder_t &dataBuilder);

    /** Unmarshalls serialized data.
     * @param data buffer with marshalled data.
     * @param size size of buffer.
     * @param type type of data to unmarshall (any, request, response, fault).
     */
    virtual void unMarshall(const char *data, unsigned int size, char type);
};

} // namespace FRPC

#endif /* FRPC_FRPCURLUNMARSHALLER_H */

