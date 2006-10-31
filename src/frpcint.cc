/*
 * FILE          $Id: frpcint.cc,v 1.2 2006-10-31 11:19:42 vasek Exp $
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
#include "frpcint.h"

namespace FRPC
{


Value_t& Int_t::clone(Pool_t &newPool) const
{
 return newPool.Int(value);  
}
}
