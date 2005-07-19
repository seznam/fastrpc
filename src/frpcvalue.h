/*
 * FILE          $Id: frpcvalue.h,v 1.1 2005-07-19 13:02:55 vasek Exp $
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
#ifndef FRPCVALUE_H
#define FRPCVALUE_H

#include <frpcplatform.h>

#include <unistd.h>
#include <string>
#include "frpctypeerror.h"

namespace FRPC
{
class Pool_t;
/**
@brief Abstract Value type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Value_t
{
    friend class Pool_t;
public:
    /**
        @brief Default constructor
        @param pool is reference to Pool_t (memory pool)
    */
    Value_t(Pool_t &pool):pool(pool)
    {}
    /**
        @brief  Destructor
    */
    virtual ~Value_t();
    /**
        @brief Abstract virtual method to getting type of value
        @return @b unsigned  @b short type of value
        @li @b  Int_t::TYPE         - inteeger value type  
        @li @b  Bool_t::TYPE        - boolean value type  
        @li @b  Double_t::TYPE      - double value type   
        @li @b  String_t::TYPE      - string value type   
        @li @b  DateTime_t::TYPE    - date time value type 
        @li @b  Binary_t::TYPE      - binary value type    
                                              
        @li @b  Struct_t::TYPE      - struct value type    
        @li @b  Arry_t::TYPE        - array value type     
    */
    virtual unsigned short getType() = 0;
    
    /**
        @brief Abstract virtual method to getting typename of value
        @return @b const @b char* typename of value
        @li @b  "Int"           - inteeger value typename  
        @li @b  "Bool"          - boolean value typename  
        @li @b  "Double"        - double value typename   
        @li @b  "String"        - string value typename   
        @li @b  "DateTime"      - date time value typename 
        @li @b  "Binary"        - binary value typename    
                                              
        @li @b  "Struct"        - struct value typename    
        @li @b  "Array"         - array value typename
    */
    virtual const char* getTypeName() = 0;

    inline Value_t& clone() const
    {
        return this->clone(pool);
    }
    /**
       @brief Abstract virtual method to clone/copy Value_t 
       @param newPool is pointer of Pool_t which is used for allocate objects
       */
    virtual Value_t& clone(Pool_t &newPool) const = 0;
protected:
    Pool_t &pool;/// Memory pool

private:

    /**
        @brief Default constructor is disabled
    */
    Value_t();

    Value_t(const Value_t&);

    Value_t& operator=(const Value_t&);
};

}

#endif
