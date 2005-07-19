/*
 * FILE          $Id: frpcunmarshaller.cc,v 1.1 2005-07-19 13:02:54 vasek Exp $
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
#include "frpcunmarshaller.h"
#include <frpcbinunmarshaller.h>
#include <frpcxmlunmarshaller.h>
#include <frpcerror.h>
#include <frpcinternals.h>

namespace FRPC
{

UnMarshaller_t::UnMarshaller_t()
{}


UnMarshaller_t::~UnMarshaller_t()
{}


UnMarshaller_t* UnMarshaller_t::create(long contentType,
                                       DataBuilder_t& dataBuilder)
{
    UnMarshaller_t *unMarshaller;

    switch(contentType)
    {
    case BINARY_RPC:
        unMarshaller = new BinUnMarshaller_t(dataBuilder);
        break;

    case XML_RPC:
        unMarshaller = new XmlUnMarshaller_t(dataBuilder);
        break;

    default:
        throw Error_t("This unMarshaller not exists");
        break;
    }

    return unMarshaller;
}

UnMarshaller_t* UnMarshaller_t::create(const char* data, long size,
                                       DataBuilder_t& dataBuilder)
{
    UnMarshaller_t *unMarshaller;
    char magic[]={0xCA, 0x11, FRPC_MAJOR_VERSION, FRPC_MINOR_VERSION};
    
    if(size < 4)
    {
        unMarshaller = new XmlUnMarshaller_t(dataBuilder);
        unMarshaller->unMarshall(data, size, TYPE_ANY);
    }
        
    if(memcmp(data, magic, 4) != 0)
    {
        unMarshaller = new XmlUnMarshaller_t(dataBuilder);
        unMarshaller->unMarshall(data, size, TYPE_ANY);
    }
    else
    {
        unMarshaller = new BinUnMarshaller_t(dataBuilder);
        unMarshaller->unMarshall(data, size, TYPE_ANY);
    }  
        

    return unMarshaller;
}
}

