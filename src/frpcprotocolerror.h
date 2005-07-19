/*
 * FILE          $Id: frpcprotocolerror.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
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
#ifndef FRPCFRPCPROTOCOLERROR_H
#define FRPCFRPCPROTOCOLERROR_H

#include <frpcplatform.h>

#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

namespace FRPC
{

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT ProtocolError_t
{
public:
    /**
    @brief Constructor from format string and arguments
    @param errNum is an error number
    @param format is const char* format string 
    @param ... is other arguments
     */
    ProtocolError_t(long errNum,const char *format, ...):errNum(errNum)
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
     * @brief constructor with only errnum 
     * used in child clases
     * */
    ProtocolError_t(long errNum):errNum(errNum)
    {}
    
    
    ~ProtocolError_t();

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
    * @brief Getting error message
    * @return long is error number
     * */
    long errorNum()
    {
        return errNum;
    }
    
    long errorNum()  const
    {
        return errNum;
    }
protected:
    ProtocolError_t();

    long errNum;
    std::string msg;

};

};

#endif
