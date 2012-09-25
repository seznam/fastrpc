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
 * $Id$
 *
 * AUTHOR      Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * DESCRIPTION
 * Python FastRPC support. Python exports.
 *
 * HISTORY
 *      2012-09-18 (ondrej holecek)
 *              Created
 */

#ifndef FRPCPYTHONHELPER_H
#define FRPCPYTHONHELPER_H

#include <Python.h>
#include <frpcmarshaller.h>
#include <frpcdatabuilder.h>

namespace FRPC {
namespace Python {

enum StringMode_t {
    STRING_MODE_INVALID = -1,
    STRING_MODE_MIXED = 0,
    STRING_MODE_UNICODE,
    STRING_MODE_STRING
};

class FeederInterface_t {
public:
    virtual ~FeederInterface_t() {}
    virtual void feed(PyObject *args) = 0;
    virtual void feedValue(PyObject *value) = 0;
};

class BuilderInterface_t {
public:
    virtual ~BuilderInterface_t() {}
    virtual FRPC::DataBuilder_t * builder() = 0;
    virtual PyObject * getRetValue() = 0;
};

} // namespace Python
} // namespace FRPC


extern "C" {
    // builder - create/destroy C function
    typedef FRPC::Python::BuilderInterface_t * (create_frpc_python_builder_t)
        (PyObject *methodObject, FRPC::Python::StringMode_t smode);

    typedef void (destroy_frpc_python_builder_t)(FRPC::Python::BuilderInterface_t *builder);


    // feeder - create/destroy C function
    typedef FRPC::Python::FeederInterface_t * (create_frpc_python_feeder_t)
            (FRPC::Marshaller_t *marshaller,const std::string *encoding);

    typedef void (destroy_frpc_python_feeder_t)(FRPC::Python::FeederInterface_t *feeder);
} // extern "C"


#endif

