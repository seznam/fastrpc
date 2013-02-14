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
 * $Id: pyobjectwrapper.h,v 1.4 2007-04-02 15:42:58 vasek Exp $
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

    PyObject* operator->() {
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
