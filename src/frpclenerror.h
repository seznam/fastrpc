/*
 * FILE          $Id: frpclenerror.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
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
#ifndef FRPCFRPCLENERROR_H
#define FRPCFRPCLENERROR_H

#include <frpcerror.h>

namespace FRPC
{

/**

@brief Error data(string) length 
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT LenError_t : public Error_t
{
public:
     /**
        @brief Constructor from format string and arguments
        @param format is const char* format string 
        @param ... is other arguments
    */
    
    LenError_t(const char *format, ...):Error_t()
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
    

    ~LenError_t();
private:
    LenError_t();
};

};

#endif
