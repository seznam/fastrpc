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
 * AUTHOR      Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * DESCRIPTION
 * Python FastRPC support. Errors.
 *
 */


// Included first to get rid of the _POSIX_C_SOURCE warning
#include <Python.h>
#include <structmember.h>

#include <string>
#include <iostream>

#include "fastrpcmodule.h"

using namespace FRPC::Python;

/* note these macros omit the last semicolon so the macro invocation may
 * include it and not look strange.
 */
#define SimpleException(EXCNAME, EXCREPR, EXCDOC) \
static PyTypeObject EXCNAME ## Exception = { \
    PyVarObject_HEAD_INIT(NULL, 0) \
    "fastrpc." # EXCNAME, \
    sizeof(PyBaseExceptionObject), 0, \
    ((PyTypeObject*)PyExc_BaseException)->tp_dealloc, 0, 0, 0, 0, \
    (reprfunc)EXCREPR, 0, 0, 0, 0, 0, \
    (reprfunc)EXCREPR, 0, 0, 0, \
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, \
    PyDoc_STR(EXCDOC), ((PyTypeObject*)PyExc_BaseException)->tp_traverse, \
    ((PyTypeObject*)PyExc_BaseException)->tp_clear, 0, 0, 0, 0, 0, 0, 0, \
    (PyTypeObject*)PyExc_Exception, 0, 0, 0, \
    offsetof(PyBaseExceptionObject, dict), \
    ((PyTypeObject*)PyExc_BaseException)->tp_init, 0, \
    ((PyTypeObject*)PyExc_BaseException)->tp_new,\
};

#define ComplexException(EXCNAME, EXCSTORE, EXCMETHODS, EXCMEMBERS, \
    EXCDOC) \
