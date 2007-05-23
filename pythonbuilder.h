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
 * $Id: pythonbuilder.h,v 1.4 2007-05-23 09:31:43 mirecta Exp $
 *
 * AUTHOR      Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * DESCRIPTION
 * Python FastRPC support. Mimics C++ FastRPC server and method registry.
 *
 * HISTORY
 *      2006-05-24 (vasek)
 *              Created
 */


#ifndef PYTHONBUILDER_H_
#define PYTHONBUILDER_H_

#include <string>
#include <vector>

#include <Python.h>

#include <frpcdatabuilder.h>
#include <frpcint.h>

/***************************************************************************/
/***Builder for unmarshaller                                             ***/
/***************************************************************************/

namespace FRPC { namespace Python {

enum StringMode_t {
    STRING_MODE_INVALID = -1,
    STRING_MODE_MIXED = 0,
    STRING_MODE_UNICODE,
    STRING_MODE_STRING
};
const Int_t::value_type ZERO = 0;
const Int_t::value_type ALLONES = ~ZERO;
const Int_t::value_type INT31_MASK = ALLONES << 31;

StringMode_t parseStringMode(const char *stringMode);

struct TypeStorage_t
{

    TypeStorage_t(PyObject *container, char
                  type):type(type),container(container)
    {}
    char type;
    PyObject* container;
};

class Builder_t : public FRPC::DataBuilder_t
{
public:
    enum{NONE=0, INT=1,BOOL,DOUBLE,STRING,DATETIME,BINARY,INTP8,INTN8,
         STRUCT=10,ARRAY,METHOD_CALL=13,METHOD_RESPONSE,FAULT,
         MEMBER_NAME = 100,METHOD_NAME,METHOD_NAME_LEN,MAGIC,MAIN };

    Builder_t(PyObject *methodObject, StringMode_t stringMode)
        : FRPC::DataBuilder_t(), first(true), error(false), retValue(Py_None),
          methodObject(methodObject), methodName(0), stringMode(stringMode)
    {
        Py_INCREF(retValue);
    }

    ~Builder_t() {
        delete methodName;
        Py_XDECREF(retValue);
    }

    virtual void buildMethodResponse();
    virtual void buildBinary(const char* data, unsigned int size);
    virtual void buildBinary(const std::string &data);
    virtual void buildBool(bool value);
    virtual void buildDateTime(short year, char month, char day,char hour, char
                               min, char sec,
                               char weekDay, time_t unixTime, char timeZone);
    virtual void buildDouble(double value);
    virtual void buildFault(int errNumber, const char* errMsg, unsigned int size );
    virtual void buildFault(int errNumber, const std::string &errMsg);
    virtual void buildInt(Int_t::value_type value);
    virtual void buildMethodCall(const char* methodName, unsigned int size );
    virtual void buildMethodCall(const std::string &methodName);
    virtual void buildString(const char* data, unsigned int size );
    virtual void buildString(const std::string &data) {
        return buildString(data.data(), data.size());
    }
    virtual void buildStructMember(const char *memberName, unsigned int size );
    virtual void buildStructMember(const std::string &memberName);
    virtual void closeArray();
    virtual void closeStruct();
    virtual void openArray(unsigned int numOfItems);
    virtual void openStruct(unsigned int numOfMembers);
    inline  void setError()
    {
        Py_XDECREF(retValue);
        retValue = 0;
        first = false;
        error = true;
    }
    inline bool isError()
    {
        return error;
    }
    inline bool isFirst( PyObject* value )
    {
        if(isError())
            return false;
        
        if(first)
        {
            Py_XDECREF(retValue);
            retValue = value;
            first = false;
            return true;
        }
        return false;
    }
    inline bool isMember( PyObject* value )
    {
        
        
        if(isError())
            return false;
    
        if(entityStorage.size() < 1)
            return false;
        switch(entityStorage.back().type)
        {
        case ARRAY:
            {
                if(PyList_Append(entityStorage.back().container, value) !=0)
                {
                    setError();
                    Py_DECREF(value);
                    return false;
                }
                Py_DECREF(value);
                //entityStorage.back().numOfItems--;
            }
            break;
        case STRUCT:
            {

                bool utf8 = false;
                for(long i = 0; i < memberName.size(); i++)
                {
                    if(memberName[i] & 0x80)
                    {
                        utf8 = true;
                        break;
                    }
                }

                if(utf8)
                {
                    PyObject *pyMemberName =
                        PyUnicode_DecodeUTF8(memberName.data(),
                                             memberName.size(), "strict");
                    if(pyMemberName == 0x00)
                    {
                        setError();
                        Py_DECREF(value);
                        return false;
                    }

                    if(PyDict_SetItem(entityStorage.back().container,
                                      pyMemberName, value) !=0 )

                    {
                        setError();
                        Py_DECREF(value);
                        return false;
                    }
                    Py_DECREF(value);
                }
                else
                {
                    if(PyDict_SetItemString(entityStorage.back().container,
                                            const_cast<char*>(memberName.c_str()),
                                            value) !=0 )
                    {
                        setError();
                        Py_DECREF(value);
                        return false;
                    }
                    Py_DECREF(value);
                }
                //entityStorage.back().numOfItems--;
            }
            break;
        default:
            //OOPS
            break;

        }
        /*if(entityStorage.back().numOfItems == 0)
        entityStorage.pop_back();*/
        return true;
    }

    inline PyObject* getUnMarshaledData() {
        if (methodName) return PySequence_Tuple(retValue);

        Py_XINCREF(retValue);
        return retValue;
    }

    inline std::string* getStringMethodName() const {
        return methodName;
    }

    inline PyObject* getMethodName() const {
        if (!methodName) {
            Py_INCREF(Py_None);
            return Py_None;
        }

        // return method
        return PyString_FromStringAndSize(methodName->data(),
                                          methodName->size());
    }

private :
    bool first;
    bool error;
    PyObject *retValue;
    std::string memberName;
    std::vector<TypeStorage_t> entityStorage;
    PyObject *methodObject;
    std::string *methodName;
    StringMode_t stringMode;
};

} } // namespace FRPC::Python

#endif // PYTHONBUILDER_H_
