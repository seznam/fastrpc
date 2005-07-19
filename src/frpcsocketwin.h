/*
 * FILE          $Id: frpcsocketwin.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
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

#include <sys/socket.h>

#define ERRNO (WSAGetLastError())
#define STRERROR(e) WSAGetLastErrorStr(e)

#endif // FRPCSOCKETWIN_H_
