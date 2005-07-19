/*
 * FILE          $Id: frpcmarshaller.cc,v 1.1 2005-07-19 13:02:54 vasek Exp $
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
#include "frpcmarshaller.h"
#include <frpcwriter.h>
#include <frpcbinmarshaller.h>
#include <frpcxmlmarshaller.h>
#include <frpcerror.h>

namespace FRPC
{

Marshaller_t::Marshaller_t()
{}


Marshaller_t::~Marshaller_t()
{}

 Marshaller_t* Marshaller_t::create(long contentType, Writer_t& writer)
 {
    Marshaller_t *marshaller;
    
        switch(contentType)
        {
         case BINARY_RPC:
            marshaller = new BinMarshaller_t(writer);
            break;
         
         case XML_RPC:
            marshaller = new XmlMarshaller_t(writer);
            break;
         
         default:
            throw Error_t("This marshaller not exists");
            break;
        }
 
        return marshaller;
 }

}
