/*
 * FILE          $Id: frpcsocket.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
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

#ifndef FRPCSOCKET_H_
#define FRPCSOCKET_H_

#ifdef WIN32

#include <frpcsocketwin.h>

#else //WIN32

#include <frpcsocketunix.h>

#endif //WIN32

#endif // FRPCSOCKET_H_
