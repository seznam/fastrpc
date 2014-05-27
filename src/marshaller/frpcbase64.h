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
 * FILE          $Id: frpcbase64.h,v 1.1 2011-02-25 09:21:07 volca Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCBASE64_H
#define FRPCBASE64_H

#include <frpcplatform.h>
#include <string>

namespace FRPC {

class Base64 {
public:
    /// Decodes a complete Base64 sequence. Any trailing bytes (0-3)
    /// that are not a part of a quad get thrown out
    static const std::string decode(const char *data, long len);

    Base64();

    /// Stateful decoder variant. Remembers the residue from last decode
    std::string process(const char *data, long len);

    /// complete == 0, otherwise there are some leftovers from last buffer.
    int remains() const { return i; }

private:
    // resets to prepare for the next quad.
    void reset();

    int i;
    unsigned char a[4];
    unsigned char b[4];
};

}; // namespace FRPC

#endif // FRPCBASE64_H
