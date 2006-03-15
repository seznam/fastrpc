/*
 * FILE          $Id: frpcint.h,v 1.3 2006-03-15 08:54:12 vasek Exp $
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
#ifndef FRPCINT_H
#define FRPCINT_H

#include <frpcvalue.h>
#include "frpcpool.h"

#include "frpctypeerror.h"
namespace FRPC
{
class Pool_t;
/**
@brief Int type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Int_t : public Value_t
{
    friend class Pool_t;
public:

    /**
    @brief Destructor
    */
    virtual ~Int_t()
    {}

    /**
    @brief Getting type of value
    @return  @b unsigned @b short always 
    @li @b Int_t::type - identificator of inteeger value
    */
    virtual unsigned short getType() const
    {
        return TYPE;
    }

    /**
        @brief Getting typename of value
        @return @b const @b char  always
        @li @b "Int" - typename of Int_t
    */
    virtual const char* getTypeName() const
    {
        return "int";
    }

    /**
    @brief Getting internal integer value
    @return   @b long - internal value 
    */
    long getValue() const
    {
        return value;
    }

    /**
    @brief Operator long
    */
    operator long () const
    {
        return value;
    }

    /**
        @brief Method for clone/copy Double_t 
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    virtual Value_t& clone(Pool_t &newPool) const;

    enum{ TYPE = 0x01 };
private:
    /**
    @brief default constructor  is disabled
    */
    Int_t();
    /**
    @brief Costructor from long number
    @param pool is a reference to Pool_t used for allocating
    @param number  is an  long value
    */
    Int_t(Pool_t &pool, long number)
            :Value_t(pool),value(number)
    {}


    long value; /**  Internal long value */

};
/**
    @brief Inline method
    
    Used to retype Value_t to Int_t 
    @return  If Value_t  can  retype to Int_t return reference to  Int_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t
    
*/
inline FRPC_DLLEXPORT Int_t& Int(Value_t &value)
{
    Int_t *integer = dynamic_cast<Int_t*>(&value);

    if(!integer)
        throw TypeError_t("Type is %s but not int",value.getTypeName());
    return *integer;
}

/**
    @brief Inline method
    
    Used to retype Value_t to Int_t 
    @return  If Value_t  can  retype to Int_t return reference to  Int_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t
    
*/
inline FRPC_DLLEXPORT const Int_t& Int(const Value_t &value)
{
    const Int_t *integer = dynamic_cast<const Int_t*>(&value);

    if(!integer)
        throw TypeError_t("Type is %s but not int",value.getTypeName());
    return *integer;
}
};

#endif
