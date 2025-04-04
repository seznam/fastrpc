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
 * $Id: fastrpcmodule.cc,v 1.28 2011-02-16 12:04:07 volca Exp $
 *
 * AUTHOR      Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *      2005-02-16 (miro)
 *              Method displatching (ServerProxy + Method classes).
 */

#define __ENABLE_WSTRING
// defined here to indicate # args to PyArgs_ParseTuple use Py_ssize_t instead of int
#define PY_SSIZE_T_CLEAN

// Included first to get rid of the _POSIX_C_SOURCE warning
#include <Python.h>

#include <new>
#include <string>
#include <vector>
#include <memory>
#include <sstream>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <frpcmarshaller.h>
#include <frpcwriter.h>
#include <frpcunmarshaller.h>
#include <frpchttpclient.h>
#include <frpchttpio.h>
#include <frpcerror.h>
#include <frpcprotocolerror.h>
#include <frpcencodingerror.h>
#include <frpctypeerror.h>
#include <frpchttperror.h>
#include <frpcstreamerror.h>
#include <frpclenerror.h>
#include <frpcresponseerror.h>
#include <frpc.h>

#include "fastrpcmodule.h"
#include "pythonbuilder.h"
#include "pythonfeeder.h"

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#define PyErr_WarnEx(A, B, C) PyErr_Warn(A, B)
#endif

using FRPC::Int_t;
using FRPC::Marshaller_t;
using FRPC::UnMarshaller_t;
using FRPC::Writer_t;
using FRPC::HTTPClient_t;
using FRPC::URL_t;
using FRPC::HTTPIO_t;
using FRPC::Error_t;
using FRPC::ProtocolError_t;
using FRPC::StreamError_t;
using FRPC::HTTPError_t;
using FRPC::TypeError_t;
using FRPC::EncodingError_t;
using FRPC::LenError_t;
using FRPC::ResponseError_t;
using FRPC::ProtocolVersion_t;
using FRPC::Connector_t;
using FRPC::SimpleConnectorIPv6_t;

using namespace FRPC::Python;

PyObject *FRPC::Python::mxDateTime = 0;
PyObject *FRPC::Python::dateTimeDateTime = 0;

// support constants
#ifdef HAVE_BINARY
static PyObject *emptyBinary = 0;
#endif

/**************************************************************************/
/* Python DateTime declaration                                            */
/**************************************************************************/

extern "C"
{
    static PyObject* DateTimeObject_new(PyTypeObject *self, PyObject *args,
                                        PyObject *kwds);
    static int DateTimeObject_init(DateTimeObject *self, PyObject *args,
                                   PyObject *kwds);
    static int LocalTimeObject_init(DateTimeObject *self, PyObject *args,
                                   PyObject *kwds);
    static int UTCTimeObject_init(DateTimeObject *self, PyObject *args,
                                   PyObject *kwds);
    static void DateTimeObject_dealloc(DateTimeObject *self);
    static PyObject* DateTimeObject_richcmp(PyObject *self, PyObject *other, int op);
    static PyObject* DateTimeObject_repr(DateTimeObject *self);
    static PyObject* DateTimeObject_getattr(DateTimeObject *self, char *name);
    static int DateTimeObject_setattr(DateTimeObject *self, char *name, PyObject
                                      *value);
}

namespace FRPC { namespace Python {
PyTypeObject DateTimeObject_Type =
    {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "DateTime",
        sizeof(DateTimeObject),
        0,
        (destructor)DateTimeObject_dealloc,       /* tp_dealloc */
        0,                                      /* tp_print */
        (getattrfunc)DateTimeObject_getattr,      /* tp_getattr */
        (setattrfunc)DateTimeObject_setattr,      /* tp_setattr */
        0,                                      /* tp_compare */
        (reprfunc)DateTimeObject_repr,            /* tp_repr */
        0,                                      /* tp_as_newDateTime->weekDay
                                                                =number */
        0,                                      /* tp_as_sequence */
        0,                                      /* tp_as_mapping */
        0,                                      /* tp_hash */
        0,                                      /* tp_call */
        0,                                      /* tp_str */
        0,                                      /* tp_getattro */
        0,                                      /* tp_setattro */
        0,                                      /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                     /* tp_flags */
        0,                                      /* tp_doc */
        0,                                      /* tp_traverse */
        0,                                      /* tp_clear */
        DateTimeObject_richcmp,                 /* tp_richcompare */
        0,                                      /* tp_weaklistoffset */
        0,                                      /* tp_iter */
        0,                                      /* tp_iternext */
        0,                                      /* tp_methods */
        0,                                      /* tp_members */
        0,                                      /* tp_getset */
        0,                                      /* tp_base */
        0,                                      /* tp_dict */
        0,                                      /* tp_descr_get */
        0,                                      /* tp_descr_set */
        0,                                      /* tp_dictoffset */
        (initproc)DateTimeObject_init,          /* tp_init */
        PyType_GenericAlloc,                    /* tp_alloc */
        DateTimeObject_new,                     /* tp_new */
        PyObject_Free                           /* tp_free */
    };

PyTypeObject LocalTimeObject_Type =
    {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "LocalTime",
        sizeof(DateTimeObject),
        0,
        (destructor)DateTimeObject_dealloc,       /* tp_dealloc */
        0,                                      /* tp_print */
        (getattrfunc)DateTimeObject_getattr,      /* tp_getattr */
        (setattrfunc)DateTimeObject_setattr,      /* tp_setattr */
        0,                                      /* tp_compare */
        (reprfunc)DateTimeObject_repr,            /* tp_repr */
        0,                                      /* tp_as_newDateTime->weekDay
                                                                =number */
        0,                                      /* tp_as_sequence */
        0,                                      /* tp_as_mapping */
        0,                                      /* tp_hash */
        0,                                      /* tp_call */
        0,                                      /* tp_str */
        0,                                      /* tp_getattro */
        0,                                      /* tp_setattro */
        0,                                      /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                     /* tp_flags */
        0,                                      /* tp_doc */
        0,                                      /* tp_traverse */
        0,                                      /* tp_clear */
        DateTimeObject_richcmp,                 /* tp_richcompare */
        0,                                      /* tp_weaklistoffset */
        0,                                      /* tp_iter */
        0,                                      /* tp_iternext */
        0,                                      /* tp_methods */
        0,                                      /* tp_members */
        0,                                      /* tp_getset */
        0,                                      /* tp_base */
        0,                                      /* tp_dict */
        0,                                      /* tp_descr_get */
        0,                                      /* tp_descr_set */
        0,                                      /* tp_dictoffset */
        (initproc)LocalTimeObject_init,         /* tp_init */
        PyType_GenericAlloc,                    /* tp_alloc */
        DateTimeObject_new,                     /* tp_new */
        PyObject_Free                           /* tp_free */
    };

PyTypeObject UTCTimeObject_Type =
    {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "UTCTime",
        sizeof(DateTimeObject),
        0,
        (destructor)DateTimeObject_dealloc,       /* tp_dealloc */
        0,                                      /* tp_print */
        (getattrfunc)DateTimeObject_getattr,      /* tp_getattr */
        (setattrfunc)DateTimeObject_setattr,      /* tp_setattr */
        0,                                      /* tp_compare */
        (reprfunc)DateTimeObject_repr,            /* tp_repr */
        0,                                      /* tp_as_newDateTime->weekDay
                                                                =number */
        0,                                      /* tp_as_sequence */
        0,                                      /* tp_as_mapping */
        0,                                      /* tp_hash */
        0,                                      /* tp_call */
        0,                                      /* tp_str */
        0,                                      /* tp_getattro */
        0,                                      /* tp_setattro */
        0,                                      /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                     /* tp_flags */
        0,                                      /* tp_doc */
        0,                                      /* tp_traverse */
        0,                                      /* tp_clear */
        DateTimeObject_richcmp,                 /* tp_richcompare */
        0,                                      /* tp_weaklistoffset */
        0,                                      /* tp_iter */
        0,                                      /* tp_iternext */
        0,                                      /* tp_methods */
        0,                                      /* tp_members */
        0,                                      /* tp_getset */
        0,                                      /* tp_base */
        0,                                      /* tp_dict */
        0,                                      /* tp_descr_get */
        0,                                      /* tp_descr_set */
        0,                                      /* tp_dictoffset */
        (initproc)UTCTimeObject_init,           /* tp_init */
        PyType_GenericAlloc,                    /* tp_alloc */
        DateTimeObject_new,                     /* tp_new */
        PyObject_Free                           /* tp_free */
    };


} } // namespace FRPC::Python

PyObject* DateTimeObject_new(PyTypeObject *type, PyObject *, PyObject *)
{
    // allocate memory
    assert(type && type->tp_alloc);
    DateTimeObject *self =
        reinterpret_cast<DateTimeObject*>(type->tp_alloc(type, 0));
    if (!self)
        return 0;

    // fill defaults (what if __init__ doesn't get called
    self->year = 0;
    self->unixTime = 0;
    self->month = self->day = self->hour = self->min = self->sec
                = self->weekDay = self->timeZone = 0;

    return reinterpret_cast<PyObject*>(self);
}

int TimeObject_init_getNow(DateTimeObject *self, bool nowShift) {
    self->unixTime = time(0);

    tm *time_tm = localtime(&self->unixTime);

    if (nowShift) {
#ifdef HAVE_ALTZONE
        self->unixTime += (time_tm->tm_isdst > 0)?
            ::altzone: ::timezone;
#else
        self->unixTime += (time_tm->tm_isdst > 0)?
            ::timezone - 3600: ::timezone;
#endif
        time_tm = localtime(&self->unixTime);
    }

    self->year = time_tm->tm_year + 1900;
    self->month = time_tm->tm_mon + 1;
    self->day = time_tm->tm_mday;
    self->hour = time_tm->tm_hour;
    self->min = time_tm->tm_min;
    self->sec = time_tm->tm_sec;
    self->weekDay = time_tm->tm_wday;

    return 0;
}

int TimeObject_init_parseInt(DateTimeObject *self, PyObject *pyValue) {
    self->unixTime = PyInt_AsLong(pyValue);

    tm *time_tm = localtime(&self->unixTime);

    self->year = time_tm->tm_year + 1900;
    self->month = time_tm->tm_mon + 1;
    self->day = time_tm->tm_mday;
    self->hour = time_tm->tm_hour;
    self->min = time_tm->tm_min;
    self->sec = time_tm->tm_sec;
    self->weekDay = time_tm->tm_wday;

    return 0;
}

int TimeObject_init_parseString(DateTimeObject *self, PyObject *pyValue) {
    self->timeZone = 0;
    self->weekDay = -1;

    // date vars
    short year;
    int timeZone;
    char month, day, hour, min, sec;

    Py_ssize_t len;
    PyStrDataType_t data;
    STR_ASSTRANDSIZE(pyValue, data, len) {
        return 0;
    }

    try {

        FRPC::parseISODateTime(data, len, year, month, day, hour,
                               min, sec, timeZone);

    } catch(const StreamError_t &e) {
        PyErr_Format(PyExc_Exception, "%s", e.message().c_str());
        return -1;
    }
    //error, extra characters on line

    self->year = year;
    self->month = month;
    self->day = day;
    self->hour = hour;
    self->min = min;
    self->sec = sec;
    self->timeZone = timeZone;

    tm time_tm;
    memset(reinterpret_cast<void*>(&time_tm), 0, sizeof(tm));
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon  = month -1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = min;
    time_tm.tm_sec = sec;
    time_tm.tm_isdst = -1;
    self->unixTime = mktime(&time_tm);

    if(self->unixTime != -1) {
        tm *ntime_tm = localtime(&(self->unixTime));
        self->weekDay = ntime_tm->tm_wday;

    } else
        self->weekDay = -1;

    return 0;
}

