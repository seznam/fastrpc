/*
 * FILE          $Id: frpcresponseerror.cc,v 1.1 2005-07-19 13:02:54 vasek Exp $
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
#include "frpcresponseerror.h"

namespace FRPC
{

ResponseError_t::ResponseError_t()
    : Error_t("Response error")
{
}


ResponseError_t::~ResponseError_t()
{}


}
;
