/*
 * FastRPC - RPC protocol suport Binary and XML.
 * Copyright (C) 2005 Seznam.cz, a.s.
 *
 * $Id: pythonfeeder.h,v 1.2 2006-06-27 12:04:06 vasek Exp $
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


#ifndef PYTHONFEEDER_H_
#define PYTHONFEEDER_H_

#include <string>
#include <vector>

#include <Python.h>

#include <frpcmarshaller.h>

namespace FRPC { namespace Python {

class  Feeder_t {
public:
    Feeder_t(FRPC::Marshaller_t *marshaller,const std::string &encoding)
            :marshaller(marshaller), encoding(encoding)
    {}

    void feed(PyObject *args);
    void feedValue(PyObject *value);

private:
    Feeder_t();
    FRPC::Marshaller_t *marshaller;
    const std::string encoding;
};

} } // namespace FRPC::Python

#endif // PYTHONFEEDER_H_