static PyTypeObject EXCNAME ## Exception = { \
    PyVarObject_HEAD_INIT(NULL, 0) \
    "fastrpc." # EXCNAME, \
    sizeof(EXCSTORE ## Object), 0, \
    (destructor)EXCSTORE ## _dealloc, 0, 0, 0, 0, \
    (reprfunc)EXCSTORE ## _repr, 0, 0, 0, 0, 0, \
    (reprfunc)EXCSTORE ## _repr, 0, 0, 0, \
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_GC, \
    PyDoc_STR(EXCDOC), (traverseproc)EXCSTORE ## _traverse, \
    (inquiry)EXCSTORE ## _clear, 0, 0, 0, 0, EXCMETHODS, \
    EXCMEMBERS, 0, (PyTypeObject*)PyExc_Exception, \
    0, 0, 0, offsetof(EXCSTORE ## Object, dict), \
    (initproc)EXCSTORE ## _init, \
}; \

//******************************************************************
//***Error */
//******************************************************************

static PyObject* Error_repr(PyObject * /*self*/)
{
    return PyUnicode_FromString("<fastrpc.Error>");
}

SimpleException(Error, Error_repr, "General FastRPC error exception");


//********************************************************************
//*****ProtocolError
//********************************************************************

typedef struct {
#if PY_MAJOR_VERSION >= 3
    PyException_HEAD
#else
    PyObject_HEAD
    PyObject *dict;
    PyObject *args;
    PyObject *message;
#endif

    PyObject *status;
    PyObject *statusMessage;
    PyObject *method;
} ProtocolErrorObject;

static int
ProtocolError_init(ProtocolErrorObject *self, PyObject *args, PyObject *kwds)
{
    if (((PyTypeObject*)PyExc_BaseException)->tp_init((PyObject *)self, args, kwds) == -1)
        return -1;

    self->method = Py_None;
    if (!PyArg_ParseTuple(args, "OO|O:__init__",
            &self->status, &self->statusMessage, &self->method))
        return -1;

    Py_XINCREF(self->status);
    Py_XINCREF(self->statusMessage);
    Py_XINCREF(self->method);

    if (!PyNumber_Check(self->status)) {
        PyErr_Format(PyExc_TypeError,
            "ProtocolError.status must be number, not %s.",
            Py_TYPE(self->status)->tp_name);
        return -1;
    }

#if PY_MAJOR_VERSION >= 3
    if (!PyUnicode_Check(self->statusMessage))
#else
    if (!(PyString_Check(self->statusMessage) || PyUnicode_Check(self->statusMessage)))
#endif
    {
        PyErr_Format(PyExc_TypeError,
            "ProtocolError.statusMessage must be string "
            "or unicode, not %s.",
            Py_TYPE(self->statusMessage)->tp_name);
        return -1;
    }

    return 0;
}

static int
ProtocolError_clear(ProtocolErrorObject *self)
{
    Py_CLEAR(self->status);
    Py_CLEAR(self->statusMessage);
    Py_CLEAR(self->method);
    return ((PyTypeObject*)PyExc_BaseException)->tp_clear((PyObject *)self);
}

static void
ProtocolError_dealloc(ProtocolErrorObject *self)
{
    ProtocolError_clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int
ProtocolError_traverse(ProtocolErrorObject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->status);
    Py_VISIT(self->statusMessage);
    Py_VISIT(self->method);
    return ((PyTypeObject*)PyExc_BaseException)->tp_traverse((PyObject *)self, visit, arg);
}

static PyObject *
ProtocolError_repr(ProtocolErrorObject *self)
{
    return errorRepr((PyObject*)self, "ProtocolError", "status", "statusMessage");
}


static PyMemberDef ProtocolError_members[] = {
    {"status", T_OBJECT_EX, offsetof(ProtocolErrorObject, status), 0,
        PyDoc_STR("status code")},
    {"statusMessage", T_OBJECT_EX, offsetof(ProtocolErrorObject, statusMessage), 0,
        PyDoc_STR("status message string")},
    {"method", T_OBJECT_EX, offsetof(ProtocolErrorObject, method), 0,
        PyDoc_STR("Method that caused the exception.")},
    {NULL}  /* Sentinel */
};

ComplexException(ProtocolError, ProtocolError, 0, ProtocolError_members,
    "ProtocolError exception");


//*************************************************************************
//**Fault
//*************************************************************************

typedef struct {
#if PY_MAJOR_VERSION >= 3
    PyException_HEAD
#else
    PyObject_HEAD
    PyObject *dict;
    PyObject *args;
    PyObject *message;
#endif

    PyObject *faultCode;
    PyObject *faultString;
    PyObject *method;
} FaultObject;

static int
Fault_init(FaultObject *self, PyObject *args, PyObject *kwds)
{
    if (((PyTypeObject*)PyExc_BaseException)->tp_init((PyObject *)self, args, kwds) == -1)
        return -1;

    self->method = Py_None;
    if (!PyArg_ParseTuple(args, "OO|O:__init__",
            &self->faultCode, &self->faultString, &self->method))
        return -1;

    Py_INCREF(self->faultCode);
    Py_INCREF(self->faultString);
    Py_INCREF(self->method);

    if (!PyNumber_Check(self->faultCode))
    {
        PyErr_Format(PyExc_TypeError,
            "Fault.self->faultCode must be number, not %s.",
            Py_TYPE(self->faultCode)->tp_name);
        return -1;
    }

#if PY_MAJOR_VERSION >= 3
    if (!PyUnicode_Check(self->faultString))
#else
    if (!(PyString_Check(self->faultString) || PyUnicode_Check(self->faultString)))
#endif
    {
        PyErr_Format(PyExc_TypeError,
            "Fault.faultString must be string "
            "or unicode, not %s.",
            Py_TYPE(self->faultString)->tp_name);
        return -1;
    }

    return 0;
}


static int
Fault_clear(FaultObject *self)
{
    Py_CLEAR(self->faultCode);
    Py_CLEAR(self->faultString);
    Py_CLEAR(self->method);
    return ((PyTypeObject*)PyExc_BaseException)->tp_clear((PyObject *)self);
}

static void
Fault_dealloc(FaultObject *self)
{
    Fault_clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int
Fault_traverse(FaultObject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->faultCode);
    Py_VISIT(self->faultString);
    Py_VISIT(self->method);
    return ((PyTypeObject*)PyExc_BaseException)->tp_traverse((PyObject *)self, visit, arg);
}


static PyObject *
Fault_repr(FaultObject *self)
{
    return errorRepr((PyObject*)self, "Fault", "faultCode", "faultString");
}

static PyMemberDef Fault_members[] = {
    {"faultCode", T_OBJECT_EX, offsetof(FaultObject, faultCode), 0,
        PyDoc_STR("fault code")},
    {"faultString", T_OBJECT_EX, offsetof(FaultObject, faultString), 0,
        PyDoc_STR("fault string")},
    {"method", T_OBJECT_EX, offsetof(FaultObject, method), 0,
        PyDoc_STR("Method that caused the exception.")},
    {NULL}  /* Sentinel */
};

ComplexException(Fault, Fault, 0, Fault_members, "Fault exception");


//*************************************************************************
//*ResponseError
//*************************************************************************

typedef struct {
#if PY_MAJOR_VERSION >= 3
    PyException_HEAD
#else
    PyObject_HEAD
    PyObject *dict;
    PyObject *args;
    PyObject *message;
#endif

    PyObject *statusMessage;
    PyObject *method;
} ResponseErrorObject;

static int
ResponseError_init(ResponseErrorObject *self, PyObject *args, PyObject *kwds)
{
    if (((PyTypeObject*)PyExc_BaseException)->tp_init((PyObject *)self, args, kwds) == -1)
        return -1;

    self->method = Py_None;
    if (!PyArg_ParseTuple(args, "O|O:__init__",
            &self->statusMessage, &self->method))
        return -1;

    Py_XINCREF(self->statusMessage);
    Py_XINCREF(self->method);

#if PY_MAJOR_VERSION >= 3
    if (!PyUnicode_Check(self->statusMessage))
#else
    if (!(PyString_Check(self->statusMessage) || PyUnicode_Check(self->statusMessage)))
#endif
    {
        PyErr_Format(PyExc_TypeError,
            "ResponseError.statusMessage must be string "
            "or unicode, not %s.",
            Py_TYPE(self->statusMessage)->tp_name);
        return -1;
    }

    return 0;
}

static int
ResponseError_clear(ResponseErrorObject *self)
{
    Py_CLEAR(self->statusMessage);
    Py_CLEAR(self->method);
    return ((PyTypeObject*)PyExc_BaseException)->tp_clear((PyObject *)self);
}

static void
ResponseError_dealloc(ResponseErrorObject *self)
{
    ResponseError_clear(self);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static int
ResponseError_traverse(ResponseErrorObject *self, visitproc visit, void *arg)
{
    Py_VISIT(self->statusMessage);
    Py_VISIT(self->method);
    return ((PyTypeObject*)PyExc_BaseException)->tp_traverse((PyObject *)self, visit, arg);
}

static PyObject *
ResponseError_repr(ResponseErrorObject *self)
{
    return errorRepr((PyObject*)self, "ResponseError", 0, "statusMessage");
}


static PyMemberDef ResponseError_members[] = {
    {"statusMessage", T_OBJECT_EX, offsetof(ResponseErrorObject, statusMessage), 0,
        PyDoc_STR("status message string")},
    {"method", T_OBJECT_EX, offsetof(ResponseErrorObject, method), 0,
        PyDoc_STR("Method that caused the exception.")},
    {NULL}  /* Sentinel */
};

ComplexException(ResponseError, ResponseError, 0, ResponseError_members,
    "ResponseError exception");

namespace FRPC { namespace Python {

    PyObject *Error;
    PyObject *ProtocolError;
    PyObject *Fault;
    PyObject *ResponseError;

    int initErrors(PyObject *fastrpc_module) {
        /**************************************************************/
        //Error init
        /**************************************************************/
        if (PyType_Ready(&ErrorException) < 0)
            return -1;

        Py_INCREF(&ErrorException);
        if (PyModule_AddObject(fastrpc_module, "Error",
                           reinterpret_cast<PyObject*>(&ErrorException)))
            return -1;
        Error = reinterpret_cast<PyObject*>(&ErrorException);

        /********************************************************************/
        //ProtocolError
        /**************************************************************/
        if (PyType_Ready(&ProtocolErrorException) < 0)
            return -1;

        Py_INCREF(&ProtocolErrorException);
        if (PyModule_AddObject(fastrpc_module, "ProtocolError",
                           reinterpret_cast<PyObject*>(&ProtocolErrorException)))
            return -1;
        ProtocolError = reinterpret_cast<PyObject*>(&ProtocolErrorException);

        /********************************************************************/
        //Fault
        /********************************************************************/
        if (PyType_Ready(&FaultException) < 0)
            return -1;

        Py_INCREF(&FaultException);
        if (PyModule_AddObject(fastrpc_module, "Fault",
                           reinterpret_cast<PyObject*>(&FaultException)))
            return -1;
        Fault = reinterpret_cast<PyObject*>(&FaultException);

        /********************************************************************/
        // ResponseError
        /**************************************************************/
        if (PyType_Ready(&ResponseErrorException) < 0)
            return -1;

        Py_INCREF(&ResponseErrorException);
        if (PyModule_AddObject(fastrpc_module, "ResponseError",
                           reinterpret_cast<PyObject*>(&ResponseErrorException)))
            return -1;
        ResponseError = reinterpret_cast<PyObject*>(&ResponseErrorException);

        return 0;
    }

} } // namespace FRPC::Python
