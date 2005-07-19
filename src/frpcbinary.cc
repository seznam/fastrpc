/*
 * FILE          $Id: frpcbinary.cc,v 1.1 2005-07-19 13:02:53 vasek Exp $
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

std::string::size_type Binary_t::size()
{
    return value.size();
}


const std::string::value_type*  Binary_t::data()
{
    return  value.data();
}


std::string Binary_t::getString()
{
    return value;
}

Value_t& Binary_t::clone(Pool_t &newPool) const
{
    return newPool.Binary(value);
}
}
