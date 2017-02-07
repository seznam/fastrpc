/*
 * FastRPC -- Fast RPC library compatible with XML-RPC
 * Copyright (C) 2005-8  Seznam.cz, a.s.
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
 * FILE             $Id: frpccompare.h,v 1.4 2011-02-16 09:23:09 burlog Exp $
 *
 * DESCRIPTION      Comparing generic values.
 *
 * PROJECT          FastRPC library.
 *
 * AUTHOR           Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2010
 * All Rights Reserved
 *
 * HISTORY
 *       2010-09-28 (bukovsky)
 *                  First draft.
 */

#ifndef FRPC_FRPCCOMPARE_H
#define FRPC_FRPCCOMPARE_H

#include <frpcbool.h>
#include <frpcint.h>
#include <frpcdouble.h>
#include <frpcstring.h>
#include <frpcbinary.h>
#include <frpcdatetime.h>
#include <frpcstruct.h>
#include <frpcarray.h>
#include <frpcnull.h>

namespace FRPC {

/**
 * @short Compares two FastRPC values and returns zero if it is equals. 1 if
 * lhs is greater and -1 if lhs is less.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return 0, -1, 1 if equals, is lhs is greater or lhs is less.
 */
int compare(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs);

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if values is same.
 */
bool operator==(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs);

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if values is not same.
 */
bool operator!=(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs);

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is less then rhs.
 */
bool operator<(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs);

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is greater then rhs.
 */
bool operator>(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs);

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is less or equal then rhs.
 */
bool operator<=(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs);

/**
 * @short Compares two FastRPC values.
 * @param lhs left operand.
 * @param rhs right operand.
 * @return true if lhs is greater or equal then rhs.
 */
bool operator>=(const FRPC::Value_t &lhs, const FRPC::Value_t &rhs);

} // namespace FRPC

#endif /* FRPC_FRPCCOMPARE_H */