int DateTimeObject_init(DateTimeObject *self, PyObject *args, PyObject *)
{
    PyObject *pyValue = 0;
    int timeZone = 0;

    // probably int and timezone
    if (PyArg_ParseTuple(args, "Oi", &pyValue, &timeZone)) {
        int result = TimeObject_init_parseInt(self, pyValue);
        self->timeZone = timeZone;
        return result;
    }

    // clear error and try parse single object
    PyErr_Clear();
    if (!PyArg_ParseTuple(args, "|O", &pyValue))
        return -1;

#if PY_MAJOR_VERSION == 2
    if (!pyValue) {
#warning remove this possibility in next major revision of fastrpc
        PyErr_WarnEx(PyExc_DeprecationWarning,
               "Deprecated call use LocalTime() or UTCtime() instead.", 1);
        return LocalTimeObject_init(self, args, NULL);

    } else if (PyString_Check(pyValue)) {
        return TimeObject_init_parseString(self, pyValue);

    } else if (PyInt_Check(pyValue)) {
#warning remove this possibility in next major revision of fastrpc
        PyErr_WarnEx(PyExc_DeprecationWarning,
               "Deprecated call use LocalTime(int) or UTCtime(int) instead.", 1);
        return LocalTimeObject_init(self, args, NULL);

    } else
#endif
    if (pyValue && PyUnicode_Check(pyValue)) {
#if PY_MAJOR_VERSION == 2
        PyObjectWrapper_t utf(PyUnicode_AsUTF8String(pyValue));
        if (utf.get()) {
            TimeObject_init_parseString(self, utf.get());
        }
#else
        return TimeObject_init_parseString(self, pyValue);
#endif
    } else
    {
        PyErr_SetString(PyExc_TypeError,
            "DateTime expects either a string argument or two integers");
        return -1;
    }

    return 0;
}

int LocalTimeObject_init(DateTimeObject *self, PyObject *args, PyObject *)
{
    PyObject *pyValue = 0;
    if(!PyArg_ParseTuple(args, "|O", &pyValue))
        return -1;

    int result = 0;
    if (!pyValue) {
        result = TimeObject_init_getNow(self, false);

    } else if (PyInt_Check(pyValue)) {
        result = TimeObject_init_parseInt(self, pyValue);

    } else {
        PyErr_Format(PyExc_TypeError, "Argument must be timestamp");
        return -1;
    }

    // get local timezone
    struct tm *time_tm = localtime(&self->unixTime);
#ifdef HAVE_ALTZONE
    self->timeZone = (time_tm->tm_isdst > 0)? ::altzone: ::timezone;
#else
    self->timeZone = (time_tm->tm_isdst > 0)? ::timezone - 3600: ::timezone;
#endif
    return result;
}

int UTCTimeObject_init(DateTimeObject *self, PyObject *args, PyObject *)
{
    PyObject *pyValue = 0;
    if(!PyArg_ParseTuple(args, "|O", &pyValue))
        return -1;

    int result = 0;
    if (!pyValue) {
        result = TimeObject_init_getNow(self, true);

    } else if (PyInt_Check(pyValue)) {
        result = TimeObject_init_parseInt(self, pyValue);

    } else {
        PyErr_Format(PyExc_TypeError, "Argument must be timestamp");
        return -1;
    }

    // set UTC timezone
    self->timeZone = 0;
    return result;
}

void DateTimeObject_dealloc(DateTimeObject *self)
{
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject *>(self));
}

PyObject* DateTimeObject_repr(DateTimeObject *self)
{
    std::string  dateTime = FRPC::getISODateTime(self->year, self->month,
                                           self->day, self->hour, self->min,
                                           self->sec, self->timeZone);

    return PyUnicode_FromString(dateTime.c_str());

}

PyObject* DateTimeObject_getattr(DateTimeObject *self, char *name)
{

    if (!strcmp(name, "year"))
    {
        return PyInt_FromLong(self->year);
    }

    if (!strcmp(name, "month"))
    {
        return PyInt_FromLong(self->month);
    }
    if (!strcmp(name, "day"))
    {
        return PyInt_FromLong(self->day);
    }

    if (!strcmp(name, "hour"))
    {
        return PyInt_FromLong(self->hour);
    }

    if (!strcmp(name, "min"))
    {
        return PyInt_FromLong(self->min);
    }
    if (!strcmp(name, "sec"))
    {
        return PyInt_FromLong(self->sec);
    }
    if (!strcmp(name, "weekDay"))
    {
        return PyInt_FromLong(self->weekDay);
    }
    if (!strcmp(name, "unixTime"))
    {
        return PyInt_FromLong(self->unixTime);
    }
    if (!strcmp(name, "timeZone"))
    {
        return PyInt_FromLong(self->timeZone);
    }
    if (!strcmp(name, "value"))
    {

        std::string  dateTime = FRPC::getISODateTime(self->year
                                               ,self->month ,
                                               self->day ,self->hour,self->min,self->sec,
                                               self->timeZone);



        return PyUnicode_FromString(dateTime.c_str());
    }

    // not found
    return PyErr_Format(PyExc_AttributeError,
                        "DateTime object has no such attribute '%s'.", name);
}
int DateTimeObject_setattr(DateTimeObject *self, char *name, PyObject *value)
{
    if(!PyInt_Check(value))
    {
        PyErr_Format(PyExc_AttributeError,
                     "New attribute must be int");
        return -1;
    }

    if (!strcmp(name, "year"))
    {
        self->year = PyInt_AsLong(value);
        return 0;
    }

    if (!strcmp(name, "month"))
    {
        self->month = PyInt_AsLong(value);
        return 0;
    }
    if (!strcmp(name, "day"))
    {
        self->day = PyInt_AsLong(value);
        return 0;
    }

    if (!strcmp(name, "hour"))
    {
        self->hour = PyInt_AsLong(value);
        return 0;
    }

    if (!strcmp(name, "min"))
    {
        self->min = PyInt_AsLong(value);
        return 0;
    }
    if (!strcmp(name, "sec"))
    {
        self->sec = PyInt_AsLong(value);
        return 0;
    }
    if (!strcmp(name, "weekDay"))
    {
        self->weekDay = PyInt_AsLong(value);
        return 0;
    }
    if (!strcmp(name, "unixTime"))
    {
        self->unixTime = PyInt_AsLong(value);
        return 0;
    }
    if (!strcmp(name, "timeZone"))
    {
        self->timeZone = PyInt_AsLong(value);
        return 0;
    }


    // not found
    PyErr_Format(PyExc_AttributeError,
                 "DateTime object has no such attribute '%s'.", name);
    return -1;
}

time_t DateTimeObject_to_timestamp(DateTimeObject *datetime) {
    struct tm timeinfo = {0};

    timeinfo.tm_year = datetime->year - 1900;
    timeinfo.tm_mon = datetime->month - 1;
    timeinfo.tm_mday = datetime->day;
    timeinfo.tm_hour = datetime->hour;
    timeinfo.tm_min = datetime->min;
    timeinfo.tm_sec = datetime->sec;
    return mktime(&timeinfo) - datetime->timeZone;
}


static PyObject* DateTimeObject_richcmp(PyObject *self, PyObject *other, int op)
{
    if (!PyObject_TypeCheck(self, &DateTimeObject_Type) &&
        !PyObject_TypeCheck(self, &LocalTimeObject_Type) &&
        !PyObject_TypeCheck(self, &UTCTimeObject_Type))
    {
        // allow for eq/neq comparability of datetime objects against any other
        // type
        if (op == Py_EQ)
            Py_RETURN_FALSE;
        if (op == Py_NE)
            Py_RETURN_TRUE;

        PyErr_Format(PyExc_TypeError, "arg #1 is not a DateTime, it is %s",
                            Py_TYPE(self)->tp_name);
        return NULL;
    }

    if (!PyObject_TypeCheck(other, &DateTimeObject_Type) &&
        !PyObject_TypeCheck(other, &LocalTimeObject_Type) &&
        !PyObject_TypeCheck(other, &UTCTimeObject_Type))
    {
        if (op == Py_EQ)
            Py_RETURN_FALSE;
        if (op == Py_NE)
            Py_RETURN_TRUE;

        PyErr_Format(PyExc_TypeError, "arg #2 is not a DateTime, it is %s",
                            Py_TYPE(other)->tp_name);
        return NULL;
    }

    time_t self_timestemp = DateTimeObject_to_timestamp(
        reinterpret_cast<DateTimeObject*>(self));
    time_t other_timestemp = DateTimeObject_to_timestamp(
        reinterpret_cast<DateTimeObject*>(other));

    PyObject *result;
    int c;
    switch (op) {
        case Py_LT: c = self_timestemp <  other_timestemp; break;
        case Py_LE: c = self_timestemp <= other_timestemp; break;
        case Py_EQ: c = self_timestemp == other_timestemp; break;
        case Py_NE: c = self_timestemp != other_timestemp; break;
        case Py_GT: c = self_timestemp >  other_timestemp; break;
        case Py_GE: c = self_timestemp >= other_timestemp; break;
    }

    result = c ? Py_True : Py_False;
    Py_INCREF(result);
    return result;
}

namespace FRPC { namespace Python {

DateTimeObject* newDateTime(short year, char month, char day, char hour, char
                            min, char sec,
                            char weekDay, time_t unixTime, int timeZone)
{
    DateTimeObject *self = PyObject_NEW(DateTimeObject, &DateTimeObject_Type);
    if (self == NULL)
        return NULL;
    self->year = year;
    self->month = month;
    self->day = day;
    self->hour = hour;
    self->min = min;
    self->sec = sec;
    self->weekDay = weekDay;
    self->unixTime = unixTime;
    self->timeZone = timeZone;
    if(unixTime != -1)
       return self;
    //compute unix timestamp and weekay
    tm time_tm;
    memset(reinterpret_cast<void*>(&time_tm), 0, sizeof(tm));
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon  = month -1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = min;
    time_tm.tm_sec = sec;
    time_tm.tm_isdst = -1;
    self->unixTime = mktime(&time_tm);

    if(self->unixTime != -1)
    {
        tm *ntime_tm = localtime(&(self->unixTime));
        self->weekDay = ntime_tm->tm_wday;
    }
    else
        self->weekDay = -1;


    return self;
}

} } // namespace FRPC::Python

#ifdef HAVE_BINARY
extern "C"
{
    static PyObject* BinaryObject_new(PyTypeObject *self, PyObject *args,
                                      PyObject *kwds);
    static int BinaryObject_init(BinaryObject *self, PyObject *args,
                                 PyObject *kwds);
    static void BinaryObject_dealloc(BinaryObject *self);
    static PyObject* BinaryObject_repr(BinaryObject *self);
    static PyObject* BinaryObject_getattr(BinaryObject *self, char *name);
    static int BinaryObject_setattr(BinaryObject *self, char *name, PyObject
                                    *value);
}

