/*
 * FILE          $Id: frpcdefaultmethod.h,v 1.1 2005-07-19 13:02:53 vasek Exp $
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
