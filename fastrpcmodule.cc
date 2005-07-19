/*
 * FastRPC - RPC protocol suport Binary and XML.
 * Copyright (C) 2005 Seznam.cz, a.s.
 *
 * $Id: fastrpcmodule.cc,v 1.1 2005-07-19 13:02:55 vasek Exp $
 * 
 * AUTHOR      Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *      2005-02-16 (miro)
 *              Method displatching (ServerProxy + Method classes).
 */

#define __ENABLE_WSTRING

#include <new>
#include <string>
#include <vector>
#include <memory>

#include <Python.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <frpcmarshaller.h>
#include <frpcwriter.h>
#include <frpcunmarshaller.h>
#include <frpcdatabuilder.h>
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

using FRPC::Marshaller_t;
using FRPC::UnMarshaller_t;
using FRPC::DataBuilder_t;
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

// exceptions
static PyObject *Error = 0;
static PyObject *ProtocolError = 0;
static PyObject *Fault = 0;
static PyObject *ResponseError = 0;

// support constants
static PyObject *emptyString = 0;

#define PyDateTime_Check(op) PyObject_TypeCheck(op, &DateTimeObject_Type)
#define PyDateTime_CheckExact(op) ((op)->ob_type == &DateTimeObject_Type)
#define PyBinary_Check(op) PyObject_TypeCheck(op, &BinaryObject_Type)
#define PyBinary_CheckExact(op) ((op)->ob_type == &BinaryObject_Type)
#define PyBoolean_Check(op) PyObject_TypeCheck(op, &BooleanObject_Type)
#define PyBoolean_CheckExact(op) ((op)->ob_type == &BooleanObject_Type)
#define PyFault_Check(op) PyObject_TypeCheck(op, &Fault)

namespace
{

class PyError_t
{
public:
    PyError_t()
    {}
    ~PyError_t()
    {}
}
;
}

/*************************************************************************/
/***New Python exceptions
           ***/
/*************************************************************************/
class PyObjectWrapper_t
{
public:
    PyObjectWrapper_t(PyObject *object = 0, bool incref = false)
            : object(object)
    {
        if (incref)
            inc();
    }

    PyObjectWrapper_t(const PyObjectWrapper_t &o)
            : object(o.object)
    {
        inc();
    }

    PyObjectWrapper_t& operator=(const PyObjectWrapper_t &o)
    {
        if (this != &o)
        {
            dec();
            object = o.object;
        }
        inc();
        return *this;
    }

    ~PyObjectWrapper_t()
    {

        Py_XDECREF(object);
    }

    PyObject*& inc()
    {

        Py_XINCREF(object);
        return object;
    }

    void dec()
    {
        Py_XDECREF(object);
    }

    operator PyObject*&()
    {
        return object;
    }

    PyObject*& get() {
        return object;
    }

    PyObject*& operator*()
    {
        return object;
    }

    bool operator !() const
    {
        return !object;
    }

    PyObject** addr()
    {
        return &object;
    }

    template <typename object_type> object_type* get() {
        return reinterpret_cast<object_type*>(object);
    }

    PyObject *object;
};

/**************************************************************************/
/* Python DateTime declaration                                            */
/**************************************************************************/