namespace FRPC { namespace Python {

PyTypeObject BinaryObject_Type =
    {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "Binary",
        sizeof(BinaryObject),
        0,
        (destructor)BinaryObject_dealloc,       /* tp_dealloc */
        0,                                      /* tp_print */
        (getattrfunc)BinaryObject_getattr,      /* tp_getattr */
        (setattrfunc)BinaryObject_setattr,      /* tp_setattr */
        0,                                      /* tp_compare */
        (reprfunc)BinaryObject_repr,            /* tp_repr */
        0,                                      /* tp_as_number */
        0,                                      /* tp_as_sequence */
        0,                                      /* tp_as_mapping */
        0,                                      /* tp_hash */
        0,                                      /* tp_call */
        0,                                      /* tp_str */
        0,                                      /* tp_getattro */
        0,                                      /* tp_setattro */
        0,                                      /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                     /* tp_flags */
        0,                                      /* tp_doc */
        0,                                      /* tp_traverse */
        0,                                      /* tp_clear */
        0,                                      /* tp_richcompare */
        0,                                      /* tp_weaklistoffset */
        0,                                      /* tp_iter */
        0,                                      /* tp_iternext */
        0,                                      /* tp_methods */
        0,                                      /* tp_members */
        0,                                      /* tp_getset */
        0,                                      /* tp_base */
        0,                                      /* tp_dict */
        0,                                      /* tp_descr_get */
        0,                                      /* tp_descr_set */
        0,                                      /* tp_dictoffset */
        (initproc)BinaryObject_init,            /* tp_init */
        PyType_GenericAlloc,                    /* tp_alloc */
        BinaryObject_new,                       /* tp_new */
        PyObject_Free                           /* tp_free */
    };

} } // namespace FRPC::Python

PyObject* BinaryObject_new(PyTypeObject *type, PyObject *, PyObject *)
{
    // allocate memory
    assert(type && type->tp_alloc);
    BinaryObject *self =
        reinterpret_cast<BinaryObject*>(type->tp_alloc(type, 0));
    if (!self)
        return 0;

    // fill defaults (what if __init__ doesn't get called
    self->value = emptyBinary;
    Py_INCREF(self->value);
    return reinterpret_cast<PyObject*>(self);
}

int BinaryObject_init(BinaryObject *self, PyObject *args, PyObject *)
{
    PyObject *pyValue = 0;

#if PY_MAJOR_VERSION >= 3
    if (!PyArg_ParseTuple(args,"|O!", &PyBytes_Type, &pyValue))
#else
    if (!PyArg_ParseTuple(args,"|O!", &PyString_Type, &pyValue))
#endif
        return -1;

    if (pyValue)
    {
        Py_XDECREF(self->value);
        self->value = pyValue;

        Py_INCREF(pyValue);
    }

    return 0;
}

void BinaryObject_dealloc(BinaryObject *self)
{
    Py_XDECREF(self->value);
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject *>(self));
}

PyObject* BinaryObject_repr(BinaryObject *self)
{

    return PyObject_Repr(self->value);

}

PyObject* BinaryObject_getattr(BinaryObject *self, char *name)
{

    if (!strcmp(name, "data"))
    {
        Py_INCREF(self->value);
        return self->value;
    }

    // not found
    return PyErr_Format(PyExc_AttributeError,
                        "Binary object has no such attribute '%s'.", name);
}
int BinaryObject_setattr(BinaryObject *self, char *name, PyObject *value)
{
    if (!strcmp(name, "data"))
    {
        if(value == 0 )
        {
            PyErr_Format(PyExc_AttributeError,
                         "Can't set empty object");
            return -1;
        }
        if(PyString_Check(value))
        {
            Py_DECREF(self->value);
            self ->value  = value;
            Py_INCREF(value);

        }
        else
        {
            PyErr_Format(PyExc_AttributeError,
                         "New object must be string");
            return -1;
        }
        return 0;
    }

    // not found
    PyErr_Format(PyExc_AttributeError,
                 "Binary object has no such attribute '%s'.", name);
    return -1;
}

namespace FRPC { namespace Python {

BinaryObject* newBinary(const char* data, long size)
{
    BinaryObject *self = PyObject_NEW(BinaryObject, &BinaryObject_Type);
    if (self == NULL)
        return NULL;
    self->value = PyString_FromStringAndSize(data, size);
    return self;
}

} } // namespace FRPC::Python
#endif

/**************************************************************************/
/* Python Bool declaration                                                */
/**************************************************************************/


extern "C"
{
    static PyObject* BooleanObject_new(PyTypeObject *self, PyObject *args,
                                       PyObject *kwds);
    static int BooleanObject_init(BooleanObject *self, PyObject *args,
                                  PyObject *kwds);
    static void BooleanObject_dealloc(BooleanObject *self);
    static PyObject* BooleanObject_richcmp(PyObject *self, PyObject *other, int op);
    static PyObject* BooleanObject_repr(BooleanObject *self);
    static PyObject* BooleanObject_str(BooleanObject *self);
    static PyObject* BooleanObject_getattr(BooleanObject *self, char *name);
    static int BooleanObject_setattr(BooleanObject *self, char *name, PyObject*
                                     value);
    static int BooleanObject_nonzero(BooleanObject *self);
    static PyObject* BooleanObject_int(BooleanObject *self);
}

static PyNumberMethods Boolean_AsNumber = {
            0,                                  /* nb_add */
            0,                                  /* nb_subtract */
            0,                                  /* nb_multiply */
#if PY_MAJOR_VERSION == 2
            0,                                  /* nb_divide */
#endif
            0,                                  /* nb_remainder */
            0,                                  /* nb_divmod */
            0,                                  /* nb_power */
            0,                                  /* nb_negative */
            0,                                  /* nb_positive */
            0,                                  /* nb_absolute */
            (inquiry)BooleanObject_nonzero,     /* nb_nonzero / nb_bool */
            0,                                  /* nb_invert */
            0,                                  /* nb_lshift */
            0,                                  /* nb_rshift */
            0,                                  /* nb_and */
            0,                                  /* nb_xor */
            0,                                  /* nb_or */
#if PY_MAJOR_VERSION == 2
            0,                                  /* nb_coerce */
#endif
            (unaryfunc)BooleanObject_int,       /* nb_int */
            0,                                  /* nb_long / nb_reserved */
            0,                                  /* nb_float */
#if PY_MAJOR_VERSION == 2
            0,                                  /* nb_oct */
            0,                                  /* nb_hex */
#endif
            0,                                  /* nb_inplace_add */
            0,                                  /* nb_inplace_subtract */
            0,                                  /* nb_inplace_multiply */
#if PY_MAJOR_VERSION == 2
            0,                                  /* nb_inplace_divide */
#endif
            0,                                  /* nb_inplace_remainder */
            0,                                  /* nb_inplace_power */
            0,                                  /* nb_inplace_lshift */
            0,                                  /* nb_inplace_rshift */
            0,                                  /* nb_inplace_and */
            0,                                  /* nb_inplace_xor */
            0,                                  /* nb_inplace_or */
            0,                                  /* nb_floor_divide */
            0,                                  /* nb_true_divide */
            0,                                  /* nb_inplace_floor_divide */
            0,                                  /* nb_inplace_true_divide */
            0                                   /* nb_index */
        };


namespace FRPC { namespace Python {

/*
 * map characterstics of a boolean
 */
PyTypeObject BooleanObject_Type =
    {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "Boolean",
        sizeof(BooleanObject),
        0,
        (destructor)BooleanObject_dealloc,      /* tp_dealloc */
        0,                                      /* tp_print */
        (getattrfunc)BooleanObject_getattr,     /* tp_getattr */
        (setattrfunc)BooleanObject_setattr,     /* tp_setattr */
        0,                                      /* tp_compare */
        (reprfunc)BooleanObject_repr,           /* tp_repr */
        &Boolean_AsNumber,                      /* tp_as_number */
        0,                                      /* tp_as_sequence */
        0,                                      /* tp_as_mapping */
        0,                                      /* tp_hash */
        0,                                      /* tp_call */
        (reprfunc)BooleanObject_str,            /* tp_str */
        0,                                      /* tp_getattro */
        0,                                      /* tp_setattro */
        0,                                      /* tp_as_buffer */
        Py_TPFLAGS_DEFAULT,                     /* tp_flags */
        0,                                      /* tp_doc */
        0,                                      /* tp_traverse */
        0,                                      /* tp_clear */
        BooleanObject_richcmp,                  /* tp_richcompare */
        0,                                      /* tp_weaklistoffset */
        0,                                      /* tp_iter */
        0,                                      /* tp_iternext */
        0,                                      /* tp_methods */
        0,                                      /* tp_members */
        0,                                      /* tp_getset */
        0,                                      /* tp_base */
        0,                                      /* tp_dict */
        0,                                      /* tp_descr_get */
        0,                                      /* tp_descr_set */
        0,                                      /* tp_dictoffset */
        (initproc)BooleanObject_init,           /* tp_init */
        PyType_GenericAlloc,                    /* tp_alloc */
        BooleanObject_new,                      /* tp_new */
        PyObject_Free                           /* tp_free */
    };


} } // namespace FRPC::Python

/*
 * free resources associated with a boolean object
 */
void BooleanObject_dealloc(BooleanObject *self)
{
    Py_XDECREF(self->value);
    Py_TYPE(self)->tp_free(reinterpret_cast<PyObject *>(self));
}

PyObject* BooleanObject_new(PyTypeObject *type, PyObject *, PyObject *)
{
    // allocate memory
    assert(type && type->tp_alloc);
    BooleanObject *self =
        reinterpret_cast<BooleanObject*>(type->tp_alloc(type, 0));
    if (!self)
        return 0;

    // fill defaults (what if __init__ doesn't get called
    self->value = Py_False;
    Py_INCREF(self->value);
    return reinterpret_cast<PyObject*>(self);
}

int BooleanObject_init(BooleanObject *self, PyObject *args, PyObject *)
{
    // parse arguments
    PyObject *pyvalue = 0;
    if (!PyArg_ParseTuple(args, "|O:fastrpc.Boolean", &pyvalue))
        return -1;

    // default to false
    if (!pyvalue)
    {
        self->value = Py_False;
        return 0;
    }

    // determine boolean value of object
    int value = PyObject_IsTrue(pyvalue);
    if (value == -1)
        return -1;

    self->value = (value) ? Py_True : Py_False;
    Py_INCREF(self->value);

    return 0;
}

PyObject* BooleanObject_getattr(BooleanObject *self, char *name)
{
    if (!strcmp(name, "value"))
    {
        Py_INCREF(self->value);
        return self->value;
    }

    // not found
    return PyErr_Format(PyExc_AttributeError,
                        "Boolean object has no such attribute '%s'.", name);
}

int BooleanObject_setattr(BooleanObject *self, char *name, PyObject* value)
{
    if (!strcmp(name, "value"))
    {
        int val =  PyObject_IsTrue(value);
        if(val == -1)
            return -1;
        if((self->value == Py_True && val == 0 )||(self->value == Py_False &&
                val != 0) )
        {
            Py_DECREF(self->value);
            self->value = (val)?Py_True:Py_False;
            Py_INCREF(self->value);

        }

        return 0;
    }
    // not found
    PyErr_Format(PyExc_AttributeError,
                 "Boolean object has no such attribute '%s'.", name);
    return -1;

}

/*
 * boolean comparison
 */
static PyObject *
BooleanObject_richcmp(PyObject *self, PyObject *other, int op)
{
    int self_value  = 0;
    int other_value = 0;

    if (PyObject_TypeCheck(self, &BooleanObject_Type)) {
        self_value = reinterpret_cast<BooleanObject*>(self)->value == Py_True ? 1 : 0;
    } else if (PyBool_Check(self)) {
        self_value = (self == Py_True) ? 1 : 0;
    } else {
        // non-boolean type. Only == and != work, and should just say values
        // differ all the time.
        if (op == Py_EQ)
            Py_RETURN_FALSE;
        if (op == Py_NE)
            Py_RETURN_TRUE;

        PyErr_Format(PyExc_TypeError, "arg #1 is not Boolean, it's %.100s()",
                     self->ob_type->tp_name);
        return NULL;
    }

    if (PyObject_TypeCheck(other, &BooleanObject_Type)) {
        other_value = reinterpret_cast<BooleanObject*>(other)->value == Py_True ? 1 : 0;
    } else if (PyBool_Check(other)) {
        other_value = (other == Py_True) ? 1 : 0;
    } else {
        if (op == Py_EQ)
            Py_RETURN_FALSE;
        if (op == Py_NE)
            Py_RETURN_TRUE;

        PyErr_Format(PyExc_TypeError, "arg #2 is not a Boolean it's %.100s()",
                     other->ob_type->tp_name);
        return NULL;
    }

    int c;
    switch (op) {
        case Py_LT: c = self_value <  other_value; break;
        case Py_LE: c = self_value <= other_value; break;
        case Py_EQ: c = self_value == other_value; break;
        case Py_NE: c = self_value != other_value; break;
        case Py_GT: c = self_value >  other_value; break;
        case Py_GE: c = self_value >= other_value; break;
    }

    PyObject *result = c ? Py_True : Py_False;
    Py_INCREF(result);
    return result;
}


