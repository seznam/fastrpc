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
 * FILE          $Id: frpcdefaultmethod.h,v 1.2 2007-04-02 15:28:21 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCDEFAULTMETHOD_H
#define FRPCFRPCDEFAULTMETHOD_H

#include <frpcplatform.h>

#include <frpc.h>

namespace FRPC
{

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT DefaultMethod_t
{
public:
    DefaultMethod_t()
    {}

    virtual ~DefaultMethod_t()
    {}

    virtual Value_t& call(Pool_t &pool, const std::string &methodName, Array_t &params) = 0;

};



template <typename Object_t>
class DefaultBoundMethod_t : public DefaultMethod_t
{

public:
    typedef Value_t& (Object_t::*Handler_t) (Pool_t &pool, const std::string &methodName,
            Array_t &params);

    DefaultBoundMethod_t(Object_t &object, Handler_t handler)
            :DefaultMethod_t(),object(object),handler(handler)
    {}

    virtual ~DefaultBoundMethod_t()
    {}

    virtual Value_t& call(Pool_t& pool, const std::string &methodName, Array_t& params)
    {
        return (object.*handler)(pool, methodName, params);
    }
private:

    Object_t &object;
    Handler_t handler;

};

template <typename Object_t>
DefaultBoundMethod_t<Object_t>* boundDefaultMethod(
    typename DefaultBoundMethod_t<Object_t>::Handler_t handler,Object_t & object )
{
    return new DefaultBoundMethod_t<Object_t>(object, handler);
}

template <typename UserData_t>
class DefaultUnboundMethod_t : public DefaultMethod_t
{


public:

    typedef Value_t& (*Handler_t)(Pool_t &pool, const std::string &methodName,
                                  Array_t &params, UserData_t &data);

    DefaultUnboundMethod_t(Handler_t handler, UserData_t &data)
            :DefaultMethod_t(),handler(handler),data(data)
    {}

    virtual ~DefaultUnboundMethod_t()
    {}

    virtual Value_t& call(Pool_t& pool, const std::string &methodName, Array_t& params)
    {
        return handler(pool, methodName, params, data);
    }

private:
    Handler_t handler;
    UserData_t &data;
};

template <typename UserData_t>
DefaultUnboundMethod_t<UserData_t>* unboundDefaultMethod(
    typename DefaultUnboundMethod_t<UserData_t>::Handler_t handler,
    UserData_t &data)
{
    return new DefaultUnboundMethod_t<UserData_t>(handler, data);
}



};

#endif
