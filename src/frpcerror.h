/*
 * FILE          $Id: frpcerror.h,v 1.2 2005-07-25 06:10:47 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2005
 * All Rights Reserved
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCERROR_H
#define FRPCFRPCERROR_H

#include <frpcplatform.h>

#include <string>
#include <stdio.h>
#include <stdio.h>
#include <stdarg.h>

namespace FRPC
{

/**
@brief API Error
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Error_t
{
public:

    /**
        @brief Constructor from format string and arguments
        @param format is const char* format string 
        @param ... is other arguments
    */
    Error_t(const char *format, ...)
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
    /**
        @brief Getting error message
        @return std::string is error message
    */
   const std::string message()
    {
        return msg;
    }
    
    const std::string message()  const
    {
        return msg;
    }
    /**
        @brief Default destructor
    */
    ~Error_t();
protected:
    /**
        @brief Default constructor
    */
    Error_t()
    {}
    std::string msg;
};

}

#endif