/*
 * represent a boolean  object
 */
PyObject* BooleanObject_repr(BooleanObject *self)
{
    return PyUnicode_FromString(PyObject_IsTrue(self->value)? "Boolean: TRUE" :
                               "Boolean: FALSE");
}

PyObject* BooleanObject_str(BooleanObject *self)
{
    return PyUnicode_FromString(PyObject_IsTrue(self->value) ? "1" : "0");
}

int BooleanObject_nonzero(BooleanObject *self)
{
    // check for true
    return self->value == Py_True;
}

PyObject* BooleanObject_int(BooleanObject *self)
{
    return PyInt_FromLong(self->value == Py_True);
}

namespace FRPC { namespace Python {

BooleanObject* newBoolean(bool value)
{
    BooleanObject *self = PyObject_NEW(BooleanObject, &BooleanObject_Type);
    if (self == NULL)
        return NULL;

    self->value = (value)?Py_True:Py_False;

    Py_INCREF(self->value);
    return self;
}

} } // namespace FRPC::Python



/**************************************************************************/
/***Writer for marshaller                                               ***/
/**************************************************************************/
namespace
{
class OutBuffer_t:public Writer_t
{
public:
    OutBuffer_t():Writer_t()
    {}
    ~OutBuffer_t()
    {}
    virtual void write(const char *data, unsigned int size );
    virtual void flush();

    inline const char* data()
    {
        return localBuffer.data();
    }

    inline unsigned int size()
    {
        return localBuffer.size();
    }

    inline const char* c_str()
    {
        return localBuffer.c_str();
    }

private:

    std::string localBuffer;

};


}

void OutBuffer_t::write(const char *data, unsigned int size )
{
    localBuffer.append(data, size);
}

void OutBuffer_t::flush()
{}


/**************************************************************************/
/*** Server proxy object declaration                                    ***/
/**************************************************************************/
namespace
{
// forward
struct MethodObject;

class Proxy_t
{
public:
    Proxy_t(const std::string &serverUrl, int readTimeout,
            int writeTimeout, int connectTimeout, bool keepAlive,
            int rpcTransferMode, const std::string &encoding,
            bool useHTTP10, bool useChunks,
            const std::string &proxyVia, StringMode_t stringMode,
            const ProtocolVersion_t &protocolVersion, int nativeBoolean,
            PyObject *datetimeBuilder, PyObject *preCall, PyObject *postCall)
        : url(serverUrl, proxyVia),
          io(-1, readTimeout, writeTimeout, -1, -1),
          rpcTransferMode(rpcTransferMode), encoding(encoding),
          serverSupportedProtocols(HTTPClient_t::XML_RPC),
          useHTTP10(useHTTP10), useChunks(useChunks),
          stringMode(stringMode),
          protocolVersion(protocolVersion),
          connector(new SimpleConnectorIPv6_t(url, connectTimeout, keepAlive)),
          nativeBoolean(nativeBoolean), datetimeBuilder(datetimeBuilder),
          preCall(preCall), postCall(postCall), allowSurrogates(false)
    {}

    ~Proxy_t() {
        Py_XDECREF(datetimeBuilder);
        Py_XDECREF(preCall);
        Py_XDECREF(postCall);
    }

    enum {
        BINARY_ON_SUPPORT_ON_KEEP_ALIVE = 0,
        BINARY_ON_SUPPORT,
        BINARY_ALWAYS,
        BINARY_NEVER,
    };

    PyObject* operator()(MethodObject *methodObject, PyObject *args);

    const URL_t& getURL() const {
        return url;
    }

    const std::string& getLastCall() const {
        return lastCall;
    }

    /**
     * @short Closes (keep alive) connection to server
     */
    void closeConnection() {
        int &fd = io.socket();
        if (fd > -1) {
            TEMP_FAILURE_RETRY(::close(fd));
            fd = -1;
        }
    }

    typedef std::pair<std::string, std::string> Header_t;

    void addRequestHttpHeader(const Header_t& header) {
        headers.push_back(header);
    }

    void addRequestHttpHeaderForCall(const Header_t& header) {
        headersForCall.push_back(header);
    }

    void enableSurrogatePass() { allowSurrogates = true; }

private:
    URL_t url;
    HTTPIO_t io;
    int readTimeout;
    int writeTimeout;
    int rpcTransferMode;
    std::string encoding;

    std::string path;
    unsigned int serverSupportedProtocols;

    bool useHTTP10;
    bool useChunks;
    std::string lastCall;
    StringMode_t stringMode;
    ProtocolVersion_t protocolVersion;
    std::auto_ptr<Connector_t> connector;
    int nativeBoolean;
    PyObject *datetimeBuilder;
    PyObject *preCall;
    PyObject *postCall;

    typedef std::vector<Header_t> Headers_t;

    Headers_t headersForCall;
    Headers_t headers;

    bool allowSurrogates;
};

struct ServerProxyObject
{
    PyObject_HEAD

    Proxy_t proxy;
    bool proxyOk;
    bool hideAttributes;

};
}

extern "C"
{
    static PyObject* ServerProxy_ServerProxy(ServerProxyObject *self,
            PyObject *args, PyObject *keywds);

    static void ServerProxy_dealloc(ServerProxyObject *self);
    static PyObject* ServerProxy_getattr(ServerProxyObject *self, char *name);
    static PyObject* ServerProxy_call(ServerProxyObject *self, PyObject *args, PyObject *kw);

};

static PyTypeObject ServerProxy_Type =
    {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "ServerProxy",                     /*tp_name*/
        sizeof (ServerProxyObject),        /*tp_basicsize*/
        0,                                 /*tp_itemsize*/
        /* methods */
        (destructor) ServerProxy_dealloc,  /*tp_dealloc*/
        0,                                 /*tp_print*/
        (getattrfunc) ServerProxy_getattr, /*tp_getattr*/
        0,                                 /*tp_setattr*/
        0,                                 /*tp_compare*/
        0,                                 /*tp_repr*/
        0,                                 /*tp_as_number*/
        0,                                 /*tp_as_sequence*/
        0,                                 /*tp_as_mapping*/
        0,                                 /*tp_hash*/
        (ternaryfunc) ServerProxy_call,   /* tp_call */
        0,                                 /* tp_str */
        0,                                 /* tp_getattro */
        0,                                 /* tp_setattro */
        0,                                 /* tp_as_buffer */
        0,                                 /* tp_flags */
        0,                                 /* tp_doc */
    };

/**************************************************************************/
/*** Method object declaration                                          ***/
/**************************************************************************/

namespace
{
struct MethodObject
{
    PyObject_HEAD

    /** Associated proxy. */
    ServerProxyObject *proxy;

    /** Name of method */
    std::string name;
};
};

extern "C"
{
    static void Method_dealloc(MethodObject *self);

    static PyObject* Method_getattr(MethodObject *self, char *name);

    static PyObject* Method_call(MethodObject *self, PyObject *args,
                                 PyObject *keys);
};

static PyTypeObject Method_Type =
    {
        PyVarObject_HEAD_INIT(&PyType_Type, 0)
        "Method",                          /*tp_name*/
        sizeof (MethodObject),             /*tp_basicsize*/
        0,                                 /*tp_itemsize*/
        /* methods */
        (destructor) Method_dealloc,       /*tp_dealloc*/
        0,                                 /*tp_print*/
        (getattrfunc) Method_getattr,      /*tp_getattr*/
        0,                                 /*tp_setattr*/
        0,                                 /*tp_compare*/
        0,                                 /*tp_repr*/
        0,                                 /*tp_as_number*/
        0,                                 /*tp_as_sequence*/
        0,                                 /*tp_as_mapping*/
        0,                                 /*tp_hash*/
        (ternaryfunc) Method_call,         /* tp_call */
        0,                                 /* tp_str */
        0,                                 /* tp_getattro */
        0,                                 /* tp_setattro */
        0,                                 /* tp_as_buffer */
        0,                                 /* tp_flags */
        0,                                 /* tp_doc */
    };

/**************************************************************************/
/*** Method object implementation                                       ***/
/**************************************************************************/

namespace
{
PyObject* newMethod(ServerProxyObject *proxy, const std::string &name)
{
    // create method object
    MethodObject *method = PyObject_New(MethodObject, &Method_Type);
    if (!method)
        return 0;

    // remember proxy
    Py_INCREF(proxy);
    method->proxy = proxy;

    // initialize name (inplace!)
    new (&method->name) std::string(name);

    // OK
    return reinterpret_cast<PyObject*>(method);
}
}

void Method_dealloc(MethodObject *self)
{
    using std::string;

    // call destructor for name
    self->name.string::~string();

    // get rid of reference to proxy
    Py_DECREF(self->proxy);

    // destroy memory
    PyObject_Del(self);
}

PyObject* Method_getattr(MethodObject *self, char *name)
{
    if (!strncmp(name, "__", 2)) {
        if (!strcmp(name, "__dict__")) {
            PyObjectWrapper_t dict(PyDict_New());
            if (!dict) return 0;

            PyObjectWrapper_t methodName
                (PyUnicode_FromStringAndSize(self->name.data(),
                                            self->name.size()));
            if (!methodName) return 0;
            if (PyDict_SetItemString(dict, "method_name", methodName))
                return 0;

            // hold incremented reference to serverProxy
            PyObjectWrapper_t serverProxy
                (reinterpret_cast<PyObject*>(self->proxy), true);
            if (PyDict_SetItemString(dict, "server_proxy", serverProxy))
                return 0;

            return dict.inc();
        }
        return PyErr_Format(PyExc_AttributeError,
                            "Method object has no attributes '%s'",
                            name);
    }

    // return new method
    return newMethod(self->proxy, self->name + '.' + name);
}

