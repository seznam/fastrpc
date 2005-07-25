/*
 * FILE          $Id: frpcsocketwin.h,v 1.2 2005-07-25 06:10:48 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Roman Marek <roman.marek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2005
 * All Rights Reserved
 *
 * HISTORY
 *       
 */

#ifndef FRPCSOCKETWIN_H_
#define FRPCSOCKETWIN_H_

#include <stdio.h>
#include <sys/timeb.h>

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
