/*
 * FILE          $Id: frpctreefeeder.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
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
#ifndef FRPCFRPCTREEFEEDER_H
#define FRPCFRPCTREEFEEDER_H

#include <frpcplatform.h>

#include <frpcmarshaller.h>
#include <frpc.h>

namespace FRPC
{

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT TreeFeeder_t
{
public:
    TreeFeeder_t(Marshaller_t &marshaller):marshaller(marshaller)
    {}
    
    void feedValue(Value_t &value);

    
    
    ~TreeFeeder_t();
    
private:

    Marshaller_t &marshaller;

};

};

#endif