namespace {

/** converts pyobject to string, returns true if succesful
 *  otherwise it sets PyErr and returns false.
 */
bool get_string(PyObject *s, std::string &tgt) {
#if PY_MAJOR_VERSION < 3
    if (PyString_Check(s)) {
        tgt = PyString_AsString(s);
        return true;
    }
#else
    if (PyUnicode_Check(s)) {
        Py_ssize_t len;
        const char * ptr = PyUnicode_AsUTF8AndSize(s, &len);
        tgt.assign(ptr, len);
        return true;
    }
#endif
    PyErr_Format(PyExc_TypeError,
                 "Expected string, found %s.",
                 Py_TYPE(s)->tp_name);
    return false;
}

// unpacks std::pair of strings from a python sequence object
bool unpack_string_pair(PyObject *o, std::pair<std::string, std::string> &res)
{
    if (!PySequence_Check(o))
    {
        PyErr_Format(PyExc_TypeError,
                     "String pair object must be a sequence, not %s.",
                     Py_TYPE(o)->tp_name);
        return false;
    }

    // iterate over the sequence, insert headers
    const Py_ssize_t len = PySequence_Size(o);

    if (len != 2) {
        PyErr_Format(PyExc_TypeError,
                     "String sequence does have %ld items, not 2.",
                     len);
        return false;
    }

    // parse the two strings
    std::string first, second;

    if (!get_string(PySequence_GetItem(o, 0), first))
        return false;

    if (!get_string(PySequence_GetItem(o, 1), second))
        return false;

    res = std::make_pair(first, second);
    return true;
}

/* feeds either list or tuple of 2 item string tuples into serverproxy object
 * as either session or call request headers
 */
bool feed_headers(PyObject *hdrs, Proxy_t &proxy, bool callScope) {
    // check the hdrs object is a sequence
    if (!PySequence_Check(hdrs))
    {
        PyErr_Format(PyExc_TypeError,
                     "headers object must be a sequence, not %s.",
                     Py_TYPE(hdrs)->tp_name);
        return false;
    }

    // iterate over the sequence, insert headers
    const Py_ssize_t len = PySequence_Size(hdrs);
    for (Py_ssize_t idx = 0; idx < len; ++idx) {
        PyObjectWrapper_t header(PySequence_GetItem(hdrs, idx));

        // is this a tuple? unpack two strings from this
        std::pair<std::string, std::string> hp;

        // unpack as a header pair, exit on error
        if (!unpack_string_pair(*header, hp))
            return false;

        if (callScope) {
            proxy.addRequestHttpHeaderForCall(hp);
        } else {
            proxy.addRequestHttpHeader(hp);
        }
    }
    return true;
}

} // namespace

PyObject* Method_call(MethodObject *self, PyObject *args, PyObject *keys)
{
    // check whether we are called with keywords and fail if so
    if (keys)
    {
        if (!PyDict_Check(keys)) {
            PyErr_SetString(PyExc_TypeError,
                        "Should not happen. KWDict is not a dict.");
            return 0;
        }

        PyObject *hdrs = PyDict_GetItemString(keys, "headers");

        if (hdrs) {
            // it should be list or tuple containing tuples of size 2
            // on error just bail out, it's already set via pyerr_ methods
            if (!feed_headers(hdrs, self->proxy->proxy, true))
                return 0;
        }
    }

    // make remote call
    return self->proxy->proxy(self, args);
}

/**************************************************************************/
/*** Server proxy object implementation                                 ***/
/**************************************************************************/

static char ServerProxy_ServerProxy__doc__[] =
    "Create new ServerProxy\n";

PyObject* ServerProxy_ServerProxy(ServerProxyObject *, PyObject *args,
                                  PyObject *keywds)
{
    static const char *kwlist[] = {"serverUrl", "readTimeout", "writeTimeout",
                                   "connectTimeout",
                                   "keepAlive", "useBinary","encoding",
                                   "useHTTP10", "useChunks",
                                   "proxyVia", "stringMode",
                                   "protocolVersionMajor",
                                   "protocolVersionMinor",
                                   "nativeBoolean", "datetimeBuilder",
                                   "preCall", "postCall", "hideAttributes",
                                   "headers", "allowSurrogates",
                                   0};

    // parse arguments
    PyStrDataType_t serverUrl;
    Py_ssize_t serverUrlLen;
    int readTimeout = -1;
    int writeTimeout = -1;
    int connectTimeout = -1;
    int keepAlive = 0;
    int mode = Proxy_t::BINARY_ON_SUPPORT_ON_KEEP_ALIVE;
    char *stringMode_ = 0;
    char *encoding = "utf-8";
    int useHTTP10 = false;
    int useChunks = true;
    char *proxyVia = "";
    int protocolVersionMajor = 2;
    int protocolVersionMinor = 1;
    PyObject *nativeBoolean = 0;
    PyObject *datetimeBuilder = 0;
    PyObject *preCall = 0;
    PyObject *postCall = 0;
    int hideAttributes = true;
    PyObject *headers = 0;
    PyObject *allowSurrogates = 0;

    static const char *kwtypes = "siiiiisiissiiOOO";
    const void *kwvars[] = { serverUrl, &readTimeout,
        &writeTimeout, &connectTimeout,
        &keepAlive, &mode, encoding,
        &useHTTP10, &useChunks, proxyVia, stringMode_,
        &protocolVersionMajor,
        &protocolVersionMinor,
        &nativeBoolean, &datetimeBuilder,
        &headers, NULL };

    // Normal initialization
    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "s#|iiiiisiissiiOOOOiOO:ServerProxy.__init__",
                                     (char **)kwlist,
                                     &serverUrl, &serverUrlLen, &readTimeout,
                                     &writeTimeout, &connectTimeout,
                                     &keepAlive, &mode, &encoding,
                                     &useHTTP10, &useChunks, &proxyVia, &stringMode_,
                                     &protocolVersionMajor,
                                     &protocolVersionMinor, &nativeBoolean,
                                     &datetimeBuilder, &preCall, &postCall,
                                     &hideAttributes, &headers,
                                     &allowSurrogates))
    {
        if (PyErr_Occurred()) {
            PyErr_Clear();
        }

        // Initialization from ConfigParser and section
        PyObject *cfgParser;
        char *cfgSection;
        Py_ssize_t cfgSectionLen;

        if (!PyArg_ParseTuple(args, "Os#:ServerProxy.__init__",
                &cfgParser, &cfgSection, &cfgSectionLen)) {
            return 0;
        }

        // serverUrl
        PyObjectWrapper_t value(PyObject_CallMethod(cfgParser, "get",
                    "s#s",
                    cfgSection, cfgSectionLen, "serverUrl"));
        if (!value) {
            return 0;
        }

        Py_ssize_t slen;
        STR_ASSTRANDSIZE(value.object, serverUrl, slen) {
            return 0;
        }
        serverUrlLen = slen;

        int i = 0;
        while (kwtypes[++i]) {
            char *method = NULL;
            switch (kwtypes[i]) {
                case 's': method = "get"; break;
                case 'i': method = "getint"; break;
                case 'f': method = "getfloat"; break;
                case 'b': method = "getboolean"; break;
                case 'O': // don't know how to initialize objects
                default:
                            method = NULL;
                            break;
            }
            if (method == NULL) continue;

            PyObjectWrapper_t value(PyObject_CallMethod(cfgParser, method,
                    "s#s",
                    cfgSection, cfgSectionLen, kwlist[i]));

            if (!value) {
                if (PyErr_ExceptionMatches(PyExc_ValueError)) {
                    return 0;
                }
                PyErr_Clear();
                continue;
            }

            switch (kwtypes[i]) {
                case 's':
#if PY_MAJOR_VERSION >= 3
                    Py_ssize_t len;
                    kwvars[i] = strdupa(PyUnicode_AsUTF8AndSize(value.object, &len));
                    if (PyErr_Occurred()){
                        return 0;
                    }
#else
                    kwvars[i] = strdupa(PyString_AsString(value.object));
                    if (PyErr_Occurred()){
                        return 0;
                    }
#endif
                    break;
                case 'i':
                    *(int*)kwvars[i] = PyLong_AsLong(value.object);
                    if (PyErr_Occurred()){
                        return 0;
                    }
                    break;
                case 'f':
                    *(float*)kwvars[i] = PyFloat_AsDouble(value.object);
                    if (PyErr_Occurred()) {
                        return 0;
                    }
                    break;
                case 'b':
                    *(bool*)kwvars[i] = PyObject_IsTrue(value.object);
                    break;
                default:
                    break;
            }
        }
    }

    if (preCall && !PyCallable_Check(preCall)) {
        PyErr_Format(PyExc_TypeError, "'%s' must be callable.", "preCall");
        return 0;
    }
    if (postCall && !PyCallable_Check(postCall)) {
        PyErr_Format(PyExc_TypeError, "'%s' must be callable.", "postCall");
        return 0;
    }

    // create server proxy object
    ServerProxyObject *proxy = PyObject_New(ServerProxyObject,
                                            &ServerProxy_Type);
    if (!proxy) return 0;

    proxy->proxyOk = false;
    proxy->hideAttributes = hideAttributes;

    StringMode_t stringMode = parseStringMode(stringMode_);
    if (stringMode == STRING_MODE_INVALID) return 0;

    if (protocolVersionMajor < 1) protocolVersionMajor = 1;
    if (protocolVersionMinor < 0) protocolVersionMinor = 0;

    Py_XINCREF(datetimeBuilder);
    Py_XINCREF(preCall);
    Py_XINCREF(postCall);
    try
    {
        // initialize underlying proxy (inplace!)
        new (&proxy->proxy) Proxy_t(std::string(serverUrl, serverUrlLen),
                                    readTimeout, writeTimeout,
                                    connectTimeout, keepAlive, mode, encoding,
                                    useHTTP10, useChunks, proxyVia, stringMode,
                                    ProtocolVersion_t(protocolVersionMajor,
                                            protocolVersionMinor),
                                    nativeBoolean != 0 ? PyObject_IsTrue(nativeBoolean) : true,
                                    datetimeBuilder, preCall, postCall);
        proxy->proxyOk = true;

        if (allowSurrogates) {
            if (PyObject_IsTrue(allowSurrogates))
                proxy->proxy.enableSurrogatePass();
        }

        if (headers)
            feed_headers(headers, proxy->proxy, /*callscope*/ false);
    }

    catch (const HTTPError_t &httpError)
    {
        Py_DECREF(proxy);
        PyObjectWrapper_t args(Py_BuildValue
            ("is", httpError.errorNum(), httpError.message().c_str()));
        if (!args) return 0;
        PyErr_SetObject(ProtocolError, args);
        return 0;
    }
    catch(const TypeError_t &typeError)
    {
        Py_DECREF(proxy);
        PyErr_SetString(PyExc_TypeError, typeError.message().c_str());
        return 0;
    }
    // wtf is there reason for that since its allocated inplace? (inplace!)
    catch(std::bad_alloc &badAlloc)
    {
        Py_DECREF(proxy);
        PyErr_SetString(PyExc_MemoryError,"Out of memory");
        return 0;
    }
    // OK
    return reinterpret_cast<PyObject*>(proxy);
}

void ServerProxy_dealloc(ServerProxyObject *self)
{
    // call destructor of proxy iff proxy constructed
    if (self->proxyOk) self->proxy.~Proxy_t();

    // destroy my memory
    PyObject_Del(self);
}

//! inspired by https://docs.python.org/2/library/xmlrpclib.html
PyObject* ServerProxy_call(ServerProxyObject *self, PyObject *args, PyObject *kw) {
    static char *kwlist[] = {"action", NULL};
    const char *action = 0;
    const char CLOSE_CONNECTION[] = "close_connection";
    const char GET_HOST[] = "get_host";
    const char GET_PORT[] = "get_port";
    const char GET_URL[] = "get_url";
    const char GET_PATH[] = "get_path";
    const char GET_LAST_CALL[] = "get_last_call";

    if (!PyArg_ParseTupleAndKeywords(args, kw, "s",  kwlist, &action))
        return 0;

    const URL_t &url = self->proxy.getURL();

    if (strcmp(action, CLOSE_CONNECTION) == 0) {
        if (self->proxyOk)
            self->proxy.closeConnection();
    } else if (!strcmp(action, GET_HOST)) {
        return PyUnicode_FromStringAndSize(url.host.data(), url.host.size());

    } else if (!strcmp(action, GET_PATH)) {
        return PyUnicode_FromStringAndSize(url.path.data(), url.path.size());

    } else if (!strcmp(action, GET_PORT)) {
        return PyInt_FromLong(url.port);

    } else if (!strcmp(action, GET_URL)) {
        std::string fullUrl(url.getUrl());
        return PyUnicode_FromStringAndSize(fullUrl.data(), fullUrl.size());

    } else if (!strcmp(action, GET_LAST_CALL)) {
        const std::string &lastCall(self->proxy.getLastCall());
        return PyUnicode_FromStringAndSize(lastCall.data(), lastCall.size());
    } else {
        PyErr_Format(PyExc_TypeError, "unknown action %s", action);
        return NULL;
    }

    Py_RETURN_NONE;
}

