 /*
 * FILE             $Id: nonglibc.h,v 1.1 2010-02-23 12:21:06 burlog Exp $
 *
 * DESCRIPTION      SUpplyt GLIBC support to non-GLIBC environment.
 *
 * AUTHOR           Vasek Blazek <blazek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2007
 * All Rights Reserved
 *
 * HISTORY
 *          2007-05-31
 *                  First draft.
 */


#ifndef NONGLIBC_H_
#define NONGLIBC_H_

// use the TEMP_FAILURE_RETRY function
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <sys/socket.h>

#ifndef MSG_NOSIGNAL
#    define MSG_NOSIGNAL 0
#endif

#ifndef TEMP_FAILURE_RETRY

// not a glibc :(

#    ifdef __GNUC__

// GNU compiler => can use extension => just get definition of
// TEMP_FAILURE_RETRY from glibc

/* Evaluate EXPRESSION, and repeat as long as it returns -1 with `errno'
   set to EINTR.  */

#        define TEMP_FAILURE_RETRY(expression) \
    (__extension__                                                \
     ({ long int __result;                                        \
         do __result = (long int) (expression);				      \
         while (__result == -1L && errno == EINTR);			      \
         __result; }))
#    else
#        warning TEMP_FAILURE_RETRY is noop!
#        define TEMP_FAILURE_RETRY(CALL) (CALL)
#    endif
#endif

#endif // NONGLIBC_H_
