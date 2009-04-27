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
 * $Id: pyerrors.cc,v 1.6 2009-04-27 04:34:43 burlog Exp $
 *
 * AUTHOR      Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * DESCRIPTION
 * Python FastRPC support. Errors.
 *
 * HISTORY
 *      2006-05-22 (vasek)
 *              Created
 */


#include <string>

#include "fastrpcmodule.h"

using namespace FRPC::Python;

//******************************************************************
/***Error */
//******************************************************************
extern "C"
{

    static PyObject* Error__init__(PyObject *, PyObject *)
    {
        Py_INCREF(Py_None);
        return Py_None;
    }

    static PyObject* Error__repr__(PyObject *, PyObject *)
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

namespace {
PyObject* initException(PyObject *module, const char *name,
                        const char *niceName, PyObject *base,
                        PyMethodDef *methodDef)
{
    // create runtime error
    PyObject *Exception = PyErr_NewException((char *)niceName, base, 0);
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
        if (!method)
            return 0;
        if (PyObject_SetAttrString(Exception, md->ml_name, method))
            return 0;
    }

    // add this exception into the xmlrpcserver module
    if (PyModule_AddObject(module, (char *)name, Exception))
        return 0;

    // OK
    return Exception;
}
}

namespace FRPC { namespace Python {

    PyObject *Error;
    PyObject *ProtocolError;
    PyObject *Fault;
    PyObject *ResponseError;

    int initErrors(PyObject *fastrpc_module) {
        /**************************************************************/
        //Error init
        /**************************************************************/
        Error = initException(fastrpc_module, "Error", "fastrpc.Error",
                              PyExc_Exception, ErrorMethod_methods);
        if (!Error) return -1;

        /********************************************************************/
        //ProtocolError
        /**************************************************************/
        ProtocolError = initException(fastrpc_module, "ProtocolError",
                                      "fastrpc.ProtocolError",
                                      Error, ProtocolErrorMethod_methods);
        if (!ProtocolError) return -1;

        /********************************************************************/
        //fault
        /********************************************************************/
        Fault = initException(fastrpc_module, "Fault", "fastrpc.Fault",
                              Error, FaultMethod_methods);
        if (!Fault) return -1;

        /********************************************************************/
        // ResponseError
        /**************************************************************/
        ResponseError = initException(fastrpc_module, "ResponseError",
                                      "fastrpc.ResponseError",
                                      Error, ResponseErrorMethod_methods);
        if (!ResponseError) return -1;

        return 0;
    }

} } // namespace FRPC::Python
