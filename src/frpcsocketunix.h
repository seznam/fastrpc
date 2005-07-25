/*
 * FILE          $Id: frpcsocketunix.h,v 1.2 2005-07-25 06:10:48 vasek Exp $
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
#include <unistd.h>
#include <sys/time.h>

#define ERRNO (errno)
#define STRERROR(e) strerror(e)

#endif // FRPCSOCKETUNIX_H_
