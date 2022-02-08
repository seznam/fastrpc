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
 * FILE          $Id: frpcstring.cc,v 1.5 2007-05-24 11:28:29 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#ifdef WIN32
#include <windows.h>
#include "frpcerror.h"
#endif //WIN32
#include "frpcpool.h"
#include "frpcconfig.h"
#include "frpcstring.h"
#include "frpcstring_view.h"

namespace FRPC {

Value_t &StringView_t::clone(Pool_t &newPool) const {
    return newPool.StringView(ptr, length);
}

void StringView_t::validateBytes(const char *ptr, std::size_t length) {
    String_t::validateBytes(ptr, length);
}

} // namespace FPRC
