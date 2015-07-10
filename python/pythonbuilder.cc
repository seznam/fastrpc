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
 * $Id: pythonbuilder.cc,v 1.9 2010-04-21 08:48:23 edois Exp $
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


#include "pythonbuilder.h"
#include "fastrpcmodule.h"
#include "frpcpythonhelper.h"

using namespace FRPC::Python;

namespace FRPC {
namespace Python {
StringMode_t parseStringMode(const char *stringMode) {
    if (stringMode) {
        if (!strcmp(stringMode, "string")) {
            return STRING_MODE_STRING;
        } else if (!strcmp(stringMode, "unicode")) {
            return STRING_MODE_UNICODE;
        } else if (strcmp(stringMode, "mixed")) {
            PyErr_Format(PyExc_ValueError,
                         "Invalid value '%s' of stringMode.", stringMode);
            return STRING_MODE_INVALID;
        }
    }

    // default
    return STRING_MODE_MIXED;
}

}
} // namespace FRPC::Python

void Builder_t::buildMethodResponse() {}
void Builder_t::buildBinary(const char* data, unsigned int size) {
    if (isError())
        return;
    PyObject *binary = reinterpret_cast<PyObject*>(newBinary(data, size));

    if (!binary)
        setError();

    if (!isMember(binary))
        isFirst(binary);

}
void Builder_t::buildBinary(const std::string &data) {
    if (isError())
        return;
    PyObject *binary = reinterpret_cast<PyObject*>(newBinary(data.data(),
                       data.size()));
    if (!binary)
        setError();

    if (!isMember(binary))
        isFirst(binary);
}
void Builder_t::buildBool(bool value) {
    if (isError())
        return;

    PyObject *boolean = 0;
    if ( nativeBoolean ) {
        boolean = value ? Py_True : Py_False;
        Py_INCREF(boolean);
    } else {
        boolean = reinterpret_cast<PyObject*>(newBoolean(value));
    }

    if (!boolean)
        setError();

    if (!isMember(boolean))
        isFirst(boolean);
}
void Builder_t::buildNull()
{
    Py_INCREF(Py_None);
    if (!isMember(Py_None))
        isFirst(Py_None);
}
void Builder_t::buildDateTime(short year, char month, char day, char hour, char
                              min, char sec, char weekDay, time_t unixTime,
                              int timeZone) {
    if (isError())
        return;

    PyObject *dateTime = 0;

    if ( datetimeBuilder != 0 && PyCallable_Check(datetimeBuilder) ) {
        dateTime = PyObject_CallFunction(datetimeBuilder,"(hbbbbbbii)", year,
                                         month, day, hour, min, sec, weekDay,
                                         unixTime, timeZone);
    } else {
        dateTime = reinterpret_cast<PyObject*>(newDateTime(year, month,
                         day, hour, min,
                         sec, weekDay, unixTime, timeZone) );
    }

    if (!dateTime)
        setError();

    if (!isMember(dateTime))
        isFirst(dateTime);
}
void Builder_t::buildDouble(double value) {
    if (isError())
        return;
    PyObject *doubleVal = PyFloat_FromDouble(value);

    if (!doubleVal)
        setError();

    if (!isMember(doubleVal))
        isFirst(doubleVal);

}
void Builder_t::buildFault(int errNumber, const char* errMsg, unsigned int size) {
    if (isError())
        return;
    PyObject *args =Py_BuildValue("iNO",errNumber,
                                  PyUnicode_FromStringAndSize(errMsg,size),
                                  methodObject ? methodObject : Py_None);
    PyObject *fault = PyObject_Call(Fault, args, 0);
    //isFirst(fault);

    if (!fault)
        setError();

    Py_XDECREF(retValue);
    retValue = fault;
    first = true;

}
void Builder_t::buildFault(int errNumber, const std::string &errMsg) {
    if (isError())
        return;
    PyObject *args =Py_BuildValue("isO", errNumber, errMsg.c_str(),
                                  methodObject ? methodObject : Py_None);
    PyObject *fault = PyObject_Call(Fault, args, 0);

    if (!fault)
        setError();

    Py_XDECREF(retValue);
    retValue = fault;
    first = true;
}
void Builder_t::buildInt(Int_t::value_type value) {
    if (isError())
        return;
    Int_t::value_type absValue = value < 0 ? -value :value;
    PyObject *integer = 0;

    if ((absValue & INT31_MASK)) {

        integer = PyLong_FromLongLong(value);
    }
    else {
        integer = PyLong_FromLong((long)value);
    }
    if (!integer)
        setError();

    if (!isMember(integer))
        isFirst(integer);

}
void Builder_t::buildMethodCall(const char* methodName1, unsigned int size) {
    if (methodName) delete methodName;
    methodName = new std::string(methodName1, size);

    PyObject *params = PyList_New(0);
    if (!params) setError();

    isFirst(params);
    entityStorage.push_back(TypeStorage_t(params, ARRAY));
}

