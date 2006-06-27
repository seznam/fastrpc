/*
 * FastRPC - RPC protocol suport Binary and XML.
 * Copyright (C) 2005 Seznam.cz, a.s.
 *
 * $Id: fastrpcmodule.h,v 1.2 2006-06-27 12:04:06 vasek Exp $
 *
 * AUTHOR      Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * DESCRIPTION
 * Python FastRPC support. Global definitions.
 *
 * HISTORY
 *      2006-05-22 (vasek)
 *              Created
 */

#ifndef PYFASTRPCMODULE_H_
#define PYFASTRPCMODULE_H_

#include <Python.h>

#include "pyobjectwrapper.h"

namespace FRPC { namespace Python {

    extern PyObject *Error;
    extern PyObject *ProtocolError;
    extern PyObject *Fault;
    extern PyObject *ResponseError;

    int initErrors(PyObject *fastrpc_module);
    int initServer(PyObject *fastrpc_module);

    PyObject* errorRepr(PyObject *self, const std::string &name,
                        char *status, char *statusMessage);

    class PyError_t {
    public:
        PyError_t() {}
        ~PyError_t() {}
    };

    extern PyTypeObject BinaryObject_Type;

    struct BinaryObject {
        PyObject_HEAD                   /* python standard */
        PyObject *value;                /* true/false value */
    };

    BinaryObject* newBinary(const char* data, long size);


    extern PyTypeObject BooleanObject_Type;

    struct BooleanObject {
        PyObject_HEAD                   /* python standard */
        PyObject *value;                /* true/false value */
    };

    BooleanObject* newBoolean(bool value);


    extern PyTypeObject DateTimeObject_Type;

    struct DateTimeObject {
        PyObject_HEAD                   /* python standard */
        short year;        ///year
        char month;        ///month
        char day;          ///day
        char hour;         ///hour
        char min;          ///minute
        char sec;          /// second
        char weekDay;      ///day of week
        time_t        unixTime;     ///long unix time
        char          timeZone;     ///time zone
    };

    DateTimeObject* newDateTime(short year, char month, char day,
                                char hour, char in, char sec, char weekDay,
                                time_t unixTime, char timeZone);

#define PyDateTime_Check(op) PyObject_TypeCheck(op, &DateTimeObject_Type)
#define PyDateTime_CheckExact(op) ((op)->ob_type == &DateTimeObject_Type)
#define PyBinary_Check(op) PyObject_TypeCheck(op, &BinaryObject_Type)
#define PyBinary_CheckExact(op) ((op)->ob_type == &BinaryObject_Type)
#define PyBoolean_Check(op) PyObject_TypeCheck(op, &BooleanObject_Type)
#define PyBoolean_CheckExact(op) ((op)->ob_type == &BooleanObject_Type)
#define PyFault_Check(op) PyObject_TypeCheck(op, &Fault)

} } // namespace FRPC::Python

#endif // PYFASTRPCMODULE_H_
