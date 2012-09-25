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
 * $Id: pythonfeeder.cc,v 1.12 2011-02-16 12:04:07 volca Exp $
 *
 * AUTHOR      Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * DESCRIPTION
 * Python FastRPC support. Python data feeder.
 *
 * HISTORY
 *      2006-05-24 (vasek)
 *              Created
 */

// Included first to get rid of the _POSIX_C_SOURCE warning
#include <Python.h>

#include <frpcint.h>
#include <frpcbinmarshaller.h>
#include <frpcxmlmarshaller.h>
#include "pythonfeeder.h"
#include "fastrpcmodule.h"
#include "frpcpythonhelper.h"

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

namespace {
class VirtFeeder_t : public FRPC::Python::FeederInterface_t {
public:
    VirtFeeder_t(FRPC::Marshaller_t *marshaller, const std::string &encoding) :
        feeder(marshaller, encoding)
    {}

    void feed(PyObject *args) { feeder.feed(args); }
    void feedValue(PyObject *value) { feeder.feedValue(value); }
private:
    FRPC::Python::Feeder_t feeder;
};
} // namespace

using FRPC::Int_t;

using namespace FRPC::Python;

namespace {

long getLongAttr(PyObject *object, char *name) {
    PyObjectWrapper_t attr(PyObject_GetAttrString(object, name));
    if (!attr) throw PyError_t();

    long value = PyInt_AsLong(attr);
    if ((value == -1) && PyErr_Occurred()) throw PyError_t();

    return value;
}

}

void Feeder_t::feed(PyObject *args)
{

    if (PyList_Check(args)) {
        int argc = PyList_GET_SIZE(args);

        for (int pos = 0; pos < argc; ++pos)
            feedValue(PyList_GET_ITEM(args, pos));

    } else if (PyTuple_Check(args)) {
        int argc = PyTuple_GET_SIZE(args);

         for (int pos = 0; pos < argc; ++pos)
           feedValue(PyTuple_GET_ITEM(args, pos));
    }


}

