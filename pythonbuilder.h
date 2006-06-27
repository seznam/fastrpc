/*
 * FastRPC - RPC protocol suport Binary and XML.
 * Copyright (C) 2005 Seznam.cz, a.s.
 *
 * $Id: pythonbuilder.h,v 1.2 2006-06-27 12:04:06 vasek Exp $
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
    enum{NONE=0, INT=1,BOOL,DOUBLE,STRING,DATETIME,BINARY,
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
    virtual void buildBinary(const char* data, long size);
    virtual void buildBinary(const std::string &data);
    virtual void buildBool(bool value);
    virtual void buildDateTime(short year, char month, char day,char hour, char
                               min, char sec,
                               char weekDay, time_t unixTime, char timeZone);
    virtual void buildDouble(double value);
    virtual void buildFault(long errNumber, const char* errMsg, long size );
    virtual void buildFault(long errNumber, const std::string &errMsg);
    virtual void buildInt(long value);
    virtual void buildMethodCall(const char* methodName, long size );
    virtual void buildMethodCall(const std::string &methodName);
    virtual void buildString(const char* data, long size );
    virtual void buildString(const std::string &data) {
        return buildString(data.data(), data.size());
    }
    virtual void buildStructMember(const char *memberName, long size );
    virtual void buildStructMember(const std::string &memberName);
    virtual void closeArray();
    virtual void closeStruct();
    virtual void openArray(long numOfItems);
    virtual void openStruct(long numOfMembers);
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
