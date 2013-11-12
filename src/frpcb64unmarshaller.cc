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
 * FILE             $Id: frpcb64unmarshaller.cc,v 1.1 2011-02-25 09:21:07 volca Exp $
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
 */

#include "frpcb64unmarshaller.h"
#include "frpcbase64.h"
#include <algorithm>

namespace FRPC {

bool isNonBase64Character(const char character) {
    if(isalnum(character)) {
        return false;
    } else if ( character == '+' || character == '\\' || character == '='){
        return false;
    }
    
    return true;
}

void filterNonBase64(const char* data, unsigned int size, std::string& output) {
    output.reserve(size);
    output.append(data, size);
    output.erase(std::remove_if(output.begin(), output.end(), isNonBase64Character), output.end());
}

Base64UnMarshaller_t::Base64UnMarshaller_t(DataBuilder_t &dataBuilder)
    : BinUnMarshaller_t(dataBuilder)
{
    m_residue.erase();
}

void Base64UnMarshaller_t::unMarshall(const char *data,
                                   unsigned int size,
                                   char type)
{
    std::string data_str; //> internal buffer
    filterNonBase64(data, size, data_str);

    std::string buffer;
    // reserve enough space = residue size + size of new incoming data
    buffer.reserve(m_residue.size() + data_str.size());

    // put residue (if there some) to the begining
    if (m_residue.size() > 0) {
        buffer.append(m_residue.c_str(), m_residue.size());
    }
    buffer.append(data_str);

    unsigned int residue_size = buffer.size() % 4; //> new residue size
    unsigned int to_decode = buffer.size() - residue_size; //> greatest size of data divisible by 4

    // handle new residue if there some
    if (residue_size > 0) {
        m_residue.reserve(residue_size);
        m_residue.erase();
        m_residue.append(buffer.data() + to_decode, residue_size);
    } else {
        m_residue.erase();
    }

    if (to_decode > 0) {
        //std::string decoded = Base64::decode(data, size);
        std::string decoded = Base64::decode(buffer.data(), to_decode);
        BinUnMarshaller_t::unMarshall(decoded.data(), decoded.size(), type);
    }
}

} // namespace FRPC

