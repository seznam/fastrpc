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
 * FILE          $Id: frpcmethodregistry.cc,v 1.11 2009-03-17 11:27:37 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include "frpcmethodregistry.h"

#include <frpcmethod.h>
#include <frpcdefaultmethod.h>
#include <frpcheadmethod.h>
#include <frpctreebuilder.h>
#include <frpctreefeeder.h>
#include <frpcunmarshaller.h>
#include <frpcmarshaller.h>
#include <frpcstreamerror.h>
#include <frpcfault.h>
#include <frpclenerror.h>
#include <frpckeyerror.h>
#include <frpcindexerror.h>
#include <frpcprotocolerror.h>
#include <frpc.h>
#include <frpcinternals.h>
#include <memory>
#include <stdexcept>

#ifdef WIN32
#include <windows.h>
#endif //WIN32

namespace FRPC
{

MethodRegistry_t::TimeDiff_t::TimeDiff_t()
{
    // get current time
#ifdef WIN32
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        time(&second);
        usecond = (ft.dwLowDateTime / 10) % 1000000;

#else //WIN32

    struct timeval now;
    gettimeofday(&now, 0);
    second = now.tv_sec;
    usecond = now.tv_usec;
#endif //WIN32
}

MethodRegistry_t::TimeDiff_t MethodRegistry_t::TimeDiff_t::diff()
{

    TimeDiff_t now;

    // check for time skew
    if ((now.second < second)
            || ((now.second == second) && (now.usecond < usecond)))
        return TimeDiff_t(0,0);

    // compute difference
    if (now.usecond >= usecond)
        return TimeDiff_t(now.second - second,
                          now.usecond - usecond);

    return TimeDiff_t
           (now.second - second - 1L,
            1000000L - usecond + now.usecond);
}

MethodRegistry_t::MethodRegistry_t(Callbacks_t *callbacks, bool introspectionEnabled)
        :callbacks(callbacks), introspectionEnabled(introspectionEnabled),
        defaultMethod(0),headMethod(0)
{
    if(introspectionEnabled)
    {
        registerMethod("system.listMethods",boundMethod(&MethodRegistry_t::listMethods, *this),
                       "A:", "List all registered methods");

        registerMethod("system.methodHelp",boundMethod(&MethodRegistry_t::methodHelp, *this),
                       "s:s", "Return given method help");

        registerMethod("system.methodSignature",boundMethod(&MethodRegistry_t::methodSignature,
                       *this), "A:s", "Return given method signature");

        registerMethod("system.multicall",boundMethod(&MethodRegistry_t::multicall,
                       *this), "A:A", "Call given methods");
    }

}

void MethodRegistry_t::registerMethod(const std::string &methodName, Method_t *method,
                                      const std::string signature , const std::string help )
{
    typedef std::map<std::string, RegistryEntry_t> Map_t;

    RegistryEntry_t entry(method, signature, help);

    // try to insert method
    std::pair<Map_t::iterator, bool>
        res(methodMap.insert(Map_t::value_type(methodName, entry)));

    if (!res.second) {
        // not inserted => replace method
        res.first->second = entry;
    }
}

void MethodRegistry_t::registerDefaultMethod(DefaultMethod_t *defaultMethod)
{
    if(this->defaultMethod != 0)
        delete  this->defaultMethod;

    this->defaultMethod = defaultMethod;
}

void MethodRegistry_t::registerHeadMethod(HeadMethod_t *headMethod)
{
    if(this->headMethod != 0)
        delete  this->headMethod;

    this->headMethod = headMethod;
}
MethodRegistry_t::~MethodRegistry_t()
{
    for(std::map<std::string, RegistryEntry_t>::iterator i = methodMap.begin();
            i != methodMap.end(); ++i)
    {
        delete i->second.method;
    }
}
/*
namespace
{
const long BUFFER_SIZE = 1<<16;
}
*/
int MethodRegistry_t::processCall(const std::string &clientIP, const std::string &methodName,
                                   Array_t &params,
                                   Writer_t &writer, unsigned int typeOut,
                                   const ProtocolVersion_t &protocolVersion)
{
    Pool_t pool;
    std::auto_ptr<Marshaller_t> marshaller(Marshaller_t::create(typeOut, writer,
                                                               protocolVersion));
    TimeDiff_t timeD;

    TreeFeeder_t feeder(*marshaller);

    try
    {

        Value_t &retValue = processCall(clientIP, methodName, params, pool);


        marshaller->packMethodResponse();
        feeder.feedValue(retValue);
        marshaller->flush();

    }

    catch(const StreamError_t &streamError)
    {
        if(callbacks)
            callbacks->postProcess(methodName, clientIP, params,Fault_t(FRPC_PARSE_ERROR,
                                   streamError.message()),timeD.diff());
        marshaller->packFault(FRPC_PARSE_ERROR,streamError.message().c_str());
        marshaller->flush();
    }
    catch(const Fault_t &fault)
    {
        if(callbacks)
            callbacks->postProcess(methodName, clientIP, params, fault,timeD.diff());
        marshaller->packFault(fault.errorNum(),fault.message().c_str());
        marshaller->flush();

    } catch (const ProtocolError_t &pe) {
        throw;

    } catch (const std::exception &e) {
        Fault_t fault(FRPC_INTERNAL_ERROR,
                      "Caught std::exception<%s> while dispatching "
                      "method call.", e.what());
        if (callbacks)
            callbacks->postProcess(methodName, clientIP, params, fault, timeD.diff());
        marshaller->packFault(fault.errorNum(),fault.message().c_str());
        marshaller->flush();

    } catch (...) {
        Fault_t fault(FRPC_INTERNAL_ERROR,
                      "Caught unknown exception while dispatching "
                      "method call.");
        if (callbacks)
            callbacks->postProcess(methodName, clientIP, params, fault, timeD.diff());
        marshaller->packFault(fault.errorNum(),fault.message().c_str());
        marshaller->flush();
    }

    return 0;
}

Value_t& MethodRegistry_t::processCall(const std::string &clientIP,
                                       const std::string &methodName,
                                       Array_t &params,
                                       Pool_t &pool)
{
    TimeDiff_t timeD;
    Value_t *result;
    try
    {
        std::map<std::string, RegistryEntry_t>::const_iterator
            pos = methodMap.find(methodName);

        if (pos == methodMap.end()){
            if (callbacks)
                callbacks->preProcess(methodName, clientIP, params);

            // if not default method generate fault
            if (!defaultMethod)
                throw Fault_t(FRPC_NO_SUCH_METHOD_ERROR, "Method %s not found",
                              methodName.c_str());

            result = &(defaultMethod->call(pool, methodName,params));
            if (callbacks)
                callbacks->postProcess(methodName, clientIP, params, *result,
                                       timeD.diff());
        } else {
            if(callbacks)
                callbacks->preProcess(methodName, clientIP, params);

            result = &(pos->second.method->call(pool, params));

            // prepare deprecated warning

            if(callbacks)
                callbacks->postProcess(methodName, clientIP, params, *result,
                                       timeD.diff());
        }
    }
    catch(const TypeError_t &typeError)
    {
        throw Fault_t(FRPC_TYPE_ERROR,typeError.message());
    }
    catch(const LenError_t &lenError)
    {
        throw Fault_t(FRPC_TYPE_ERROR,lenError.message());
    }
    catch(const KeyError_t &keyError)
    {
        throw Fault_t(FRPC_INDEX_ERROR,keyError.message());
    }
    catch(const IndexError_t &indexError)
    {
        throw Fault_t(FRPC_INDEX_ERROR,indexError.message());
    }
    catch(const StreamError_t &streamError)
    {
        throw Fault_t(FRPC_PARSE_ERROR,streamError.message());
    }

    return *result;

}

Value_t& MethodRegistry_t::processCall(const std::string &clientIP, Reader_t &reader,
                                       unsigned int typeIn, Pool_t &pool)
{
    TreeBuilder_t builder(pool);
    std::auto_ptr<UnMarshaller_t> unmarshaller(UnMarshaller_t::create(typeIn, builder));
    char buffer[BUFFER_SIZE];
    long readed;
    Value_t *result;

    try
    {
        while((readed = reader.read(buffer,BUFFER_SIZE)))
        {
            unmarshaller->unMarshall(buffer, readed,
                                     UnMarshaller_t::TYPE_METHOD_CALL);
        }


        result = &(processCall(clientIP, builder.getUnMarshaledMethodName(),
                               Array(builder.getUnMarshaledData()),pool));
    }
    catch(const StreamError_t &streamError)
    {
        throw Fault_t(FRPC_PARSE_ERROR,streamError.message());
    }

    return *result;
}

int MethodRegistry_t::processCall(const std::string &clientIP, Reader_t &reader,
                                   unsigned int typeIn, Writer_t &writer,
                                   unsigned int typeOut)
{
    Pool_t pool;
    TreeBuilder_t builder(pool);
    std::auto_ptr<UnMarshaller_t> unmarshaller(UnMarshaller_t::create(typeIn, builder));
    Value_t *retValue;


    char buffer[BUFFER_SIZE];
    unsigned int readed;


    try
    {
        while((readed = reader.read(buffer,BUFFER_SIZE)))
        {
            unmarshaller->unMarshall(buffer, readed,
                                     UnMarshaller_t::TYPE_METHOD_CALL);
        }
        std::auto_ptr<Marshaller_t> marshaller(Marshaller_t::create(typeOut,
                                               writer,
                                               unmarshaller->getProtocolVersion()));
        TreeFeeder_t feeder(*marshaller);

        retValue = &(processCall(clientIP, builder.getUnMarshaledMethodName(),
                                 Array(builder.getUnMarshaledData()),pool));

        marshaller->packMethodResponse();

        feeder.feedValue(*retValue);
        marshaller->flush();
    }
    catch(const StreamError_t &streamError)
    {
        std::auto_ptr<Marshaller_t> marshaller(Marshaller_t::create(typeOut,
                                               writer,
                                               unmarshaller->getProtocolVersion()));
        TreeFeeder_t feeder(*marshaller);
        marshaller->packFault(FRPC_PARSE_ERROR,streamError.message().c_str());
        marshaller->flush();
    }
    catch(const Fault_t &fault)
    {
        std::auto_ptr<Marshaller_t> marshaller(Marshaller_t::create(typeOut,
                                               writer,
                                               unmarshaller->getProtocolVersion()));
        TreeFeeder_t feeder(*marshaller);
        marshaller->packFault(fault.errorNum(),fault.message().c_str());
        marshaller->flush();
    }

    return 0;
}

int MethodRegistry_t::headCall()
{

    //if head method registered call it
    if(!headMethod)
    {
        return -1;
    }
    else
    {
        if(headMethod->call())
            return 0;
        else
            return 1;
    }

}


//*******************system methods************************************
Value_t& MethodRegistry_t::listMethods(Pool_t &pool, Array_t &params)
{
    if(params.size() != 0)
        throw Fault_t(FRPC_TYPE_ERROR,"Method required 0 arguments but %d argumet(s) given",
                      params.size());

    Array_t &retArray = pool.Array();

    for(std::map<std::string, RegistryEntry_t>::iterator i = methodMap.begin();
            i != methodMap.end(); ++i)
    {
        retArray.append(pool.String(i->first));
    }

    return retArray;
}

Value_t& MethodRegistry_t::methodHelp(Pool_t &pool, Array_t &params)
{
    if(params.size() != 1)
        throw Fault_t(FRPC_TYPE_ERROR,"Method required 1 argument but %d argumet(s) given",
                      params.size());

    params.checkItems("s");


    std::map<std::string, RegistryEntry_t>::const_iterator pos = methodMap.find(
                String(params[0]).getString());

    if(pos == methodMap.end())
        throw Fault_t(FRPC_NO_SUCH_METHOD_ERROR,"Method %s not found",
                      String(params[0]).getString().c_str());

    return pool.String(pos->second.help);
}

Value_t& MethodRegistry_t::methodSignature(Pool_t &pool, Array_t &params)
{
    if(params.size() != 1)
        throw Fault_t(FRPC_TYPE_ERROR,"Method required 1 argument but %d argumet(s) given",
                      params.size());

    params.checkItems("s");

    std::map<std::string, RegistryEntry_t>::const_iterator pos = methodMap.find(
                String(params[0]).getString());

    if(pos == methodMap.end())
        throw Fault_t(FRPC_NO_SUCH_METHOD_ERROR,"Method %s not found",
                      String(params[0]).getString().c_str());

    //build array signature
    Array_t &array = pool.Array();
    Array_t *actual = &(pool.Array());

    array.append(*actual);

    for(unsigned long i = 0; i <= pos->second.signature.size(); i++)
    {
        switch(pos->second.signature[i])
        {
        case 'A':
            actual->append(pool.String("array"));
            break;
        case 'S':
            actual->append(pool.String("struct"));
            break;
        case 'B':
            actual->append(pool.String("binary"));
            break;
        case 'D':
            actual->append(pool.String("dateTime"));
            break;
        case 'b':
            actual->append(pool.String("bool"));
            break;
        case 'd':
            actual->append(pool.String("double"));
            break;
        case 'i':
            actual->append(pool.String("int"));
            break;
        case 's':
            actual->append(pool.String("string"));
            break;
        case ':':
            break;
        case ',':
            actual = &(pool.Array());
            array.append(*actual);
            break;
        default:
            break;

        }

    }
    return array;
}
Value_t& MethodRegistry_t::multicall(Pool_t &pool, Array_t &params)
{
    if(params.size() != 1)
        throw Fault_t(FRPC_TYPE_ERROR,"Method required 1 argument but %d argumet(s) given",
                      params.size());

    params.checkItems("A");

    Array_t &array = pool.Array();

    for(Array_t::const_iterator pos = Array(params[0]).begin(); pos != Array(params[0]).end();
            ++pos)
    {
        if((*pos)->getType() != Struct_t::TYPE)
        {
            array.append(pool.Struct("faultCode",pool.Int(FRPC_TYPE_ERROR),
                                     "faultString",pool.String("Parameter must be struct")));
            continue;
        }

        try
        {

            Struct_t &strct = Struct(**pos);

            std::map<std::string, RegistryEntry_t>::const_iterator pos = methodMap.find(
                        String(strct["methodName"]).getString());

            if(pos == methodMap.end())
            {

                //if default method registered call it
                if(!defaultMethod)
                {
                    throw Fault_t(FRPC_NO_SUCH_METHOD_ERROR,"Method %s not found",
                                  String(strct["methodName"]).getString().c_str());

                }
                else
                {
                    array.append(pool.Array(defaultMethod->call(pool,
                                            String(strct["methodName"]).getString(),
                                            Array(strct["params"]))));
                }

            }
            else
            {
                array.append(pool.Array(pos->second.method->call(pool,
                                        Array(strct["params"]))));

            }

        }
        catch(const TypeError_t &typeError)
        {
            array.append(pool.Struct("faultCode",pool.Int(FRPC_TYPE_ERROR),
                                     "faultString",pool.String(typeError.message())));
        }
        catch(const LenError_t &lenError)
        {
            array.append(pool.Struct("faultCode",pool.Int(FRPC_TYPE_ERROR),
                                     "faultString",pool.String(lenError.message())));
        }
        catch(const KeyError_t &keyError)
        {
            array.append(pool.Struct("faultCode",pool.Int(FRPC_INDEX_ERROR),
                                     "faultString",pool.String(keyError.message())));
        }
        catch(const IndexError_t &indexError)
        {
            array.append(pool.Struct("faultCode",pool.Int(FRPC_INDEX_ERROR),
                                     "faultString",pool.String(indexError.message())));

        }
        catch(const Fault_t &fault)
        {

            array.append(pool.Struct("faultCode",pool.Int(fault.errorNum()),
                                     "faultString",pool.String(fault.message())));
        }


    }

    return array;
}

}
