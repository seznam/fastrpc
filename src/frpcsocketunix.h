/*
 * FILE          $Id: frpcsocketunix.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
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

#ifndef FRPCSOCKETUNIX_H_
#define FRPCSOCKETUNIX_H_

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ERRNO (errno)
#define STRERROR(e) strerror(e)

#endif // FRPCSOCKETUNIX_H_
