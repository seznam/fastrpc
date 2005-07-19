/*
 * FILE          $Id: frpcfault.h,v 1.1 2005-07-19 13:02:53 vasek Exp $
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
#ifndef FRPCFRPCFAULT_H
#define FRPCFRPCFAULT_H

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
class FRPC_DLLEXPORT Fault_t
{
public:
    Fault_t(long errNum, const std::string &errMsg):errNum(errNum),
            errMsg(errMsg)
    {}
    
     Fault_t(long errNum, const char *format, ...):errNum(errNum)
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
        errMsg = buf;

     
     }

    ~Fault_t();
    long errorNum()
    {
        return errNum;
    }

    const std::string& message()
    {
        return errMsg;
    }
    long errorNum()   const 
    {
        return errNum;
    }

    const std::string& message() const 
    {
        return errMsg;
    }
private:
    Fault_t();
    long errNum;
    std::string errMsg;


};

};

#endif
