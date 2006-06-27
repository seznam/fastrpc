/*
 * FastRPC - RPC protocol suport Binary and XML.
 * Copyright (C) 2005 Seznam.cz, a.s.
 *
 * $Id: pythonfeeder.cc,v 1.2 2006-06-27 12:04:06 vasek Exp $
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


#include "pythonfeeder.h"
#include "fastrpcmodule.h"


using namespace FRPC::Python;

void Feeder_t::feed(PyObject *args)
{
    int argc = PyTuple_GET_SIZE(args);

    for (int pos = 0; pos < argc; ++pos)
        feedValue(PyTuple_GET_ITEM(args, pos));
}

void Feeder_t::feedValue(PyObject *value)
{
    if (PyInt_Check(value)) {
        marshaller->packInt(PyInt_AsLong(value));
    } else if(PyLong_Check(value)) {
        marshaller->packInt(PyLong_AsLong(value));
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
        int strLen;
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
            int strLen;
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
            int strLen;
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
        int strLen;
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
        int argc = PyDict_Size(value);
        int pos = 0;
        PyObject *key, *member;

        marshaller->packStruct(argc);

        while (PyDict_Next(value, &pos, &key, &member)) {
            if(!PyString_Check(key)) {
                PyErr_SetString(PyExc_TypeError,
                                "Key in Dictionary must be string.");
                throw PyError_t();
            }

            char *str;
            int strLen;
            if (PyString_AsStringAndSize(key, &str, &strLen))
                throw PyError_t();

            marshaller->packStructMember(str, strLen);
            feedValue(member);
        }
    } else {
        PyErr_SetString(PyExc_TypeError,"Unknown type to marshall");
        throw PyError_t();
    }
}
