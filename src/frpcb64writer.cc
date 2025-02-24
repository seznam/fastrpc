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
 * FILE          $Id: frpcb64writer.cc,v 1.1 2011-02-25 09:21:07 volca Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Filip Volejnik <filip.volejnik@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include <frpcb64writer.h>

namespace FRPC {
namespace {
const char B64_ALPHABET[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
};
}; // namespace

const Base64Writer_t::States_t Base64Writer_t::State_t::STATE_NEXT[3] = {
    Base64Writer_t::STATE_SECOND,
    Base64Writer_t::STATE_THIRD,
    Base64Writer_t::STATE_FIRST
};

Base64Writer_t::~Base64Writer_t() {
    //try to flush, ignore exceptions
    try {
        flush();
    } catch (...)
    {};
}

void Base64Writer_t::write(const char *data, unsigned int size) {
    const unsigned char *ud = reinterpret_cast<const unsigned char *>(data);

    while (size) {
        switch (state.state) {
        case STATE_FIRST:
            // fresh byte start
            writer.write(&B64_ALPHABET[*ud >> 2], 1);
            state.next((*ud & 0x03) << 4, 1);

            ++ud;
            if (!--size)
                return;

            // FALL THROUGH
        case STATE_SECOND:
            writer.write(&B64_ALPHABET[state.prev | ((*ud & 0x0f0) >> 4)], 1);
            state.next((*ud & 0x0f) << 2, 1);

            ++ud;
            if (!--size)
                return;

            // FALL THROUGH
        case STATE_THIRD:
            writer.write(&B64_ALPHABET[state.prev | ((*ud & 0x0c0) >> 6)], 1);
            writer.write(&B64_ALPHABET[*ud & 0x03f], 1);
            state.next(0, 2);

            ++ud;
            if (!--size)
                return;
        }
    }
}

void Base64Writer_t::flush() {
    // encode the data tail
    switch (state.state) {
    case STATE_SECOND:
        writer.write(&B64_ALPHABET[state.prev], 1);
        writer.write("==", 2);
        break;

    case STATE_THIRD:
        writer.write(&B64_ALPHABET[state.prev], 1);
        writer.write("=", 1);
        break;

    default:
            break;
    }

    state.reset();
    writer.flush();
}

}; // namespace FRPC
