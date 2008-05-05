/*
 * FastRPC -- Fast RPC library compatible with XML-RPC
 * Copyright (C) 2005-7  Seznam.cz, a.s.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Seznam.cz, a.s.
 * Radlicka 2, Praha 5, 15000, Czech Republic
 * http://www.seznam.cz, mailto:fastrpc@firma.seznam.cz
 *
 * FILE          $Id: frpcdouble.h,v 1.7 2008-05-05 12:52:00 burlog Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
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
    double getValue() const
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
        @brief Method for clone/copy Double_t 
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    virtual Value_t& clone(Pool_t &newPool) const;
    ///static members
    static const Double_t &FRPC_ZERO;

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
    explicit Double_t(const double &value)
        :value(value)
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
