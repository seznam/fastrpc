/*
 * FILE          $Id: frpcbinary.cc,v 1.2 2006-02-09 16:00:26 vasek Exp $
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
#include "frpcbinary.h"
#include "frpcpool.h"

namespace FRPC
{




Binary_t::~Binary_t()
{}

Binary_t::Binary_t(Pool_t &pool,std::string::value_type *pData, std::string::size_type dataSize)
        :Value_t(pool),value(pData,dataSize)
{}

Binary_t::Binary_t(Pool_t &pool, const std::string &value)
        :Value_t(pool),value(value)
{}

std::string::size_type Binary_t::size() const
{
    return value.size();
}


const std::string::value_type*  Binary_t::data() const
{
    return  value.data();
}


std::string Binary_t::getString() const
{
    return value;
}

Value_t& Binary_t::clone(Pool_t &newPool) const
{
    return newPool.Binary(value);
}
}
