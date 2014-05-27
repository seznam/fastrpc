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
 * FILE             $Id: frpcurlunmarshaller.h,v 1.1 2011-01-10 22:27:04 burlog Exp $
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

#ifndef FRPC_FRPCURLUNMARSHALLER_H
#define FRPC_FRPCURLUNMARSHALLER_H

#include <frpcunmarshaller.h>
#include <frpcdatabuilder.h>
#include "frpcinternals.h"
#include <frpc.h>
#include <vector>
#include <string>

namespace FRPC {

/**
 * @short Unmarshalling url encoded form data.
 */
class URLUnMarshaller_t : public UnMarshaller_t {
public:
    /** 
     * @short C'tor.
     */
    URLUnMarshaller_t(DataBuilder_t &dataBuilder,
                      const std::string &path = std::string("RPC2"));

    /** Unmarshalls serialized data.
     * @param data buffer with marshalled data.
     * @param size size of buffer.
     * @param type type of data to unmarshall (any, request, response, fault).
     */
    virtual void unMarshall(const char *data, unsigned int size, char type);

    /** 
     * @short Flush data.
     */
    virtual void finish();

private:
    DataBuilder_t &dataBuilder; //!< data builder
    std::string buffer;         //!< buffer for data
    std::string method;         //!< called method
};

} // namespace FRPC

#endif /* FRPC_FRPCURLUNMARSHALLER_H */

