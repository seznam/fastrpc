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
 * FILE          $Id: frpcmethodregistry.h,v 1.5 2007-05-18 15:29:45 mirecta Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCMETHODREGISTRY_H
#define FRPCFRPCMETHODREGISTRY_H

#include<map>
#include<string>
#include<frpcmethod.h>

#include "frpcsocket.h"


namespace FRPC
{

/**
@brief MethodRegistry_t is used to register method on server side
@author Miroslav Talasek
*/
class Writer_t;

class Array_t;
class Value_t;
class Fault_t;
class DefaultMethod_t;
class HeadMethod_t;
class Pool_t;

class FRPC_DLLEXPORT MethodRegistry_t
{
public:
    enum {FRPC_INTERNAL_ERROR  = -500,
          FRPC_TYPE_ERROR = -501,
          FRPC_INDEX_ERROR  = -502,
          FRPC_PARSE_ERROR  = -503,
          FRPC_NETWORK_ERROR = -504,
          FRPC_TIMEOUT_ERROR = -505,
          FRPC_NO_SUCH_METHOD_ERROR = -506,
          FRPC_REQUEST_REFUSED_ERROR = -507,
          FRPC_INTROSPECTION_DISABLED_ERROR = -508,
          FRPC_LIMIT_EXCEEDED_ERROR = -509,
          FRPC_INVALID_UTF8_ERROR = -510
         };

    struct RegistryEntry_t
    {
        RegistryEntry_t(Method_t* method, const std::string &signature,const std::string &help)
                :method(method),signature(signature),help(help)
        {}
        ~RegistryEntry_t()
        {}

        Method_t *method;
        std::string signature;
        std::string help;
    };


    class Reader_t
    {
    public:
        Reader_t()
        {}
        virtual unsigned int read(char *data, unsigned int size) = 0;
        virtual ~Reader_t()
        {}
    }
    ;

    struct TimeDiff_t
    {
        TimeDiff_t(unsigned int second, unsigned int usecond):second(second),usecond(usecond)
        {}
        TimeDiff_t();
        TimeDiff_t diff();

        long second;
        long usecond;
    };

    /**
    @brief Callbacks_t is using to logging on server side
    @author Miroslav Talasek
    */
    class Callbacks_t
    {
    public :
        Callbacks_t()
        {}
        
        
        /**
        @brief this method was called before method call 
        */
        virtual void preProcess(const std::string &methodName, const std::string &clientIP
                                ,Array_t &params) = 0;

        /**
        @brief this method was called after method call if status ok 
        */
        virtual void postProcess(const std::string &methodName, const std::string &clientIP,
                                 const Array_t &params,
                                 const Value_t &result, const TimeDiff_t &time) = 0;
        /**
        @brief this method was called after method call if status is fault 
        */

        virtual void postProcess(const std::string &methodName, const std::string &clientIP,
                                 const Array_t &params,
                                 const Fault_t &fault, const TimeDiff_t &time) = 0 ;

        
        virtual ~Callbacks_t()
        {}
        
//         /**
//         @brief this method was called before reading from socket
//         */
        
//         virtual void preRead(const std::string &clientIP, int requestCount)
//         {}
    }
    ;
    /**
     * @brief Method registy constructor
     * @param callbacks it is the pointer to exist Class Callbacks_t or 0
     * @param introspectionEnabled if is @b true the builtin methods will be registered
     * @n Builtin methods:
     * @li  array system.listMethods() -  List all registered methods
     * @li  string system.methodHelp(string) - Return given method help
     * @li  array  system.methodSignature(string) - Return given method signature
     * @li struct  system.multicall(struct) - Call given methods
     * 
     */
    MethodRegistry_t(Callbacks_t *callbacks, bool introspectionEnabled);

    /**
    @brief register method by method name and method handler
    @param methodName it is the method name in string
    @param method it is the Method_t handler of method which be registered
    @param signature is method signature returnType:param1:param2:param3
    @n example : method : string  stat(struct,int,array)
    @n have signature s:S:i:A
    @param help  is method help as string
    */
    void registerMethod(const std::string &methodName, Method_t *method,
                        const std::string signature = "",
                        const std::string help = "No help" );
    /**
    @brief call head method on HTTP HEAD
    @return long 
    @li @b   0 -OK
    @li @b   1 - error
    @li @b  -1 method not registered     

    */
    long headCall();

    /**
    @brief call method  
    */
    long processCall(const std::string &clientIP, Reader_t &reader, unsigned int typeIn,
                     Writer_t &writer, unsigned int typeOut);

    long processCall(const std::string &clientIP, const std::string &methodName,
                     Array_t &params, Writer_t &writer, unsigned int typeOut,
                     const ProtocolVersion_t &protocolVersion);

    Value_t& processCall(const std::string &clientIP, const std::string &methodName,
                         Array_t &params, Pool_t &pool);

    Value_t& processCall(const std::string &clientIP, Reader_t &reader, 
                         unsigned int typeIn,Pool_t &pool);

    /**
    @brief register  default method which be call when method not found
    */
    void registerDefaultMethod(DefaultMethod_t *defaultMethod);
    /**
    @brief register  head method for HTTP HEAD
    */
    void registerHeadMethod(HeadMethod_t *headMethod);
    ~MethodRegistry_t();


private:
    //system methods
    Value_t& listMethods(Pool_t &pool, Array_t &params);
    Value_t& methodHelp(Pool_t &pool, Array_t &params);
    Value_t& methodSignature(Pool_t &pool, Array_t &params);
    Value_t& muticall(Pool_t &pool, Array_t &params);


    std::map<std::string, RegistryEntry_t> methodMap;

    Callbacks_t *callbacks;
    bool introspectionEnabled;
    DefaultMethod_t *defaultMethod;
    HeadMethod_t *headMethod;
};

};

#endif