void Feeder_t::feedValue(PyObject *value)
{
#if PYTHON_API_VERSION >= 1012
    if (PyBool_Check(value)) {
        marshaller->packBool(PyObject_IsTrue(value));
    } else
#endif
    if (PyInt_Check(value)) {
        marshaller->packInt(PyInt_AsLong(value));
    } else if(PyLong_Check(value)) {
        size_t bits = _PyLong_NumBits(value);
        int sign = _PyLong_Sign(value);
        Int_t::value_type i;
        if (bits == 64 && sign == 1)
            i = PyLong_AsUnsignedLongLong(value);
        else
            i = PyLong_AsLongLong(value);
               
        // check for error
        if (PyErr_Occurred()) throw PyError_t();
        marshaller->packInt(i);
    } else if (PyFloat_Check(value)) {
        marshaller->packDouble(PyFloat_AsDouble(value));
    } else if (PyDateTime_Check(value)) {
        DateTimeObject *dateTime = reinterpret_cast<DateTimeObject*>(value);
        marshaller->packDateTime(dateTime->year, dateTime->month, dateTime->day,
                                 dateTime->hour, dateTime->min, dateTime->sec,
                                 dateTime->weekDay, dateTime->unixTime,
                                 dateTime->timeZone);
    } else if (PyBinary_Check(value)) {
        BinaryObject *bin = reinterpret_cast<BinaryObject*>(value);

        char *str;
        Py_ssize_t strLen;
        if (PyString_AsStringAndSize(bin->value, &str, &strLen))
            throw PyError_t();

        marshaller->packBinary(str, strLen);
    } else if (PyBoolean_Check(value)) {
        BooleanObject *boolean = reinterpret_cast<BooleanObject*>(value);
        marshaller->packBool(boolean->value == Py_True);
    } else if (PyString_Check(value)) {
        //is utf8 ?
        if (encoding == "utf-8")  {
            // get string and marshall it
            char *str;
            Py_ssize_t strLen;
            if (PyString_AsStringAndSize(value, &str, &strLen))
                throw PyError_t();

            marshaller->packString(str, strLen);
        } else {
            PyObjectWrapper_t unicode
                (PyUnicode_FromEncodedObject(value, encoding.c_str(), "strict"));
            if (!unicode) throw PyError_t();

            // convert unicode to utf-8
            PyObjectWrapper_t utf8(PyUnicode_AsUTF8String(unicode));
            if (!utf8) throw PyError_t();

            // get string and marshall it
            char *str;
            Py_ssize_t strLen;
            if (PyString_AsStringAndSize(utf8, &str, &strLen))
                throw PyError_t();

            marshaller->packString(str, strLen);
        }
    } else if (PyUnicode_Check(value)) {
        // convert unicode to utf-8
        PyObjectWrapper_t utf8(PyUnicode_AsUTF8String(value));
        if (!utf8) throw PyError_t();

        // get string and marshall it
        char *str;
        Py_ssize_t strLen;
        if (PyString_AsStringAndSize(utf8, &str, &strLen))
            throw PyError_t();

        marshaller->packString(str, strLen);
    } else if (PyList_Check(value)) {
        int argc = PyList_GET_SIZE(value);

        marshaller->packArray(argc);

        for (int pos = 0; pos < argc; ++pos)
            feedValue(PyList_GET_ITEM(value, pos));
    } else if (PyTuple_Check(value)) {
        int argc = PyTuple_GET_SIZE(value);

        marshaller->packArray(argc);

        for (int pos = 0; pos < argc; ++pos)
            feedValue(PyTuple_GET_ITEM(value, pos));
    } else if (PyDict_Check(value)) {
        Py_ssize_t argc = PyDict_Size(value);
        Py_ssize_t pos = 0;
        PyObject *key, *member;

        marshaller->packStruct(argc);

        while (PyDict_Next(value, &pos, &key, &member)) {
            if(!PyString_Check(key)) {
                PyErr_SetString(PyExc_TypeError,
                                "Key in Dictionary must be string.");
                throw PyError_t();
            }

            char *str;
            Py_ssize_t strLen;
            if (PyString_AsStringAndSize(key, &str, &strLen))
                throw PyError_t();

            marshaller->packStructMember(str, strLen);
            feedValue(member);
        }
    } else if (mxDateTime && (PyObject_IsInstance(value, mxDateTime) == 1)) {

        marshaller->packDateTime(getLongAttr(value, (char *)"year"),
                                 getLongAttr(value, (char *)"month"),
                                 getLongAttr(value, (char *)"day"),
                                 getLongAttr(value, (char *)"hour"),
                                 getLongAttr(value, (char *)"minute"),
                                 getLongAttr(value, (char *)"second"),
                                 -1, -1, -1);

    } else if (dateTimeDateTime
            && (PyObject_IsInstance(value, dateTimeDateTime) == 1)) {

        marshaller->packDateTime(getLongAttr(value, (char *)"year"),
                                 getLongAttr(value, (char *)"month"),
                                 getLongAttr(value, (char *)"day"),
                                 getLongAttr(value, (char *)"hour"),
                                 getLongAttr(value, (char *)"minute"),
                                 getLongAttr(value, (char *)"second"),
                                 -1, -1, -1);

    } else if (value == Py_None) {
        FRPC::BinMarshaller_t *binMarshaller(dynamic_cast<FRPC::BinMarshaller_t*>(marshaller));
        if (binMarshaller) {
            binMarshaller->packNull();
        } else {
            dynamic_cast<FRPC::XmlMarshaller_t&>(*marshaller).packNull();
        }
    } else {

        std::string objectRepr = "unknown";
        if (PyObjectWrapper_t repr = PyObject_Repr(value)) {
            objectRepr.assign(PyString_AsString(repr));
        }

        PyErr_Format(PyExc_TypeError, "Unknown type to marshall for object: %s",
                                      objectRepr.c_str());
        throw PyError_t();
    }
}

extern "C"  {

FRPC::Python::FeederInterface_t * create_frpc_python_feeder(FRPC::Marshaller_t *marshaller,const std::string *encoding) {
    return new VirtFeeder_t(marshaller, *encoding);
}

void destroy_frpc_python_feeder(FRPC::Python::FeederInterface_t *feeder) {
    delete feeder;
}

} // extern "C"



