/*
 * FILE          $Id: frpchttperror.h,v 1.1 2005-07-19 13:02:53 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2002
 * All Rights Reserved
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCHTTPERROR_H
#define FRPCFRPCHTTPERROR_H

#include <frpcplatform.h>

#include <frpcprotocolerror.h>

namespace FRPC
{

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT HTTPError_t : public ProtocolError_t
{
public:
    HTTPError_t(long errNum,const char *format, ...):ProtocolError_t(errNum)
    {
        
        // open variadic arguments
        va_list valist;
        va_start(valist, format);

        // format message
        char buf[1024];
        vsnprintf(buf, sizeof(buf), format, valist);

        // close variadic arguments
        va_end(valist);

        // return formated message
        msg = buf;
    }

    ~HTTPError_t();
private:

    HTTPError_t();
};

};

#endif
