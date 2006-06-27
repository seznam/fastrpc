/*
 * FastRPC - RPC protocol suport Binary and XML.
 * Copyright (C) 2005 Seznam.cz, a.s.
 *
 * $Id: pyobjectwrapper.h,v 1.2 2006-06-27 12:04:06 vasek Exp $
 *
 * AUTHOR      Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * DESCRIPTION
 * Python FastRPC support. Object wrapper.
 *
 * HISTORY
 *      2006-05-19 (vasek)
 *              Created
 */


#ifndef PYOBJECTWRAPPER_H_
#define PYOBJECTWRAPPER_H_

namespace FRPC { namespace Python {

class PyObjectWrapper_t {
public:
    PyObjectWrapper_t(PyObject *object = 0, bool incref = false)
            : object(object)
    {
        if (incref) inc();
    }

    PyObjectWrapper_t(const PyObjectWrapper_t &o)
            : object(o.object)
    {
        inc();
    }

    PyObjectWrapper_t& operator=(const PyObjectWrapper_t &o)
    {
        if (this != &o) {
            dec();
            object = o.object;
        }
        inc();
        return *this;
    }

    ~PyObjectWrapper_t() {
        Py_XDECREF(object);
    }

    PyObject*& inc() {
        Py_XINCREF(object);
        return object;
    }

    PyObject* inc() const {
        Py_XINCREF(object);
        return object;
    }

    void dec() {
        Py_XDECREF(object);
    }

    operator PyObject*&() {
        return object;
    }

    PyObject*& get() {
        return object;
    }

    PyObject*& operator*() {
        return object;
    }

    bool operator!() const{
        return !object;
    }

    PyObject** addr() {
        return &object;
    }

    template <typename object_type> object_type* get() {
        return reinterpret_cast<object_type*>(object);
    }

    PyObject *object;
};

} } // namespace FRPC::Python

#endif // PYOBJECTWRAPPER_H_
