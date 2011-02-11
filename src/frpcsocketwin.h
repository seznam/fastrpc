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
 * FILE          $Id: frpcsocketwin.h,v 1.4 2011-02-11 08:56:17 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Roman Marek <roman.marek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#ifndef FRPCSOCKETWIN_H_
#define FRPCSOCKETWIN_H_

#include <stdio.h>
#include <sys/timeb.h>

#define STRERROR_PRE() {}
#define ERRNO (WSAGetLastError())
#define STRERROR(e) WSAGetLastErrorStr(e)

#include <Winsock2.h>
#include <Winsock.h>

#define close(x) closesocket(x)

#define MSG_NOSIGNAL 0L

#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define EWOULDBLOCK				WSAEWOULDBLOCK

typedef int socklen_t;

char* WSAGetLastErrorStr(const int error);

#define vsnprintf _vsnprintf

#endif // FRPCSOCKETWIN_H_