PyObject* ServerProxy_getattr(ServerProxyObject *self, char *name)
{

    // check for __dict__ before checking hideAttributes,
    // so __dict__ will be accessible regardless of hideAttributes
    const URL_t &url = self->proxy.getURL();
    if (!strncmp(name, "__", 2)) {
        if (!strcmp(name, "__dict__")) {

            PyObjectWrapper_t dict(PyDict_New());
            if (!dict) return 0;

            PyObjectWrapper_t host
                (PyUnicode_FromStringAndSize(url.host.data(), url.host.size()));
            if (!host) return 0;
            if (PyDict_SetItemString(dict, "host", host)) return 0;

            PyObjectWrapper_t path
                (PyUnicode_FromStringAndSize(url.path.data(), url.path.size()));
            if (!path) return 0;
            if (PyDict_SetItemString(dict, "path", path)) return 0;

            PyObjectWrapper_t port(PyInt_FromLong(url.port));
            if (!port) return 0;
            if (PyDict_SetItemString(dict, "port", port)) return 0;

            std::string fullUrl(url.getUrl());
            PyObjectWrapper_t pyfullUrl
                (PyUnicode_FromStringAndSize(fullUrl.data(), fullUrl.size()));
            if (!pyfullUrl) return 0;
            if (PyDict_SetItemString(dict, "url", pyfullUrl)) return 0;

            const std::string &lastCall(self->proxy.getLastCall());
            PyObjectWrapper_t pyLastCall
                (PyUnicode_FromStringAndSize(lastCall.data(), lastCall.size()));
            if (PyDict_SetItemString(dict, "last_call", pyLastCall)) return 0;

            return dict.inc();
        }
        return PyErr_Format(PyExc_AttributeError,
                            "ServerProxy object has no attributes '%s'",
                            name);
    }

    if (self->hideAttributes) {
        return newMethod(self, name);
    }

    if (!strcmp(name, "host")) {
        return PyUnicode_FromStringAndSize(url.host.data(), url.host.size());
    } else if (!strcmp(name, "path")) {
        return PyUnicode_FromStringAndSize(url.path.data(), url.path.size());
    } else if (!strcmp(name, "port")) {
        return PyInt_FromLong(url.port);
    } else if (!strcmp(name, "url")) {
        std::string fullUrl(url.getUrl());
        return PyUnicode_FromStringAndSize(fullUrl.data(), fullUrl.size());
    } else if (!strcmp(name, "last_call")) {
        const std::string &lastCall(self->proxy.getLastCall());
        return PyUnicode_FromStringAndSize(lastCall.data(), lastCall.size());
    }

    // return new method
    return newMethod(self, name);
}

PyObject* Proxy_t::operator()(MethodObject *methodObject, PyObject *args) {
    // remember last method
    lastCall = methodObject->name;

    HTTPClient_t client(io, url, connector.get(), useHTTP10, useChunks);
    Builder_t builder(reinterpret_cast<PyObject*>(methodObject),
                      stringMode, nativeBoolean, datetimeBuilder);

    if (allowSurrogates) builder.enableSurrogatePass();

    Marshaller_t *marshaller;

    {
        client.addCustomRequestHeader(headers);
        client.addCustomRequestHeader(headersForCall);
        headersForCall.clear();
    }

    switch(rpcTransferMode) {
    case BINARY_NEVER:
        //using XML_RPC
        marshaller= Marshaller_t::create(Marshaller_t::XML_RPC,client,
                                         protocolVersion);
        client.prepare(HTTPClient_t::XML_RPC);
        break;

    case BINARY_ON_SUPPORT:
        if(serverSupportedProtocols & HTTPClient_t::BINARY_RPC) {
            //using BINARY_RPC
            marshaller=
                Marshaller_t::create(Marshaller_t::BINARY_RPC,client,
                                     protocolVersion);
            client.prepare(HTTPClient_t::BINARY_RPC);
        } else {
            //using XML_RPC
            marshaller= Marshaller_t::create(Marshaller_t::XML_RPC,client,
                                             protocolVersion);
            client.prepare(HTTPClient_t::XML_RPC);
        }
        break;

    case BINARY_ALWAYS:
        //using BINARY_RPC  always
        marshaller= Marshaller_t::create(Marshaller_t::BINARY_RPC,client,
                                         protocolVersion);
        client.prepare(HTTPClient_t::BINARY_RPC);
        break;

    case BINARY_ON_SUPPORT_ON_KEEP_ALIVE:
    default:
        if(serverSupportedProtocols & HTTPClient_t::XML_RPC
           || connector->getKeepAlive() == false || io.socket() != -1) {
            //using XML_RPC
            marshaller= Marshaller_t::create(Marshaller_t::XML_RPC,client,
                                             protocolVersion);
            client.prepare(HTTPClient_t::XML_RPC);
        } else {
            //using BINARY_RPC
            marshaller=
                Marshaller_t::create(Marshaller_t::BINARY_RPC,client,
                                     protocolVersion);
            client.prepare(HTTPClient_t::BINARY_RPC);
        }
        break;
    }

    PyObject *callbackData = NULL;
    PyObject *errEx = NULL;
    PyObject *errArgs = NULL;
    try {

        // Pre-call hook
        if (preCall) {
            callbackData = PyObject_CallFunction(preCall, "sO",
                methodObject->name.c_str(), args, 0);
            if (PyErr_Occurred()) {
                // Ignore error
                PyErr_Clear();
            }
        }

        Feeder_t feeder(marshaller,encoding);
        try {
            marshaller->packMethodCall(methodObject->name.c_str());
            feeder.feed(args);
            marshaller->flush();
        } catch(const ResponseError_t &e) {
            // premature response occured => just ignore here
        }

        client.readResponse(builder);
        serverSupportedProtocols = client.getSupportedProtocols();
        protocolVersion = client.getProtocolVersion();

    }

    catch (const LenError_t &lenError) {
        errEx = PyExc_TypeError;
        Py_INCREF(errEx);
        errArgs = Py_BuildValue("s", lenError.message().c_str());

    } catch (const HTTPError_t &httpError) {
        errEx = ProtocolError;
        Py_INCREF(errEx);
        errArgs = Py_BuildValue("isO", httpError.errorNum(),
            httpError.message().c_str(),
            methodObject);

    } catch (const ProtocolError_t &protocolError) {
        errEx = ProtocolError;
        Py_INCREF(errEx);
        errArgs =Py_BuildValue("isO", protocolError.errorNum(),
            protocolError.message().c_str(),
            methodObject);

    } catch (const StreamError_t &streamError) {
        errEx = ResponseError;
        Py_INCREF(errEx);
        errArgs = Py_BuildValue("sO", streamError.message().c_str(),
            methodObject);

    } catch (const TypeError_t &typeError) {
        errEx = PyExc_TypeError;
        Py_INCREF(errEx);
        errArgs = Py_BuildValue("s", typeError.message().c_str());

    } catch (const Error_t &error) {
        errEx = PyExc_RuntimeError;
        Py_INCREF(errEx);
        errArgs = Py_BuildValue("s", error.message().c_str());

    } catch (std::bad_alloc &badAlloc) {
        errEx = PyExc_MemoryError;
        Py_INCREF(errEx);
        errArgs = Py_BuildValue("s", "Out of memory");

    } catch (PyError_t &pyErr) {
        //closeSocket();
        errEx = PyExc_RuntimeError;
        Py_INCREF(errEx);
        errArgs = Py_BuildValue("s", "FRPC Runtime Error");

    }

    delete marshaller;

    PyObject *umdata = NULL;
    if (errEx == NULL) {
        // check for error (exception already raised)
        umdata = builder.getUnMarshaledData();
        if (!umdata) {
            errEx = PyExc_RuntimeError;
            Py_INCREF(errEx);
            errArgs = Py_BuildValue("s", "FRPC Runtime Error");
        }
    }

    if (errEx == NULL) {
        // check for Fault response
        switch (PyObject_IsInstance(umdata, Fault)) {
            case -1:
                // some exception occurred
                Py_DECREF(umdata);
                PyErr_Fetch(&errEx, &errArgs, NULL);
                break;

            case 1:
                // is Fault
                errEx = Fault;
                Py_INCREF(errEx);
                errArgs = umdata;
                break;
        }
    }

    // Post-call hook
    if (postCall) {
        // Pass following argumens (in this order):
        //   method name
        //   method arguments
        //   callback data from preCall (or None)
        //   exception (or None if no error occured)
        //   exception arguments (or None if no error occured)
        PyObjectWrapper_t result(
            PyObject_CallFunction(postCall, "sOOOO",
                methodObject->name.c_str(), args,
                (callbackData == NULL ? Py_None : callbackData),
                (errEx == NULL ? Py_None : errEx),
                (errArgs == NULL ? Py_None : errArgs)
                ));
        if (PyErr_Occurred()) {
            // Ignore error
            PyErr_Clear();
        }
    }
    if (callbackData != NULL) {
        Py_DECREF(callbackData);
    }

    if (errEx != NULL) {
        if (errArgs) {
            PyErr_SetObject(errEx, errArgs);
            Py_DECREF(errArgs);
        }
        Py_DECREF(errEx);
        return 0;
    }

    // return unmarshalled data
    return umdata;
}

/**************************************************************************/
/*** Other methods                                                      ***/
/**************************************************************************/

extern "C"
{
    static PyObject* fastrpc_boolean(PyObject *self, PyObject *args,
                                     PyObject *keywds);

    static PyObject* fastrpc_dumps(PyObject *self, PyObject *args,
                                   PyObject *keywds);

    static PyObject* fastrpc_loads(PyObject *self, PyObject *args,
                                   PyObject *keywds);

    static PyObject* fastrpc_cleanup(PyObject *self, PyObject *args,
                                   PyObject *keywds);
}

static char fastrpc_boolean__doc__[] =
    "Convert any Python value to FastRPC 'boolean'.\n";

PyObject* fastrpc_boolean(PyObject *, PyObject *args, PyObject *keywds) {
    static const char *kwlist[] = {"value", "truefalse", 0};

    // parse arguments
    PyObject *value;
    PyObject *truefalse = 0;

    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "O|O:fastrpc.boolean", (char **)kwlist,
                                     &value, &truefalse))
        return 0;

    // get index from value (truth value)
    int index = PyObject_IsTrue(value);
    if (index < 0)
        return 0;

    if (!truefalse)
    {
        // just return true/false
        return reinterpret_cast<PyObject*>(newBoolean(index));
    }

    if (!PySequence_Check(truefalse))
    {
        return PyErr_Format(PyExc_TypeError,
                            "truefalse must be sequence not %s.",
                            Py_TYPE(truefalse)->tp_name);
    }

    // get value at given index
    PyObjectWrapper_t pyTruthValue(PySequence_GetItem(truefalse, index));
    if (!pyTruthValue)
        return 0;

    int truthValue = PyObject_IsTrue(pyTruthValue);
    if (truthValue < 0)
        return 0;

    return reinterpret_cast<PyObject*>(newBoolean(truthValue));
}

static char fastrpc_cleanup__doc__[] =
    "Cleanup libxml2 parser in xmlunmarshaller\n";

PyObject* fastrpc_cleanup(PyObject *, PyObject *args, PyObject *keywds) {
    FRPC::cleanupFastrpc();
    Py_RETURN_NONE;
}

namespace {
    class StringWriter_t : public FRPC::Writer_t {
    public:
        StringWriter_t()
            : Writer_t()
        {}

        virtual ~StringWriter_t() {}

        virtual void write(const char *data, unsigned  int size) {
            this->data.append(data, size);
        }

