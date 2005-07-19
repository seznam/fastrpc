/*
 * FILE          $Id: frpcheadmethod.h,v 1.1 2005-07-19 13:02:53 vasek Exp $
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
