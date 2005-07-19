/*
 * FILE          $Id: frpcunmarshaller.h,v 1.1 2005-07-19 13:02:54 vasek Exp $
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
#ifndef FRPCFRPCUNMARSHALLER_H
#define FRPCFRPCUNMARSHALLER_H

#include <frpcplatform.h>


namespace FRPC
{

class DataBuilder_t;


/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT UnMarshaller_t
{

public:
    enum {
        TYPE_METHOD_CALL=13,
        TYPE_METHOD_RESPONSE,
        TYPE_FAULT,
        TYPE_ANY
    };
    
    enum {
        BINARY_RPC,
        XML_RPC
    };
    
    UnMarshaller_t();

    virtual ~UnMarshaller_t();
    /**
        @brief unmarshall data
        @param data is input data 
            
        @param size is size of input data
        @param type expected main data type 
            @li @b TYPE_METHOD_CALL - method call
            @li @b TYPE_METHOD_RESPONSE - method response
            @li @b TYPE_ANY - any
    
    */  
    
    virtual void unMarshall(const char *data, long size, char type) = 0;
    /**
        @brief create marshaller object with contentType
        @param contentType is an content type 
            @li @b XML_RPC - create xml marshaller
            @li @b BINARY_RPC create binary marshaler
        
        @param dataBuilder is abstract object used to build data tree
        @return reference to new unMarshaller
    
    */  
    static UnMarshaller_t* create(long contentType, DataBuilder_t& dataBuilder);
    /**
        @brief create marshaller object from first container of data min 4 bytes
        @param data pointer to data which be unmarshalled
        @param size is data size min 4 bytes
        @param dataBuilder is abstract object used to build data tree
        @return reference to new unMarshaller
    
    */  

    static UnMarshaller_t* create(const char* data, long size, DataBuilder_t& dataBuilder);
    /**
        @brief finishing unmarshalling  
    */
    virtual void finish() = 0;

};

};

#endif
