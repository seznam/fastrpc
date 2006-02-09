/*
 * FILE          $Id: frpcdouble.h,v 1.2 2006-02-09 16:00:26 vasek Exp $
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
#ifndef FRPCDOUBLE_H
#define FRPCDOUBLE_H

#include <frpcvalue.h>

namespace FRPC
{
class Pool_t;

/**
@brief Double type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Double_t : public Value_t
{
    friend class Pool_t;
public:
    enum{TYPE = 0x03};

    /**
    @brief Destructor
    */
    virtual ~Double_t();
    /**
    @brief Getting type of value
    @return  @b unsigned @b short always 
    @li @b Double_t::TYPE - identificator of double value
    */
    virtual unsigned short getType() const
    {
        return TYPE;
    }
    /**
        @brief Getting typename of value
        @return @b const @b char* always
        @li @b "Double" - typename of Double_t
    */
    virtual const char* getTypeName() const
    {
        return "double";
    }
    /**
    @brief Getting internal double value
    @return  @b double - internal value 
    */
    double getValue()
    {
        return value;
    }
    /**
        @brief Operator double
    */
    inline operator double& ()
    {
        return value;
    }
    /**
        @brief Operator double const
    */
    inline operator double () const 
    {
        return value;
    }
    /**
        @brief Operator const double const
    */
    inline operator const double& () const 
    {
        return value;
    }
    /**
        @brief Method for clone/copy Double_t 
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    virtual Value_t& clone(Pool_t &newPool) const;


private:
    /**
    @brief Default constructor is disabled
    */
    Double_t();
    /**
    @brief Costructor from double number
    @param pool is a reference to Pool_t used for allocating
    @param value  is a double value
    */
    Double_t(Pool_t &pool, double value):Value_t(pool),value(value)
    {}

    double value;/**  @brief Internal double value */

};
/**
    @brief Inline method
    
    Used to retype Value_t to Double_t 
    @return  If Value_t  can  retype to Double_t return reference to Double_t
    @n If Value_t can't retype to Double_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT Double_t& Double(Value_t &value)
{
    Double_t *double_v = dynamic_cast<Double_t*>(&value);

    if(!double_v)
        throw TypeError_t("Type is %s but not double",value.getTypeName());
    
    return *double_v;
}

/**
    @brief Inline method
    
    Used to retype Value_t to Double_t 
    @return  If Value_t  can  retype to Double_t return reference to Double_t
    @n If Value_t can't retype to Double_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT const Double_t& Double(const Value_t &value)
{
    const Double_t *double_v = dynamic_cast<const Double_t*>(&value);

    if(!double_v)
        throw TypeError_t("Type is %s but not double",value.getTypeName());
    
    return *double_v;
}

};

#endif
