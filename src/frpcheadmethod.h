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
 * FILE          $Id: frpcheadmethod.h,v 1.2 2007-04-02 15:28:21 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCHEADMETHOD_H
#define FRPCFRPCHEADMETHOD_H

#include <frpcplatform.h>

#include <frpc.h>

namespace FRPC
{

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT HeadMethod_t
{
public:
    HeadMethod_t()
    {}

    virtual ~HeadMethod_t()
    {}

    virtual bool call() = 0;

};



template <typename Object_t>
class HeadBoundMethod_t : public HeadMethod_t
{

public:
    typedef bool (Object_t::*Handler_t) ();

    HeadBoundMethod_t(Object_t &object, Handler_t handler)
            :HeadMethod_t(),object(object),handler(handler)
    {}

    virtual ~HeadBoundMethod_t()
    {}

    virtual bool call()
    {
        return (object.*handler)();
    }
private:

    Object_t &object;
    Handler_t handler;

};

template <typename Object_t>
HeadBoundMethod_t<Object_t>* boundHeadMethod(
    typename HeadBoundMethod_t<Object_t>::Handler_t handler,Object_t & object )
{
    return new HeadBoundMethod_t<Object_t>(object, handler);
}

template <typename UserData_t>
class HeadUnboundMethod_t : public HeadMethod_t
{


public:

    typedef bool (*Handler_t)(UserData_t &data);

    HeadUnboundMethod_t(Handler_t handler, UserData_t &data)
            :HeadMethod_t(),handler(handler),data(data)
    {}

    virtual ~HeadUnboundMethod_t()
    {}

    virtual bool call()
    {
        return handler(data);
    }

private:
    Handler_t handler;
    UserData_t &data;
};

template <typename UserData_t>
HeadUnboundMethod_t<UserData_t>* unboundHeadMethod(
    typename HeadUnboundMethod_t<UserData_t>::Handler_t handler,
    UserData_t &data)
{
    return new HeadUnboundMethod_t<UserData_t>(handler, data);
}



};

#endif
