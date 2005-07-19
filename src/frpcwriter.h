/*
 * FILE          $Id: frpcwriter.h,v 1.1 2005-07-19 13:02:55 vasek Exp $
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
#ifndef FRPCFRPCWRITER_H
#define FRPCFRPCWRITER_H
//#include <string>
#include <frpcplatform.h>


namespace FRPC
{

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Writer_t
{
public:
    Writer_t();

    virtual ~Writer_t();
    virtual void write(const char *data, long size ) = 0;
    virtual void flush() = 0;

    
    
};

};

#endif
