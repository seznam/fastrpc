/*
 * FILE          $Id: frpcbool.h,v 1.2 2006-02-09 16:00:26 vasek Exp $
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
#ifndef FRPCBOOL_H
#define FRPCBOOL_H

#include <frpcvalue.h>

namespace FRPC
{
class Pool_t;

/**
@brief Bool type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT  Bool_t : public Value_t
{
    friend class Pool_t;
public:
    enum{TYPE = 0x02};
    /**
        @brief Destructor
    */
    virtual ~Bool_t();
    /**
        @brief Getting type of value
        @return @b unsigned @b short always 
        @li @b Bool_t::TYPE - identificator of boolean value
    */
    virtual unsigned short getType() const
    {
        return TYPE;
    }
    /**
        @brief Getting typename of value
        @return @b const @b char* always
        @li @b "Bool" - typename of Bool_t
    */
    virtual const char* getTypeName() const
    {
        return "bool";
    }
    /**
        @brief Getting internal boolean value
        @return  @b bool - internal value 
    */
    bool getValue() const
    {
        return value;
    }
    /**
        @brief Operator bool
    */
    inline operator bool& ()
    {
    return value;
    }
    /**
        @brief Operator bool const
    */
    inline operator bool() const 
    {
    return value;
    }
    /**
        @brief Operator const bool const
    */
    inline operator const bool& () const
    {
    return value;
    }
    /**
        @brief Method to clone/copy Bool_t 
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    virtual Value_t& clone(Pool_t &newPool) const;

private :
    /**
        @brief Default constructor is disabled
    */
    Bool_t(Pool_t &pool):Value_t(pool)
    {}
    Bool_t();
    /**
        @brief Costructor from bool value
        @param pool is a reference to Pool_t used for allocating
        @param boolean  is a bool value
    */
    Bool_t(Pool_t &pool, bool boolean):Value_t(pool),value(boolean)
    {}

    bool value; /**  Internal bool value */
    ;

};
/**
    @brief Inline method
    
    Used to retype Value_t to Bool_t 
    @return  If Value_t  can  retype to Bool_t return reference to Bool_t
    @n If Value_t can't retype to Bool_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT Bool_t& Bool(Value_t &value)
{
    Bool_t *boolean = dynamic_cast<Bool_t*>(&value);

    if(!boolean)
        throw TypeError_t("Type is %s but not bool",value.getTypeName());
        
    return *boolean;
}

inline FRPC_DLLEXPORT const Bool_t& Bool(const Value_t &value)
{
    const Bool_t *boolean = dynamic_cast<const Bool_t*>(&value);

    if(!boolean)
        throw TypeError_t("Type is %s but not bool",value.getTypeName());
        
    return *boolean;
}
};

#endif