void Builder_t::buildMethodCall(const std::string &methodName1) {
    if (methodName) delete methodName;
    methodName = new std::string(methodName1);

    PyObject *params = PyList_New(0);
    if (!params) setError();

    isFirst(params);
    entityStorage.push_back(TypeStorage_t(params, ARRAY));
}

void Builder_t::buildString(const char* data, unsigned int size) {
    if (isError())
        return;
    bool utf8 = (stringMode == STRING_MODE_UNICODE);

    // check 8-bit string only iff mixed
    if (stringMode == STRING_MODE_MIXED) {
        for (long i = 0; i < size; i++) {
            if (data[i] & 0x80) {
                utf8 = true;
                break;
            }
        }
    }

    PyObject *stringVal;

    if (utf8) {
        stringVal = PyUnicode_DecodeUTF8(data, size, "strict");
    } else {
        stringVal = PyUnicode_FromStringAndSize(const_cast<char*>(data), size);
    }

    if (!stringVal)
        setError();

    if (!isMember(stringVal))
        isFirst(stringVal);
}

void Builder_t::buildStructMember(const char *memberName, unsigned int size ) {
    if (isError())
        return;
    std::string buf(memberName,size);
    this->memberName =  buf;

}
void Builder_t::buildStructMember(const std::string &memberName) {
    if (isError())
        return;
    this->memberName = memberName;
}
void Builder_t::closeArray() {
    if (isError())
        return;
    entityStorage.pop_back();
}
void Builder_t::closeStruct() {
    if (isError())
        return;
    entityStorage.pop_back();
}
void Builder_t::openArray(unsigned int) {
    if (isError())
        return;
    PyObject *array = PyList_New(0);

    if (!array)
        setError();

    if (!isMember(array))
        isFirst(array);

    entityStorage.push_back(TypeStorage_t(array,ARRAY));
}
void Builder_t::openStruct(unsigned int) {

    if (isError())
        return;
    PyObject *structVal = PyDict_New();

    if (!structVal)
        setError();

    if (!isMember(structVal))
        isFirst(structVal);

    entityStorage.push_back(TypeStorage_t(structVal,STRUCT));
}

namespace {
class VirtBuilder_t : public BuilderInterface_t {
public:
    VirtBuilder_t(PyObject *methodObject, FRPC::Python::StringMode_t smode) :
        b(methodObject, smode)
    {}

    FRPC::DataBuilder_t * builder() {
        return &b;
    }

    PyObject * getRetValue() {
        return b.getRetValue();
    }

private:
    Builder_t b;
};
}

extern "C" {

    FRPC::Python::BuilderInterface_t *
    create_frpc_python_builder(PyObject *methodObject, FRPC::Python::StringMode_t smode) {
        return new VirtBuilder_t(methodObject, smode);
    }

    void destroy_frpc_python_builder(FRPC::Python::BuilderInterface_t *builder) {
        delete builder;
    }

} // extern "C"

