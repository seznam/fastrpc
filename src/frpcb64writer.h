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
 * FILE          $Id: frpcb64writer.h,v 1.1 2011-02-25 09:21:07 volca Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Filip Volejnik <filip.volejnik@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#ifndef FRPCB64WRITER_H
#define FRPCB64WRITER_H

#include <frpcwriter.h>

namespace FRPC {

class Base64Writer_t: public Writer_t {
public:
    Base64Writer_t(Writer_t &writer): writer(writer), state() {}

    ~Base64Writer_t() override;

    /// Encoder's state (current byte position in tripplet)
    enum States_t { STATE_FIRST = 0x0, STATE_SECOND = 0x1, STATE_THIRD = 0x2 };

    /// Complex encoder's state
    struct State_t {
        State_t() : state(STATE_FIRST), lineLen(0), prev(0x0) {}

        void next(unsigned char pr, size_t count) {
            lineLen += count;
            prev = pr;
            state = STATE_NEXT[state];
        }

        void reset() {
            state = STATE_FIRST;
            prev = 0;
            lineLen = 0;
        }

        States_t state;
        size_t lineLen;
        unsigned char prev;

        static const States_t STATE_NEXT[3];
    };

    void write(const char *data, unsigned int size) override;
    void flush() override;

private:
    Writer_t &writer;
    State_t state;
};

} // namespace FRPC

#endif // FRPCB64WRITER_H