namespace
{
struct DateTimeObject
{
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
}
extern "C"
{
    static PyObject* DateTimeObject_new(PyTypeObject *self, PyObject *args,
                                        PyObject *kwds);
    static int DateTimeObject_init(DateTimeObject *self, PyObject *args,
                                   PyObject *kwds);
    static void DateTimeObject_dealloc(DateTimeObject *self);
    //static int DateTimeObject_cmp(BooleanObject *self, BooleanObject *other);
    static PyObject* DateTimeObject_repr(DateTimeObject *self);
    static PyObject* DateTimeObject_getattr(DateTimeObject *self, char *name);
    static int DateTimeObject_setattr(DateTimeObject *self, char *name, PyObject
                                      *value);
}

/*
* map characterstics of a boolean
*/
PyTypeObject DateTimeObject_Type =
    {
        PyObject_HEAD_INIT(&PyType_Type)
        0,
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
        (initproc)DateTimeObject_init,          /* tp_init */
        PyType_GenericAlloc,                    /* tp_alloc */
        DateTimeObject_new,                     /* tp_new */
        _PyObject_Del                           /* tp_free */
    };


PyObject* DateTimeObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
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

int DateTimeObject_init(DateTimeObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *pyValue = 0;
    DateTimeObject  *newDateTime;
    time_t  locTime;

    if(!PyArg_ParseTuple(args, "|O", &pyValue))
        return -1;

    if(pyValue == 0)
    {
        locTime = time(0);
        self->unixTime = locTime;

        tm *time_tm = localtime(&locTime);
        self->year = time_tm->tm_year + 1900;
        self->month = time_tm->tm_mon + 1;
        self->day = time_tm->tm_mday;
        self->hour = time_tm->tm_hour;
        self->min = time_tm->tm_min;
        self->sec = time_tm->tm_sec;
        self->weekDay = time_tm->tm_wday;
        self->timeZone = 0;

    }
    else if(PyInt_Check(pyValue))
    {
        locTime = PyInt_AsLong(pyValue);
        tm *time_tm = localtime(&locTime);
        self->unixTime = locTime;
        self->year = time_tm->tm_year + 1900;
        self->month = time_tm->tm_mon + 1;
        self->day = time_tm->tm_mday;
        self->hour = time_tm->tm_hour ;
        self->min = time_tm->tm_min;
        self->sec = time_tm->tm_sec;
        self->weekDay = time_tm->tm_wday;
        self->timeZone = 0;

    }
    else if(PyString_Check(pyValue))
    {
        self->timeZone = 0;
        self->weekDay = -1;
        // date vars
        short year;
        char timeZone, month, day, hour, min, sec;
        long len = PyString_Size(pyValue);
        char *data = PyString_AsString(pyValue);
        try
        {
            parseISODateTime(data, len, year, month,
                             day, hour,
                             min, sec, timeZone);
        }
        catch(const StreamError_t &e)
        {
            PyErr_Format(PyExc_Exception,e.message().c_str());
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
        self->unixTime = mktime(&time_tm);

        if(self->unixTime != -1)
        {
            tm *ntime_tm = localtime(&(self->unixTime));
            self->weekDay = ntime_tm->tm_wday;
        }
        else
            self->weekDay = -1;

    }
    else
    {
        PyErr_Format(PyExc_TypeError, "Argument must be string");
        return -1;
    }

    return 0;
}

void DateTimeObject_dealloc(DateTimeObject *self)
{
    self->ob_type->tp_free(reinterpret_cast<PyObject *>(self));
}

PyObject* DateTimeObject_repr(DateTimeObject *self)
{
    std::string  dateTime = getISODateTime(self->year
                                           ,self->month ,
                                           self->day ,self->hour,self->min,self->sec,
                                           self->timeZone);

    return PyString_FromString(dateTime.c_str());

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

        std::string  dateTime = getISODateTime(self->year
                                               ,self->month ,
                                               self->day ,self->hour,self->min,self->sec,
                                               self->timeZone);



        return PyString_FromString(dateTime.c_str());
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

namespace
{
DateTimeObject* newDateTime(short year, char month, char day, char hour, char
                            min, char sec,
                            char weekDay, time_t unixTime, char timeZone)
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
    //compute unix timestamp and weekday
    tm time_tm;
    memset(reinterpret_cast<void*>(&time_tm), 0, sizeof(tm));
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon  = month -1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = min;
    time_tm.tm_sec = sec;
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
}

/**************************************************************************/
/* Python Binary declaration                                              */
/**************************************************************************/

namespace
{
struct BinaryObject
{
    PyObject_HEAD                   /* python standard */
    PyObject *value;                /* true/false value */
};
}
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

/*
* map characterstics of a boolean
*/
PyTypeObject BinaryObject_Type =
    {
        PyObject_HEAD_INIT(&PyType_Type)
        0,
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
        _PyObject_Del                           /* tp_free */
    };


PyObject* BinaryObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    // allocate memory
    assert(type && type->tp_alloc);
    BinaryObject *self =
        reinterpret_cast<BinaryObject*>(type->tp_alloc(type, 0));
    if (!self)
        return 0;

    // fill defaults (what if __init__ doesn't get called
    self->value = emptyString;
    Py_INCREF(self->value);
    return reinterpret_cast<PyObject*>(self);
}

int BinaryObject_init(BinaryObject *self, PyObject *args, PyObject *kwds)
{
    PyObject *pyValue = 0;

    if (!PyArg_ParseTuple(args,"|O!", &PyString_Type, &pyValue))
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
    self->ob_type->tp_free(reinterpret_cast<PyObject *>(self));
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

namespace
{
BinaryObject* newBinary(const char* data, long size)
{
    BinaryObject *self = PyObject_NEW(BinaryObject, &BinaryObject_Type);
    if (self == NULL)
        return NULL;
    self->value = PyString_FromStringAndSize(data, size);
    return self;
}
}

/**************************************************************************/
/* Python Bool declaration                                                */
/**************************************************************************/

namespace
{
/*
    * boolean object
    */
struct BooleanObject
{
    PyObject_HEAD                   /* python standard */
    PyObject *value;                /* true/false value */
};


};

extern "C"
{
    static PyObject* BooleanObject_new(PyTypeObject *self, PyObject *args,
                                       PyObject *kwds);
    static int BooleanObject_init(BooleanObject *self, PyObject *args,
                                  PyObject *kwds);
    static void BooleanObject_dealloc(BooleanObject *self);
    static int BooleanObject_cmp(BooleanObject *self, BooleanObject *other);
    static PyObject* BooleanObject_repr(BooleanObject *self);
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
            0,                                  /* nb_divide */
            0,                                  /* nb_remainder */
            0,                                  /* nb_divmod */
            0,                                  /* nb_power */
            0,                                  /* nb_negative */
            0,                                  /* nb_positive */
            0,                                  /* nb_absolute */
            (inquiry)BooleanObject_nonzero,     /* nb_nonzero */
            0,                                  /* nb_invert */
            0,                                  /* nb_lshift */
            0,                                  /* nb_rshift */
            0,                                  /* nb_and */
            0,                                  /* nb_xor */
            0,                                  /* nb_or */
            0,                                  /* nb_coerce */
            (unaryfunc)BooleanObject_int,       /* nb_int */
            0,                                  /* nb_long */
            0,                                  /* nb_float */
            0,                                  /* nb_oct */
            0,                                  /* nb_hex */
            0,                                  /* nb_inplace_add */
            0,                                  /* nb_inplace_subtract */
            0,                                  /* nb_inplace_multiply */
            0,                                  /* nb_inplace_divide */
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
            0                                   /* nb_inplace_true_divide */
        };


/*
 * map characterstics of a boolean
 */
PyTypeObject BooleanObject_Type =
    {
        PyObject_HEAD_INIT(&PyType_Type)
        0,
        "Boolean",
        sizeof(BooleanObject),
        0,
        (destructor)BooleanObject_dealloc,      /* tp_dealloc */
        0,                                      /* tp_print */
        (getattrfunc)BooleanObject_getattr,     /* tp_getattr */
        (setattrfunc)BooleanObject_setattr,     /* tp_setattr */
        (cmpfunc)BooleanObject_cmp,             /* tp_compare */
        (reprfunc)BooleanObject_repr,           /* tp_repr */
        &Boolean_AsNumber,                      /* tp_as_number */
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
        (initproc)BooleanObject_init,           /* tp_init */
        PyType_GenericAlloc,                    /* tp_alloc */
        BooleanObject_new,                      /* tp_new */
        _PyObject_Del                           /* tp_free */
    };

/*
 * free resources associated with a boolean object
 */
void BooleanObject_dealloc(BooleanObject *self)
{
    Py_XDECREF(self->value);
    self->ob_type->tp_free(reinterpret_cast<PyObject *>(self));
}

PyObject* BooleanObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
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

int BooleanObject_init(BooleanObject *self, PyObject *args, PyObject *kwds)
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
int BooleanObject_cmp(BooleanObject *self, BooleanObject *other)
{
    if (self == other)
        return 0;


    if ((self->value == Py_True) && (other->value == Py_False))
        return 1;
    return -1;
}


/*
 * represent a boolean  object
 */
PyObject* BooleanObject_repr(BooleanObject *self)
{
    return PyString_FromString(PyObject_IsTrue(self->value)? "Boolean: TRUE" :
                               "Boolean: FALSE");
}

int BooleanObject_nonzero(BooleanObject *self)
{
    // check for true
    return self->value == Py_True;
}

PyObject* BooleanObject_int(BooleanObject *self)
{
    Py_INCREF(self->value);
    return self->value;
}

namespace
{
BooleanObject* newBoolean(bool value)
{
    BooleanObject *self = PyObject_NEW(BooleanObject, &BooleanObject_Type);
    if (self == NULL)
        return NULL;

    self->value = (value)?Py_True:Py_False;

    Py_INCREF(self->value);
    return self;
}
}
/***************************************************************************/
/***Builder for unmarshaller                                             ***/
/***************************************************************************/
namespace
{

struct TypeStorage_t
{

    TypeStorage_t(PyObject *container, char
                  type):type(type),container(container)
    {}
    char type;
    PyObject* container;
};

class Builder_t:public DataBuilder_t
{
public:
    enum{NONE=0, INT=1,BOOL,DOUBLE,STRING,DATETIME,BINARY,
         STRUCT=10,ARRAY,METHOD_CALL=13,METHOD_RESPONSE,FAULT,
         MEMBER_NAME = 100,METHOD_NAME,METHOD_NAME_LEN,MAGIC,MAIN };

    Builder_t(PyObject *methodObject)
        : DataBuilder_t(), first(true), error(false), retValue(Py_None),
          methodObject(methodObject), methodName(0)
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
    virtual void buildString(const std::string &data);
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

    inline PyObject* getMethodName() {
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
};

void Builder_t::buildMethodResponse()
{}
void Builder_t::buildBinary(const char* data, long size)
{
    if(isError())
        return;
    PyObject *binary = reinterpret_cast<PyObject*>(newBinary(data, size));

    if(!binary)
        setError();
    
    if(!isMember(binary))
        isFirst(binary);

}
void Builder_t::buildBinary(const std::string &data)
{
    if(isError())
        return;
    PyObject *binary = reinterpret_cast<PyObject*>(newBinary(data.data(),
                       data.size()));
    if(!binary)
        setError();
                       
    if(!isMember(binary))
        isFirst(binary);
}
void Builder_t::buildBool(bool value)
{
    if(isError())
        return;
    PyObject *boolean = reinterpret_cast<PyObject*>(newBoolean(value));

    if(!boolean)
        setError();
        
    if(!isMember(boolean))
        isFirst(boolean);
}
void Builder_t::buildDateTime(short year, char month, char day,char hour, char
                              min, char sec,
                              char weekDay, time_t unixTime, char timeZone)
{
    if(isError())
        return;
    
    PyObject *dateTime = reinterpret_cast<PyObject*>(newDateTime(year, month,
                         day, hour, min,
                         sec, weekDay, unixTime, timeZone) );
    if(!dateTime)
        setError();
        
    if(!isMember(dateTime))
        isFirst(dateTime);
}
void Builder_t::buildDouble(double value)
{
    if(isError())
        return;
    PyObject *doubleVal = PyFloat_FromDouble(value);

    if(!doubleVal)
        setError();
    
    if(!isMember(doubleVal))
        isFirst(doubleVal);

}
void Builder_t::buildFault(long errNumber, const char* errMsg, long size)
{
    if(isError())
        return;
    PyObject *args =Py_BuildValue("iNO",errNumber,
                                  PyString_FromStringAndSize(errMsg,size),
                                  methodObject ? methodObject : Py_None);
    PyObject *fault = PyInstance_New(Fault, args, 0);
    //isFirst(fault);
    
    if(!fault)
        setError();
    
    Py_XDECREF(retValue);
    retValue = fault;
    first = true;

}
void Builder_t::buildFault(long errNumber, const std::string &errMsg)
{
    if(isError())
        return;
    PyObject *args =Py_BuildValue("isO", errNumber, errMsg.c_str(),
                                  methodObject ? methodObject : Py_None);
    PyObject *fault = PyInstance_New(Fault, args, 0);
        
    if(!fault)
        setError();
        
    Py_XDECREF(retValue);
    retValue = fault;
    first = true;
}
void Builder_t::buildInt(long value)
{
    if(isError())
        return;
    PyObject *integer = PyInt_FromLong(value);

    if(!integer)
        setError();
        
    if(!isMember(integer))
        isFirst(integer);

}
void Builder_t::buildMethodCall(const char* methodName1, long size) {
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

void Builder_t::buildString(const char* data, long size)
{
    if(isError())
        return;
    bool utf8 = false;
    PyObject *stringVal;

    for(long i = 0; i < size; i++)
    {
        if(data[i] & 0x80)
        {
            utf8 = true;
            break;
        }
    }

    if(utf8)
    {
        stringVal = PyUnicode_DecodeUTF8(data, size, "strict");
    }
    else
    {
        stringVal = PyString_FromStringAndSize(const_cast<char*>(data), size);
    }

    if(!stringVal)
        setError();
        
    
    if(!isMember(stringVal))
        isFirst(stringVal);

}
void Builder_t::buildString(const std::string &data)
{
    if(isError())
        return;
    PyObject *stringVal;
    bool utf8 = false;

    for(long i = 0; i < data.size(); i++)
    {
        if(data[i] & 0x80)
        {
            utf8 = true;
            break;
        }
    }

    if(utf8)
    {
        stringVal = PyUnicode_DecodeUTF8(data.data(), data.size(), "strict");
    }
    else
    {
        stringVal = PyString_FromString(data.c_str());
    }

    if(!stringVal)
        setError();
    
    if(!isMember(stringVal))
        isFirst(stringVal);


}
void Builder_t::buildStructMember(const char *memberName, long size )
{
    if(isError())
        return;
    std::string buf(memberName,size);
    this->memberName =  buf;

}
void Builder_t::buildStructMember(const std::string &memberName)
{
    if(isError())
        return;
    this->memberName = memberName;
}
void Builder_t::closeArray()
{
    if(isError())
        return;
    entityStorage.pop_back();
}
void Builder_t::closeStruct()
{
    if(isError())
        return;
    entityStorage.pop_back();
}
void Builder_t::openArray(long numOfItems)
{
    if(isError())
        return;
    PyObject *array = PyList_New(0);

    if(!array)
        setError();
    
    if(!isMember(array))
        isFirst(array);

    entityStorage.push_back(TypeStorage_t(array,ARRAY));
}
void Builder_t::openStruct(long numOfMembers)
{

    if(isError())
        return;
    PyObject *structVal = PyDict_New();

    if(!structVal)
        setError();
    
    if(!isMember(structVal))
        isFirst(structVal);

    entityStorage.push_back(TypeStorage_t(structVal,STRUCT));
}



}


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
    virtual void write(const char *data, long size );
    virtual void flush();

    inline const char* data()
    {
        return localBuffer.data();
    }

    inline long size()
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

void OutBuffer_t::write(const char *data, long size )
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
            int rpcTransferMode, const std::string &encoding, bool useHTTP10,
            const std::string &proxyVia)
        : url(serverUrl, proxyVia), socket(-1),
          io(socket, readTimeout, writeTimeout, -1, -1),
          connectTimeout(connectTimeout), keepAlive(keepAlive),
          rpcTransferMode(rpcTransferMode), encoding(encoding),
          serverSupportedProtocols(HTTPClient_t::XML_RPC),
          useHTTP10(useHTTP10)
    {}

    ~Proxy_t()
    {}

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

private:

    URL_t url;
    int socket;
    HTTPIO_t io;
    int readTimeout;
    int writeTimeout;
    int connectTimeout;
    bool keepAlive;
    int rpcTransferMode;
    std::string encoding;

    std::string path;
    unsigned long serverSupportedProtocols;

    bool useHTTP10;
    std::string lastCall;
};

struct ServerProxyObject
{
    PyObject_HEAD

    Proxy_t proxy;
    bool proxyOk;
};
}

namespace
{
class  Feeder_t
{
public:
    Feeder_t(Marshaller_t *marshaller,const std::string &encoding)
            :marshaller(marshaller), encoding(encoding)
    {}

    void feed(PyObject *args);
    void feedValue(PyObject *value);

private:
    Feeder_t();
    Marshaller_t *marshaller;
    const std::string encoding;
};

}

extern "C"
{
    static PyObject* ServerProxy_ServerProxy(ServerProxyObject *self,
            PyObject *args, PyObject *keywds);

    static void ServerProxy_dealloc(ServerProxyObject *self);

    static PyObject* ServerProxy_getattr(ServerProxyObject *self, char *name);
};

static PyTypeObject ServerProxy_Type =
    {
        PyObject_HEAD_INIT(&ServerProxy_Type)
        0,                                 /*ob_size*/
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
        0,                                 /* tp_call */
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
        PyObject_HEAD_INIT(&Method_Type)
        0,                                 /*ob_size*/
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
    // call destructor for name
    self->name.~string();

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
                (PyString_FromStringAndSize(self->name.data(), self->name.size()));
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

PyObject* Method_call(MethodObject *self, PyObject *args, PyObject *keys)
{
    // check whether we are called with keywords and fail if so
    if (keys)
    {
        PyErr_SetString(PyExc_TypeError,
                        "Cannot call remote method with keyword arguments.");
        return 0;
    }

    // make remote call
    return self->proxy->proxy(self, args);
}

/**************************************************************************/
/*** Server proxy object implementation                                 ***/
/**************************************************************************/

static char ServerProxy_ServerProxy__doc__[] =
    "Create new ServerProxy\n"
    ;

PyObject* ServerProxy_ServerProxy(ServerProxyObject *self, PyObject *args,
                                  PyObject *keywds)
{
    static char *kwlist[] = {"serverUrl", "readTimeout", "writeTimeout",
                             "connectTimeout",
                             "keepAlive","useBinary","encoding",
                             "useHTTP10", "proxyVia", 0};

    // parse arguments
    char *serverUrl;
    int serverUrlLen;
    int readTimeout = -1;
    int writeTimeout = -1;
    int connectTimeout = -1;
    int keepAlive = 0;
    int mode = Proxy_t::BINARY_ON_SUPPORT_ON_KEEP_ALIVE;
    char *encoding = "utf-8";
    int useHTTP10 = false;
    char *proxyVia = "";

    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "s#|iiiiisis:ServerProxy.__init__", kwlist,
                                     &serverUrl, &serverUrlLen, &readTimeout,
                                     &writeTimeout, &connectTimeout,
                                     &keepAlive,&mode, &encoding,
                                     &useHTTP10, &proxyVia))
        return 0;

    // create server proxy object
    ServerProxyObject *proxy = PyObject_New(ServerProxyObject,
                                            &ServerProxy_Type);
    if (!proxy) return 0;

    proxy->proxyOk = false;

    try
    {
        // initialize underlying proxy (inplace!)
        new (&proxy->proxy) Proxy_t(std::string(serverUrl, serverUrlLen),
                                    readTimeout, writeTimeout,
                                    connectTimeout, keepAlive,mode, encoding,
                                    useHTTP10, proxyVia);
        proxy->proxyOk = true;
    }
    catch (const HTTPError_t &httpError)
    {
        Py_DECREF(proxy);
        PyObject *args = Py_BuildValue
            ("is", httpError.errorNum(), httpError.message().c_str());
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

PyObject* ServerProxy_getattr(ServerProxyObject *self, char *name)
{
    if (!strncmp(name, "__", 2)) {
        if (!strcmp(name, "__dict__")) {
            const URL_t &url = self->proxy.getURL();

            PyObjectWrapper_t dict(PyDict_New());
            if (!dict) return 0;

            PyObjectWrapper_t host
                (PyString_FromStringAndSize(url.host.data(), url.host.size()));
            if (!host) return 0;
            if (PyDict_SetItemString(dict, "host", host)) return 0;

            PyObjectWrapper_t path
                (PyString_FromStringAndSize(url.path.data(), url.path.size()));
            if (!path) return 0;
            if (PyDict_SetItemString(dict, "path", path)) return 0;

            PyObjectWrapper_t port(PyInt_FromLong(url.port));
            if (!port) return 0;
            if (PyDict_SetItemString(dict, "port", port)) return 0;

            std::string fullUrl(url.getUrl());
            PyObjectWrapper_t pyfullUrl
                (PyString_FromStringAndSize(fullUrl.data(), fullUrl.size()));
            if (!pyfullUrl) return 0;
            if (PyDict_SetItemString(dict, "url", pyfullUrl)) return 0;

            const std::string &lastCall(self->proxy.getLastCall());
            PyObjectWrapper_t pyLastCall
                (PyString_FromStringAndSize(lastCall.data(), lastCall.size()));
            if (PyDict_SetItemString(dict, "last_call", pyLastCall)) return 0;

            return dict.inc();
        }
        return PyErr_Format(PyExc_AttributeError,
                            "ServerProxy object has no attributes '%s'",
                            name);
    }

    // return new method
    return newMethod(self, name);
}

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


PyObject* Proxy_t::operator()(MethodObject *methodObject, PyObject *args)
{
    // remember last method
    lastCall = methodObject->name;

    HTTPClient_t client(io,url,connectTimeout,false, useHTTP10);
    Builder_t builder(reinterpret_cast<PyObject*>(methodObject));
    Marshaller_t *marshaller;

    switch(rpcTransferMode) {
    case BINARY_NEVER:
        //using XML_RPC
        marshaller= Marshaller_t::create(Marshaller_t::XML_RPC,client);
        client.prepare(HTTPClient_t::XML_RPC);
        break;

    case BINARY_ON_SUPPORT:
        if(serverSupportedProtocols & HTTPClient_t::BINARY_RPC) {
            //using BINARY_RPC
            marshaller=
                Marshaller_t::create(Marshaller_t::BINARY_RPC,client);
            client.prepare(HTTPClient_t::BINARY_RPC);
        } else {
            //using XML_RPC
            marshaller= Marshaller_t::create(Marshaller_t::XML_RPC,client);
            client.prepare(HTTPClient_t::XML_RPC);
        }
        break;

    case BINARY_ALWAYS:
        //using BINARY_RPC  always
        marshaller= Marshaller_t::create(Marshaller_t::BINARY_RPC,client);
        client.prepare(HTTPClient_t::BINARY_RPC);
        break;

    case BINARY_ON_SUPPORT_ON_KEEP_ALIVE:
    default:
        if(serverSupportedProtocols & HTTPClient_t::XML_RPC || keepAlive ==
           false || socket != -1) {
            //using XML_RPC
            marshaller= Marshaller_t::create(Marshaller_t::XML_RPC,client);
            client.prepare(HTTPClient_t::XML_RPC);
        } else {
            //using BINARY_RPC
            marshaller=
                Marshaller_t::create(Marshaller_t::BINARY_RPC,client);
            client.prepare(HTTPClient_t::BINARY_RPC);
        }
        break;
    }

    try {
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
    }

    catch (const LenError_t &lenError) {
        delete marshaller;
        PyErr_SetString(PyExc_TypeError, lenError.message().c_str());
        return 0;

    } catch (const HTTPError_t &httpError) {
        delete marshaller;
        PyObject *args = Py_BuildValue("isO", httpError.errorNum(),
                                       httpError.message().c_str(),
                                       methodObject);
        if (args) PyErr_SetObject(ProtocolError, args);
        return 0;

    } catch (const ProtocolError_t &protocolError) {
        delete marshaller;
        PyObject *args =Py_BuildValue("isO", protocolError.errorNum(),
                                      protocolError.message().c_str(),
                                      methodObject);
        if (args) PyErr_SetObject(ProtocolError, args);
        return 0;

    } catch (const StreamError_t &streamError) {
        delete marshaller;
        PyObject *args = Py_BuildValue("sO", streamError.message().c_str(),
                                       methodObject);
        if (args) PyErr_SetObject(ResponseError, args);
        return 0;

    } catch (const TypeError_t &typeError) {
        delete marshaller;
        PyErr_SetString(PyExc_TypeError, typeError.message().c_str());
        return 0;

    } catch (const Error_t &error) {
        delete marshaller;
        PyErr_SetString(PyExc_RuntimeError, error.message().c_str());
        return 0;

    } catch (std::bad_alloc &badAlloc) {
        delete marshaller;
        PyErr_SetString(PyExc_MemoryError, "Out of memory");
        return 0;

    } catch (PyError_t &pyErr) {
        delete marshaller;
        //closeSocket();
        return 0;
    }

    delete marshaller;

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

    static PyObject* fastrpc_datetime(PyObject *self, PyObject *args);

    static PyObject* fastrpc_dumps(PyObject *self, PyObject *args,
                                   PyObject *keywds);

    static PyObject* fastrpc_loads(PyObject *self, PyObject *args,
                                   PyObject *keywds);
}

static char fastrpc_boolean__doc__[] =
    "Convert any Python value to FastRPC 'boolean'.\n";

PyObject* fastrpc_boolean(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *kwlist[] = {"value", "truefalse", 0};

    // parse arguments
    PyObject *value;
    PyObject *truefalse = 0;

    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "O|O:fastrpc.boolean", kwlist,
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
                            truefalse->ob_type->tp_name);
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

namespace {
    class StringWriter_t : public FRPC::Writer_t {
    public:
        StringWriter_t()
            : Writer_t()
        {}

        virtual ~StringWriter_t() {}

        virtual void write(const char *data, long int size) {
            this->data.append(data, size);
        }

        virtual void flush() { /* noop */ }

        PyObject* getData() {
            return PyString_FromStringAndSize(data.data(), data.size());
        }

    private:
        std::string data;
    };
}

static char fastrpc_dumps__doc__[] =
    "Convert an argument tuple or a Fault instance to an XML-RPC\n"
    "request (or response, if the methodresponse option is used).\n";

PyObject* fastrpc_dumps(PyObject *self, PyObject *args, PyObject *keywds) {
    static char *kwlist[] = {"params", "methodname", "methodresponse",
                             "encoding", "useBinary",0};

    // parse arguments
    PyObject *params;
    char *methodname = 0;
    int methodresponse = false;
    char *encoding = "utf-8";
    int useBinary = false;

    if (!PyArg_ParseTupleAndKeywords(args, keywds,
                                     "O|sisi:fastrpc.dumps", kwlist,
                                     &params, &methodname, &methodresponse,
                                     &encoding, &useBinary))
        return 0;

    // create writer
    StringWriter_t writer;

    // create marshaller
    std::auto_ptr<Marshaller_t> marshaller
        (Marshaller_t::create((useBinary
                               ? Marshaller_t::BINARY_RPC
                               : Marshaller_t::XML_RPC),
                              writer));

    try {
        // normal data
        if (PyTuple_Check(params) || PyList_Check(params)) {
            Feeder_t feeder(marshaller.get(), encoding);
            if (methodname) {
                marshaller->packMethodCall(methodname);
                feeder.feed(params);
            } else if (methodresponse) {
                marshaller->packMethodResponse();
                feeder.feedValue(params);
            } else {
                // raw data
                feeder.feedValue(params);
            }
        } else if (PyObject_IsInstance(params, Fault)) {
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
            
            char *str;
            int strSize;
            if (PyString_AsStringAndSize(faultString.get(), &str, &strSize))
                return 0;

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
    return writer.getData();
}

static char fastrpc_loads__doc__[] =
    "Convert an XML-RPC packet to unmarshalled data plus a method\n"
    "name (None if not present).\n";

PyObject* fastrpc_loads(PyObject *self, PyObject *args) {
    // parse arguments
    PyObject *data;

    if (!PyArg_ParseTuple(args, "O:fastrpc.loads", &data))
        return 0;

    char *dataStr;
    int dataSize;
    if (PyString_AsStringAndSize(data, &dataStr, &dataSize))
        return 0;

    try {
        Builder_t builder(0);
        std::auto_ptr<UnMarshaller_t> unmarshaller
            (UnMarshaller_t::create(dataStr, dataSize, builder));

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
            "dumps",
            (PyCFunction) fastrpc_dumps,
            METH_VARARGS | METH_KEYWORDS,
            fastrpc_dumps__doc__
        }, {
            "loads",
            (PyCFunction) fastrpc_loads,
            METH_VARARGS,
            fastrpc_loads__doc__
        },
        {0, 0 } // end of map
    };

namespace
{
PyObject* initException(PyObject *module, char *name,
                        char *niceName, PyObject *base,
                        PyMethodDef *methodDef)
{
    PyObjectWrapper_t Exception_dict(PyDict_New());
    // create runtime error
    PyObject *Exception = PyErr_NewException(niceName, base,
                          Exception_dict);
    if (!Exception)
        return 0;

    // populate runtime error class with its methods
    for (PyMethodDef *md = methodDef; md->ml_name; ++md)
    {
        PyObjectWrapper_t func(PyCFunction_New(md, NULL));
        if (!func)
            return 0;

        // turn the function into an unbound method
        PyObjectWrapper_t method(PyMethod_New(func, NULL, Exception));
        if (!func)
            return 0;
        if (PyDict_SetItemString(Exception_dict, md->ml_name, method))
            return 0;
    }

    // add this exception into the xmlrpcserver module
    if (PyModule_AddObject(module, name, Exception))
        return 0;

    // OK
    return Exception;
}
}

//******************************************************************
/***Error */
//******************************************************************
extern "C"
{

    static PyObject* Error__init__(PyObject *self, PyObject *args)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyObject* Error__repr__(PyObject *self, PyObject *args)
    {
        return PyString_FromString("<fastrpc.Error>");
    }

    static PyMethodDef ErrorMethod_methods[] = {
                { "__init__", Error__init__,
                  METH_VARARGS, "fastrpc.Error constuctor" },
                { "__repr__", Error__repr__,
                  METH_VARARGS, "print fastrpc.Error)" },
                { "__str__", Error__repr__,
                  METH_VARARGS, "print fastrpc.Error)" },
                { NULL, NULL, 0, NULL } // sentinel
            };

} // extern "C"

//********************************************************************
//*****ProtocolError
//********************************************************************

namespace {
    PyObject* errorRepr(PyObject *self, const std::string &name,
                        char *status, char *statusMessage)
    {
        // get method and ingore non-existence
        PyObjectWrapper_t method(PyObject_GetAttrString(self, "method"));
        if (!method) PyErr_Clear();

        PyObjectWrapper_t formatString;
        PyObjectWrapper_t formatArgs;
    
        // build format string and arguments
        if (!method || (method == Py_None)) {
            // no method
            formatString = (status
                            ? PyString_FromString("<fastrpc.%s: %d, %s>")
                            : PyString_FromString("<fastrpc.%s: %s>"));
            if (!formatString) return 0;

            formatArgs =
                (status
                 ? Py_BuildValue("(sNN)", name.c_str(), 
                                 PyObject_GetAttrString(self, status),
                                 PyObject_GetAttrString(self, statusMessage))
                 : Py_BuildValue("(sN)", name.c_str(), 
                                 PyObject_GetAttrString(self, statusMessage)));
            if (!formatArgs) return 0;
        } else {
            // method!
            MethodObject &methodObject(*method.get<MethodObject>());

            formatString = (status
                            ? PyString_FromString("<fastrpc.%s: %d, %s "
                                                  "[method %s() @ %s]>")
                            : PyString_FromString("<fastrpc.%s: %s "
                                                  "[method %s() @ %s]>"));
            if (!formatString) return 0;

            // get server's full URL
            std::string url(methodObject.proxy->proxy.getURL().getUrl());

            formatArgs =
                (status
                 ? Py_BuildValue("(sNNs#s#)", name.c_str(),
                                 PyObject_GetAttrString(self, status),
                                 PyObject_GetAttrString(self, statusMessage),
                                 methodObject.name.data(),
                                 methodObject.name.size(),
                                 url.data(), url.size())
                 : Py_BuildValue("(sNs#s#)", name.c_str(),
                                 PyObject_GetAttrString(self, statusMessage),
                                 methodObject.name.data(),
                                 methodObject.name.size(),
                                 url.data(), url.size()));
            if (!formatArgs) return 0;
        }

        // do the format
        return PyString_Format(formatString, formatArgs);
    }
}

extern "C"
{

    static PyObject* ProtocolError__init__(PyObject *self, PyObject *args)
    {
        PyObject *status = 0;
        PyObject *statusMessage = 0;
        PyObject *method = Py_None;

        if (!PyArg_ParseTuple(args, "OOO|O:__init__", &self,
                              &status, &statusMessage, &method))
            return 0;

        if (!PyNumber_Check(status)) {
            PyErr_Format(PyExc_TypeError,
                         "ProtocolError.status must be number, not %s.",
                         status->ob_type->tp_name);
            return 0;
        }

        if (!(PyString_Check(statusMessage) || PyUnicode_Check(statusMessage)))
        {
            PyErr_Format(PyExc_TypeError,
                         "ProtocolError.statusMessage must be string "
                         "or unicode, not %s.",
                         statusMessage->ob_type->tp_name);
            return 0;
        }

        if (PyObject_SetAttrString(self, "status", status)
            || PyObject_SetAttrString(self, "statusMessage", statusMessage)
            || PyObject_SetAttrString(self, "method", method))
            return 0;
        

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyObject* ProtocolError__repr__(PyObject *self, PyObject *args)
    {
        self = PyTuple_GetItem(args, 0);
        if (!self) return 0;

        return errorRepr(self, "ProtocolError", "status", "statusMessage");
    }

    static PyMethodDef ProtocolErrorMethod_methods[] = {
                { "__init__", ProtocolError__init__,
                  METH_VARARGS, "fastrpc.ProtocolError constuctor" },
                { "__repr__", ProtocolError__repr__,
                  METH_VARARGS, "repr(ProtocolError)" },
                { "__str__", ProtocolError__repr__,
                  METH_VARARGS, "str(ProtocolError)" },
                { NULL, NULL, 0, NULL } // sentinel
            };

} // extern "C"

//*************************************************************************
//**Fault
//*************************************************************************

extern "C"
{

    static PyObject* Fault__init__(PyObject *self, PyObject *args)
    {
        PyObject *faultCode;
        PyObject *faultString;
        PyObject *method = Py_None;

        if (!PyArg_ParseTuple(args, "OOO|O:__init__", &self,
                              &faultCode, &faultString, &method))
            return 0;

        if (!PyNumber_Check(faultCode))
        {
            PyErr_Format(PyExc_TypeError,
                         "Fault.faultCode must be number, not %s.",
                         faultCode->ob_type->tp_name);
            return 0;
        }

        if (!(PyString_Check(faultString) || PyUnicode_Check(faultString)))
        {
            PyErr_Format(PyExc_TypeError,
                         "Fault.faultString must be string "
                         "or unicode, not %s.",
                         faultString->ob_type->tp_name);
            return 0;
        }

        if (PyObject_SetAttrString(self, "faultCode", faultCode)
            || PyObject_SetAttrString(self, "faultString", faultString)
            || PyObject_SetAttrString(self, "method", method))
            return 0;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyObject* Fault__repr__(PyObject *self, PyObject *args)
    {
        self = PyTuple_GetItem(args, 0);
        if (!self)
            return 0;

        return errorRepr(self, "Fault", "faultCode", "faultString");
    }

    static PyMethodDef FaultMethod_methods[] = {
                { "__init__", Fault__init__,
                  METH_VARARGS, "fastrpc.Fault constuctor" },
                { "__repr__", Fault__repr__,
                  METH_VARARGS, "repr(Fault)" },
                { "__str__", Fault__repr__,
                  METH_VARARGS, "str(Fault)" },
                { NULL, NULL, 0, NULL } // sentinel
            };

} // extern "C"

//*************************************************************************
//*ResponseError
//*************************************************************************
extern "C"
{

    static PyObject* ResponseError__init__(PyObject *self, PyObject *args)
    {

        PyObject *statusMessage;
        PyObject *method = Py_None;

        if (!PyArg_ParseTuple(args, "OO|O:__init__", &self, &statusMessage,
                              &method))
            return 0;

        if (!(PyString_Check(statusMessage) || PyUnicode_Check(statusMessage)))
        {
            PyErr_Format(PyExc_TypeError,
                         "ResponseError.statusMessage must be string "
                         "or unicode, not %s.",
                         statusMessage->ob_type->tp_name);
            return 0;
        }


        if (PyObject_SetAttrString(self, "statusMessage", statusMessage)
            || PyObject_SetAttrString(self, "method", method))
            return 0;

        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyObject* ResponseError__repr__(PyObject *self, PyObject *args)
    {
        self = PyTuple_GetItem(args, 0);
        if (!self) return 0;


        return errorRepr(self, "ResponseError", 0, "statusMessage");
    }

    static PyMethodDef ResponseErrorMethod_methods[] = {
                { "__init__", ResponseError__init__,
                  METH_VARARGS, "fastrpc.ResponseError constuctor" },
                { "__repr__", ResponseError__repr__,
                  METH_VARARGS, "repr(ResponseError)" },
                { "__str__", ResponseError__repr__,
                  METH_VARARGS, "str(ResponseError)" },
                { NULL, NULL, 0, NULL } // sentinel
            };

} // extern "C"

/** Initialize FRPC module
 */
extern "C" DL_EXPORT(void) initfastrpc(void)
{
    /* Create the module and add the functions */
    PyObject *fastrpc_module = Py_InitModule("fastrpc", frpc_methods);
    if (!fastrpc_module)
        return;

    /**************************************************************/
    // Add types
    /**************************************************************/
    Py_INCREF(&DateTimeObject_Type);
    if (PyModule_AddObject(fastrpc_module, "DateTime",
                           reinterpret_cast<PyObject*>(&DateTimeObject_Type)))
        return;
    Py_INCREF(&BinaryObject_Type);
    if (PyModule_AddObject(fastrpc_module, "Binary",
                           reinterpret_cast<PyObject*>(&BinaryObject_Type)))
        return;
    Py_INCREF(&BooleanObject_Type);
    if (PyModule_AddObject(fastrpc_module, "Boolean",
                           reinterpret_cast<PyObject*>(&BooleanObject_Type)))
        return;

    /**************************************************************/
    //Error init
    /**************************************************************/
    Error = initException(fastrpc_module, "Error", "fastrpc.Error",
                          PyExc_Exception, ErrorMethod_methods);
    if (!Error)
        return;

    /********************************************************************/
    //ProtocolError
    /**************************************************************/
    ProtocolError = initException(fastrpc_module, "ProtocolError",
                                  "fastrpc.ProtocolError",
                                  Error, ProtocolErrorMethod_methods);
    if (!ProtocolError)
        return;

    /********************************************************************/
    //fault
    /********************************************************************/
    Fault = initException(fastrpc_module, "Fault", "fastrpc.Fault",
                          Error, FaultMethod_methods);
    if (!Fault)
        return;

    /********************************************************************/
    // ResponseError
    /**************************************************************/
    ResponseError = initException(fastrpc_module, "ResponseError",
                                  "fastrpc.ResponseError",
                                  Error, ResponseErrorMethod_methods);
    if (!ResponseError)
        return;

    /********************************************************************/
    // Add constants
    /********************************************************************/
    if (PyModule_AddIntConstant(fastrpc_module, "ON_SUPPORT_ON_KEEP_ALIVE",
                                Proxy_t::BINARY_ON_SUPPORT_ON_KEEP_ALIVE))
        return;
    if (PyModule_AddIntConstant(fastrpc_module, "ON_SUPPORT",
                                Proxy_t::BINARY_ON_SUPPORT))
        return;
    if (PyModule_AddIntConstant(fastrpc_module, "ALWAYS",
                                Proxy_t::BINARY_ALWAYS))
        return;
    if (PyModule_AddIntConstant(fastrpc_module, "NEVER",
                                Proxy_t::BINARY_NEVER))
        return;

    /********************************************************************/
    // Add False/True Boolean constants
    /********************************************************************/
    if (PyObject *False = reinterpret_cast<PyObject*>(newBoolean(false)))
    {
        if (PyModule_AddObject(fastrpc_module, "False", False))
        {
            Py_DECREF(False);
            return;
        }
    }
    else
        return;

    if (PyObject *True = reinterpret_cast<PyObject*>(newBoolean(true)))
    {
        if (PyModule_AddObject(fastrpc_module, "True", True))
        {
            Py_DECREF(True);
            return;
        }
    }
    else
        return;

    // create empty string
    emptyString = PyString_FromString("");
    if (!emptyString)
        return;
}