        virtual void flush() { /* noop */ }

        PyObject* getData(bool binary) {
#if PY_MAJOR_VERSION >= 3
            if (binary) {
                return PyBytes_FromStringAndSize(data.data(), data.size());
            } else {
                return PyUnicode_FromStringAndSize(data.data(), data.size());
            }
#else
            return PyString_FromStringAndSize(data.data(), data.size());
#endif
        }

    private:
        std::string data;
    };
}

static char fastrpc_dumps__doc__[] =
    "Convert an argument tuple or a Fault instance to an XML-RPC\n"
    "request (or response, if the methodresponse option is used).\n";

PyObject* fastrpc_dumps(PyObject *, PyObject *args, PyObject *keywds) {
    static const char *kwlist[] = {"params", "methodname", "methodresponse",
                                   "encoding", "useBinary",
                                   "protocolVersionMajor",
                                   "protocolVersionMinor",0};

    // parse arguments
    PyObject *params;
    char *methodname = 0;
    int methodresponse = false;
    const char *encoding = "utf-8";
    int useBinary = false;
    int protocolVersionMajor = 2;
    int protocolVersionMinor = 1;

    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "O|zisiii:fastrpc.dumps", (char **)kwlist,
                                     &params, &methodname, &methodresponse,
                                     &encoding, &useBinary,
                                     &protocolVersionMajor,
                                     &protocolVersionMinor))
        return 0;

    if ((protocolVersionMajor < 0) || (protocolVersionMinor < 0)) {
        PyErr_SetString(PyExc_RuntimeError, "Protocol version must not be negative");
        return 0;
    }

    // create writer
    StringWriter_t writer;

    // create marshaller
    std::auto_ptr<Marshaller_t> marshaller
        (Marshaller_t::create((useBinary
                               ? Marshaller_t::BINARY_RPC
                               : Marshaller_t::XML_RPC),
                               writer,ProtocolVersion_t(protocolVersionMajor,
                                                      protocolVersionMinor)));

    try {
        // normal data
        if (PyTuple_Check(params) || PyList_Check(params)) {
            Feeder_t feeder(marshaller.get(), encoding);
            if (methodname) {
                marshaller->packMethodCall(methodname);
                feeder.feed(params);
            } else  {// now is default method response  if (methodresponse) {
                marshaller->packMethodResponse();
                PyObject *firstParam = 0;
                if (PyTuple_Check(params)) {
                    firstParam = (PyTuple_GET_SIZE(params)
                                  ? PyTuple_GET_ITEM(params, 0)
                                  : 0);
                } else {
                    firstParam = (PyList_GET_SIZE(params)
                                  ? PyList_GET_ITEM(params, 0)
                                  : 0);
                }
                if (firstParam) feeder.feedValue(firstParam);
            } //else {
                // raw data
                //feeder.feedValue(params);
           // }
        } else if (PyObject_IsInstance(params, Fault) > 0) {
            PyObjectWrapper_t faultCode
                (PyObject_GetAttrString(params, "faultCode"));
            if (!faultCode) return 0;
            PyObjectWrapper_t faultString
                (PyObject_GetAttrString(params, "faultString"));
            if (!faultString) return 0;

            if (!PyInt_Check(faultCode.get())) {
                PyErr_SetString(PyExc_TypeError,
                                "Fault.faultCode is not an int");
                return 0;
            }

            PyStrDataType_t str;
            Py_ssize_t strSize;
            STR_ASSTRANDSIZE(faultString.get(), str, strSize) {
                return 0;
            }

            marshaller->packFault(PyInt_AsLong(faultCode.get()), str, strSize);
        } else {
            PyErr_SetString(PyExc_TypeError,
                            "Parameter params must be tuple or fastrpc.Fault "
                            "instance.");
            return 0;
        }

        marshaller->flush();

    } catch (const TypeError_t &typeError) {
        PyErr_SetString(PyExc_TypeError, typeError.message().c_str());
        return 0;

    } catch (const LenError_t &lenError) {
        PyErr_SetString(PyExc_TypeError, lenError.message().c_str());
        return 0;

    } catch (const Error_t &error) {
        PyErr_SetString(PyExc_RuntimeError, error.message().c_str());
        return 0;

    } catch (std::bad_alloc &badAlloc) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;

    } catch (PyError_t &pyErr) {
        return 0;
    }

    // return mashalled string
    return writer.getData(useBinary);
}

static char fastrpc_loads__doc__[] =
    "Convert an XML-RPC packet to unmarshalled data plus a method\n"
    "name (None if not present).\n"
    "Takes optional parameters:\n"
    " * stringMode ('string', 'unicode', 'mixed')\n"
    " * nativeBoolean (True/False, defaults to True)\n"
    " * datetimeBuilder (callable that converts datetime components to python object)\n"
    " * useBinary (True/False/None - Defaults to None, which means 'detect')\n";

PyObject* fastrpc_loads(PyObject *, PyObject *args, PyObject *keywds) {
    static const char *kwlist[] = {"data",
                                   "stringMode",
                                   "nativeBoolean",
                                   "datetimeBuilder",
                                   "useBinary",
                                   0};

    // parse arguments
    PyObject *data;
    char *stringMode_ = 0;
    PyObject *nativeBoolean = 0;
    PyObject *datetimeBuilder = 0;
    PyObject *useBinary = 0;

    if (!PyArg_ParseTupleAndKeywords(
                args, keywds,
                "O|sOOO:fastrpc.loads", (char **)kwlist,
                &data,
                &stringMode_,
                &nativeBoolean, &datetimeBuilder, &useBinary))
        return 0;

    StringMode_t stringMode = parseStringMode(stringMode_);
    if (stringMode == STRING_MODE_INVALID) return 0;

    PyStrDataType_t dataStr;
    Py_ssize_t dataSize;
#if PY_MAJOR_VERSION >= 3
    if (PyBytes_Check(data)) {
        char *ncDataStr;
        if (PyBytes_AsStringAndSize(data, &ncDataStr, &dataSize) < 0) {
            return 0;
        }
        dataStr = ncDataStr;
    } else STR_ASSTRANDSIZE(data, dataStr, dataSize) {
        return 0;
    }
#else
    STR_ASSTRANDSIZE(data, dataStr, dataSize) {
        return 0;
    }
#endif

    try {
        Builder_t builder(0, stringMode,
                nativeBoolean != 0 ? PyObject_IsTrue(nativeBoolean) : true,
                datetimeBuilder);

        std::auto_ptr<UnMarshaller_t> unmarshaller;

        // Use detection if useBinary is unspecified or None
        if (!useBinary || useBinary == Py_None) {
            unmarshaller.reset(
                    UnMarshaller_t::create(dataStr, dataSize, builder));
        } else {
            unmarshaller.reset(
                    UnMarshaller_t::create((PyObject_IsTrue(useBinary)
                                            ? Marshaller_t::BINARY_RPC
                                            : Marshaller_t::XML_RPC),
                                           builder));

            unmarshaller->unMarshall(dataStr, dataSize,
                                     FRPC::UnMarshaller_t::TYPE_ANY);
        }

        // check for error (exception already raised)
        PyObject *umdata = builder.getUnMarshaledData();
        if (!umdata) return 0;

        // check for Fault response
        switch (PyObject_IsInstance(umdata, Fault)) {
        case -1:
            // some exception occurred
            Py_DECREF(umdata);
            return 0;

        case 1:
            // is Fault
            PyErr_SetObject(Fault, umdata);
            return 0;
        }

        // normal response
        return Py_BuildValue("(NN)", umdata, builder.getMethodName());
    } catch (const TypeError_t &typeError) {
        PyErr_SetString(PyExc_TypeError, typeError.message().c_str());
        return 0;

    } catch (const LenError_t &lenError) {
        PyErr_SetString(PyExc_TypeError, lenError.message().c_str());
        return 0;

    } catch (const Error_t &error) {
        PyErr_SetString(PyExc_RuntimeError, error.message().c_str());
        return 0;

    } catch (std::bad_alloc &badAlloc) {
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;

    } catch (PyError_t &pyErr) {
        return 0;
    }
}

static char fastrpc_dumpTree__doc__[] =
    "Dump value as RPC data tree.\n";

namespace {

struct HexWriter_t
    : std::iterator<std::output_iterator_tag, void, void, void, void>
{
    HexWriter_t(std::ostream &os): os(os), space() {}
    HexWriter_t &operator*() { return *this;}
    HexWriter_t &operator++() { return *this;}
    HexWriter_t &operator=(const unsigned char &ch) {
        char x[4];
        snprintf(x, 4, "%02x", ch);
        os << space << x;
        space = " ";
        return *this;
    }
    std::ostream &os;
    std::string space;
};

inline int printString(std::ostringstream &out, PyObject *obj,
                       const char *prefix = "\"",
                       const char *suffix = "\"",
                       const char *err = "<STRING>",
                       Py_ssize_t limit = 10, bool binary = false)
{
    // 7bit string
    PyStrDataType_t str;
    Py_ssize_t len;
    STR_ASSTRANDSIZE(obj, str, len) {
        // oops, cannot get string
        out << err;
        return -1;
    } else {
        // ok, put at most MAX_STRING_LENGTH chars
        out << prefix;

        if (binary) {
            HexWriter_t hexWriter(out);
            std::copy(str, str + std::min(len, limit), hexWriter);
            if ((limit < 0) || (len < limit)) out << "...";

        } else {

            if ((limit < 0) || (len < limit)) {
                out << std::string(str, len);
            } else {
                out << std::string(str, limit) << "...";
            }
        }
        out << suffix;
    }
    return 0;
}

int printPyFastRPCTree(PyObject *tree, std::ostringstream &out,
                       int level, PyObject *names,
                       std::bitset<sizeof(unsigned long) * 8> pos)
{

    if ( Py_TYPE(tree) == Py_TYPE(Py_True) ||
        PyBoolean_Check(tree) ) {
        // print Boolean
        int value = PyObject_IsTrue(tree);
        if (value < 0) {
            return -1;
        } else out << (value ? "true" : "false");
#if PY_MAJOR_VERSION == 2
    } else if (PyInt_Check(tree)) {
        // integer
        out << PyInt_AS_LONG(tree);
#endif
    } else if (PyLong_Check(tree)) {
        Int_t::value_type i = PyLong_AsLongLong(tree);
        // check for error
        if (PyErr_Occurred()) return -1;
        out << i;
    } else if (PyFloat_Check(tree)) {
        // float
        out << PyFloat_AS_DOUBLE(tree);
#if PY_MAJOR_VERSION == 2
    } else if (PyString_Check(tree)) {
        if (printString(out, tree)) return -1;
    } else if (PyUnicode_Check(tree)) {
        // unicode string
        PyObjectWrapper_t pystr =
            PyUnicode_AsUnicodeEscapeString(tree);
        if (!pystr) return -1;
        if (printString(out, pystr, "u\"", "\"", "<UNICODE>"))
            return -1;
#else
    } else if (PyUnicode_Check(tree)) {
        if (printString(out, tree)) return -1;
#endif
    } else if (PyTuple_Check(tree) || PyList_Check(tree)) {
        // list or tuple
        out << '(';
        if (level) {
            // level is ok, we can go on
            Py_ssize_t size = PySequence_Size(tree);
            if (size < 0) {
                // oops, cannot get size
                return -1;
            } else {
                // print all items
                for (size_t i = 0; i < static_cast<size_t>(size); ++i) {
                    if (i) out << ", ";
                    if ((i < pos.size()) && pos.test(i)) {
                        out << "-hidden-";
                        continue;
                    }
                    PyObjectWrapper_t subTree(PySequence_GetItem(tree, i));
                    if (!subTree) {
                        // oops, cannot get item
                        return -1;
                    }
                    // print item
                    if (printPyFastRPCTree(subTree, out, level - 1, names, 0))
                        return -1;
                }
            }
        } else out << "...";
        out << ')';
    } else if (PyDict_Check(tree)) {
        // dict
        out << '{';
        if (level) {
            // level is ok, we can go on
            PyObject *key;
            PyObject *value;
            bool other = false;

            // iterate over dict
            for (Py_ssize_t pos = 0; PyDict_Next(tree, &pos, &key, &value); ) {
                if (other) out << ", ";
                else other = true;

                // print key
                std::ostringstream os;
                if (printPyFastRPCTree(key, os, level - 1, names, 0))
                    return -1;

                // put key && value
                out << os.str() << ": ";

                std::string xkey;
#if PY_MAJOR_VERSION == 2
                if (PyString_Check(key)) {
                    xkey = PyString_AsString(key);
                }
#else
                if (PyUnicode_Check(key)) {
                    xkey = PyUnicode_AsUTF8(key);
                }
#endif
                if (PyMapping_HasKeyString(names,
                                           const_cast<char *>(xkey.c_str()))) {
                    out << "-hidden-";
                } else {
                    // print value
                    if (printPyFastRPCTree(value, out, level - 1, names, 0))
                        return -1;
                }

            }
        } else out << "...";
        out << '}';
    } else if (PyDateTime_Check(tree)) {
        // print DateTime
        PyObjectWrapper_t value(PyObject_GetAttrString(tree, "value"));
        if (!value) {
            out << "<DATETIME>";
            return -1;
        }
        if (printString(out, value, "", "", "<DATETIME>", -1))
            return -1;
#ifdef HAVE_BINARY
    } else if (PyBinary_Check(tree)) {
        PyObjectWrapper_t data(PyObject_GetAttrString(tree, "data"));
        if (!data) return -1;
        if (printString(out, data, "b\"", "\"", "<BASE64>", 10, true))
            return -1;
#endif
#if PY_MAJOR_VERSION >= 3
    } else if (PyBytes_Check(tree)) {
        PyObjectWrapper_t data(PyUnicode_FromFormat("%S", tree));
        if (!data) return -1;
        if (printString(out, data, "", "", "<BASE64>", 10, false))
            return -1;
#endif
    } else {
        // other, unsupported type
        PyObjectWrapper_t value(PyObject_Repr(tree));
        if ( value ) {
            if (printString(out, value, "", "", "", -1))
                return -1;
        } else {
            out << '<' << Py_TYPE(tree)->tp_name << '>';
        }
    }

    // OK
    return 0;
}
}

