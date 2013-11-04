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
 * $Id: fastrpcmodule.h,v 1.7 2008-11-14 10:18:22 burlog Exp $
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
                        const char *status, const char *statusMessage);

    class PyError_t {
    public:
        PyError_t() {}
        ~PyError_t() {}
    };

    extern PyTypeObject BinaryObject_Type;

    struct BinaryObject {
        PyObject_HEAD                   /* python standard */
        PyObject *value;                /* string value */
    };

    BinaryObject* newBinary(const char* data, long size);


    extern PyTypeObject BooleanObject_Type;

    struct BooleanObject {
        PyObject_HEAD                   /* python standard */
        PyObject *value;                /* true/false value */
    };

    BooleanObject* newBoolean(bool value);


    extern PyTypeObject DateTimeObject_Type;
    extern PyTypeObject LocalTimeObject_Type;
    extern PyTypeObject UTCTimeObject_Type;

    struct DateTimeObject {
        PyObject_HEAD                   /* python standard */
        short year;        ///year
        char month;        ///month
        char day;          ///day
        char hour;         ///hour
        char min;          ///minute
        char sec;          /// second
        char weekDay;      ///day of week
        time_t unixTime;   ///long unix time
        int timeZone;      ///time zone
    };

    DateTimeObject* newDateTime(short year, char month, char day,
                                char hour, char in, char sec, char weekDay,
                                time_t unixTime, int timeZone);

    extern PyObject *mxDateTime;
    extern PyObject *dateTimeDateTime;

#define PyDateTime_Check(op) (PyObject_TypeCheck(op, &DateTimeObject_Type) \
                           || PyObject_TypeCheck(op, &LocalTimeObject_Type) \
                           || PyObject_TypeCheck(op, &UTCTimeObject_Type))
#define PyDateTime_CheckExact(op) ((op)->ob_type == &DateTimeObject_Type)
#define PyBinary_Check(op) PyObject_TypeCheck(op, &BinaryObject_Type)
#define PyBinary_CheckExact(op) ((op)->ob_type == &BinaryObject_Type)
#define PyBoolean_Check(op) PyObject_TypeCheck(op, &BooleanObject_Type)
#define PyBoolean_CheckExact(op) ((op)->ob_type == &BooleanObject_Type)
#define PyFault_Check(op) PyObject_TypeCheck(op, &Fault)

} } // namespace FRPC::Python

#endif // PYFASTRPCMODULE_H_
