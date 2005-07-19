/*
 * FILE          $Id: frpcresponseerror.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
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
#ifndef FRPCFRPCRESPONSEERROR_H
#define FRPCFRPCRESPONSEERROR_H

#include <frpcerror.h>

namespace FRPC
{

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT ResponseError_t : public Error_t
{
public:
    ResponseError_t();

    
    ~ResponseError_t();

};

};

#endif