PyObject* fastrpc_dumpTree(PyObject *, PyObject *args) {
    PyObject *value;
    PyObject *names = 0x0;
    int pos = 0;
    int level = 1;
    if (!PyArg_ParseTuple(args, "O|iOi:fastrpc.dumpTree",
                                &value, &level, &names, &pos))
        return 0;

    std::ostringstream out;
    printPyFastRPCTree(value, out, level, names, pos);

    // OK
    return PyUnicode_FromStringAndSize(out.str().data(), out.str().size());
}

/**************************************************************************/
/*** Module stuff                                                       ***/
/**************************************************************************/

/** Mapping of FRPC module methods to C functions
 */
static PyMethodDef frpc_methods[] =
    {
        {
            "ServerProxy",
            (PyCFunction) ServerProxy_ServerProxy,
            METH_VARARGS | METH_KEYWORDS,
            ServerProxy_ServerProxy__doc__
        }, {
            "boolean",
            (PyCFunction) fastrpc_boolean,
            METH_VARARGS | METH_KEYWORDS,
            fastrpc_boolean__doc__
        }, {
            "cleanup",
            (PyCFunction) fastrpc_cleanup,
            METH_VARARGS | METH_KEYWORDS,
            fastrpc_cleanup__doc__
        }, {
            "dumps",
            (PyCFunction) fastrpc_dumps,
            METH_VARARGS | METH_KEYWORDS,
            fastrpc_dumps__doc__
        }, {
            "loads",
            (PyCFunction) fastrpc_loads,
            METH_VARARGS | METH_KEYWORDS,
            fastrpc_loads__doc__
        }, {
            "dumpTree",
            (PyCFunction) fastrpc_dumpTree,
            METH_VARARGS,
            fastrpc_dumpTree__doc__
        },
        {0, 0 } // end of map
    };



namespace FRPC { namespace Python {
    PyObject* errorRepr(PyObject *self, const std::string &name,
                        const char *status, const char *statusMessage)
    {
        // get method and ingore non-existence
        PyObjectWrapper_t method(PyObject_GetAttrString(self,
                                                        (char *)"method"));
        if (!method) PyErr_Clear();

        PyObjectWrapper_t formatString;
        PyObjectWrapper_t formatArgs;

        // build format string and arguments
        if (!method || (method == Py_None)) {
            // no method
            formatString = (status
                            ? PyUnicode_FromString("<fastrpc.%s: %d, %s>")
                            : PyUnicode_FromString("<fastrpc.%s: %s>"));
            if (!formatString) return 0;

            formatArgs =
                (status
                 ? Py_BuildValue("(sNN)", name.c_str(),
                                 PyObject_GetAttrString(self, (char *)status),
                                 PyObject_GetAttrString(self,
                                     (char *)statusMessage))
                 : Py_BuildValue("(sN)", name.c_str(),
                                 PyObject_GetAttrString(self,
                                     (char *)statusMessage)));
            if (!formatArgs) return 0;
        } else {
            // method!
            MethodObject &methodObject(*method.get<MethodObject>());

            formatString = (status
                            ? PyUnicode_FromString("<fastrpc.%s: %d, %s "
                                                  "[method %s() @ %s]>")
                            : PyUnicode_FromString("<fastrpc.%s: %s "
                                                  "[method %s() @ %s]>"));
            if (!formatString) return 0;

            // get server's full URL
            std::string url(methodObject.proxy->proxy.getURL().getUrl());

            formatArgs =
                (status
                 ? Py_BuildValue("(sNNs#s#)", name.c_str(),
                                 PyObject_GetAttrString(self, (char *)status),
                                 PyObject_GetAttrString(self,
                                                        (char *)statusMessage),
                                 methodObject.name.data(),
                                 methodObject.name.size(),
                                 url.data(), url.size())
                 : Py_BuildValue("(sNs#s#)", name.c_str(),
                                 PyObject_GetAttrString(self,
                                                        (char *)statusMessage),
                                 methodObject.name.data(),
                                 methodObject.name.size(),
                                 url.data(), url.size()));
            if (!formatArgs) return 0;
        }

        // do the format
        return PyUnicode_Format(formatString, formatArgs);
    }
} } // namespace FRPC::Python

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "_fastrpc",
        NULL,
        -1, // TODO: verify that we have module state in global variables
        frpc_methods,
        NULL,
        NULL, //myextension_traverse,
        NULL, //myextension_clear,
        NULL
};

#define INITERROR return NULL

/** Initialize FRPC module
 */
PyMODINIT_FUNC PyInit__fastrpc(void)

#else // PY_MAJOR_VERSION >= 3

#define INITERROR return

/** Initialize FRPC module
 */
PyMODINIT_FUNC init_fastrpc(void)

#endif // PY_MAJOR_VERSION >= 3

{
    /* Create the module and add the functions */
#if PY_MAJOR_VERSION >= 3
    PyObject *fastrpc_module = PyModule_Create(&moduledef);
#else
    PyObject *fastrpc_module = Py_InitModule("_fastrpc", frpc_methods);
#endif
    if (!fastrpc_module)
        INITERROR;

    /**************************************************************/
    // Add types
    /**************************************************************/

    if ((PyType_Ready(&DateTimeObject_Type) < 0) ||
        (PyType_Ready(&LocalTimeObject_Type) < 0) ||
        (PyType_Ready(&UTCTimeObject_Type) < 0) ||
#ifdef HAVE_BINARY
        (PyType_Ready(&BinaryObject_Type) < 0) ||
#endif
        (PyType_Ready(&BooleanObject_Type) < 0) ||
        (PyType_Ready(&ServerProxy_Type) < 0))
        INITERROR;

    Py_INCREF(&DateTimeObject_Type);
    if (PyModule_AddObject(fastrpc_module, "DateTime",
                           reinterpret_cast<PyObject*>(&DateTimeObject_Type)))
        INITERROR;

    Py_INCREF(&LocalTimeObject_Type);
    if (PyModule_AddObject(fastrpc_module, "LocalTime",
                           reinterpret_cast<PyObject*>(&LocalTimeObject_Type)))
        INITERROR;
    Py_INCREF(&UTCTimeObject_Type);
    if (PyModule_AddObject(fastrpc_module, "UTCTime",
                           reinterpret_cast<PyObject*>(&UTCTimeObject_Type)))
        INITERROR;
#ifdef HAVE_BINARY
    Py_INCREF(&BinaryObject_Type);
    if (PyModule_AddObject(fastrpc_module, "Binary",
                           reinterpret_cast<PyObject*>(&BinaryObject_Type)))
        INITERROR;
#endif
    Py_INCREF(&BooleanObject_Type);
    if (PyModule_AddObject(fastrpc_module, "Boolean",
                           reinterpret_cast<PyObject*>(&BooleanObject_Type)))
        INITERROR;

    // initialize server support
    if (FRPC::Python::initServer(fastrpc_module))
        INITERROR;

    // initialize errors
    initErrors(fastrpc_module);

    /********************************************************************/
    // Add constants
    /********************************************************************/
    if (PyModule_AddIntConstant(fastrpc_module, "ON_SUPPORT_ON_KEEP_ALIVE",
                                Proxy_t::BINARY_ON_SUPPORT_ON_KEEP_ALIVE))
        INITERROR;
    if (PyModule_AddIntConstant(fastrpc_module, "ON_SUPPORT",
                                Proxy_t::BINARY_ON_SUPPORT))
        INITERROR;
    if (PyModule_AddIntConstant(fastrpc_module, "ALWAYS",
                                Proxy_t::BINARY_ALWAYS))
        INITERROR;
    if (PyModule_AddIntConstant(fastrpc_module, "NEVER",
                                Proxy_t::BINARY_NEVER))
        INITERROR;

    /********************************************************************/
    // Add False/True Boolean constants
    /********************************************************************/
    if (PyObject *False = reinterpret_cast<PyObject*>(newBoolean(false)))
    {
        if (PyModule_AddObject(fastrpc_module, "False", False))
        {
            Py_DECREF(False);
            INITERROR;
        }
    }
    else
        INITERROR;

    if (PyObject *True = reinterpret_cast<PyObject*>(newBoolean(true)))
    {
        if (PyModule_AddObject(fastrpc_module, "True", True))
        {
            Py_DECREF(True);
            INITERROR;
        }
    }
    else
        INITERROR;

    // import datetime modules
    PyObject *module = 0;
    if ((module = PyImport_ImportModule("mx.DateTime"))) {
        if (!(mxDateTime = PyObject_GetAttrString(module, "DateTimeType")))
            PyErr_Clear();
    } else {
        PyErr_Clear();
    }
    if ((module = PyImport_ImportModule("datetime"))) {
        if (!(dateTimeDateTime = PyObject_GetAttrString(module, "datetime")))
            PyErr_Clear();
    } else {
        PyErr_Clear();
    }

    // create empty string
#ifdef HAVE_BINARY
#if PY_MAJOR_VERSION >= 3
    emptyBinary = PyBytes_FromString("");
#else
    emptyBinary = PyString_FromString("");
#endif
    if (!emptyBinary)
        INITERROR;
#endif

#if PY_MAJOR_VERSION >= 3
        return fastrpc_module;
#endif
}
