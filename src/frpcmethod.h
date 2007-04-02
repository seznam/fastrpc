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
 * FILE          $Id: frpcmethod.h,v 1.2 2007-04-02 15:28:20 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCMETHOD_H
#define FRPCFRPCMETHOD_H

#include <frpcplatform.h>

#include <frpc.h>

namespace FRPC
{

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Method_t
{
public:
    Method_t()
    {}

    virtual ~Method_t()
    {}

    virtual Value_t& call(Pool_t &pool, Array_t &params) = 0;

};



template <typename Object_t>
class BoundMethod_t : public Method_t
{

public:
    typedef Value_t& (Object_t::*Handler_t) (Pool_t &pool, Array_t &params);

    BoundMethod_t(Object_t &object, Handler_t handler)
            :Method_t(),object(object),handler(handler)
    {}

    virtual ~BoundMethod_t()
    {}

    virtual Value_t& call(Pool_t& pool, Array_t& params)
    {
        return (object.*handler)(pool, params);
    }
private:

    Object_t &object;
    Handler_t handler;

};

template <typename Object_t>
BoundMethod_t<Object_t>* boundMethod(typename BoundMethod_t<Object_t>::Handler_t handler,Object_t & object )
{
    return new BoundMethod_t<Object_t>(object, handler);
}

template <typename UserData_t>
class UnboundMethod_t : public Method_t
{
    

public:
    
typedef Value_t& (*Handler_t)(Pool_t &pool, Array_t &params, UserData_t &data);

    UnboundMethod_t(Handler_t handler, UserData_t &data)
            :Method_t(),handler(handler),data(data)
    {}

    virtual ~UnboundMethod_t()
    {}

    virtual Value_t& call(Pool_t& pool, Array_t& params)
    {
        return handler(pool, params, data);
    }

private:
    Handler_t handler;
    UserData_t &data;
};

template <typename UserData_t>
UnboundMethod_t<UserData_t>* unboundMethod(typename UnboundMethod_t<UserData_t>::Handler_t handler,
 UserData_t &data)
{
    return new UnboundMethod_t<UserData_t>(handler, data);
}



}

#endif
