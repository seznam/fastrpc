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
 * $Id: pythonserver.cc,v 1.22 2008-11-18 13:23:18 burlog Exp $
 *
 * AUTHOR      Vaclav Blazek <blazek@firma.seznam.cz>
 *
 * DESCRIPTION
 * Python FastRPC support. Mimics C++ FastRPC server and method registry.
 *
 * HISTORY
 *      2006-05-19 (vasek)
 *              Created
 */

#define __ENABLE_WSTRING

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <list>
#include <memory>
#include <algorithm>
#include <functional>

#include <Python.h>
#include <structmember.h>
#include <methodobject.h>

#include <frpcmethodregistry.h>
#include <frpchttpio.h>
#include <frpchttp.h>
#include <frpcwriter.h>
#include <frpc.h>
#include <frpchttperror.h>
#include <frpcserver.h>
#include <frpcstreamerror.h>
#include <frpcprotocolerror.h>
#include <frpcmarshaller.h>
#include <frpcunmarshaller.h>
#include <frpchttpclient.h>
#include <frpcfault.h>
#include <frpc.h>

#include "fastrpcmodule.h"
#include "pythonbuilder.h"
#include "pythonfeeder.h"

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)
typedef int Py_ssize_t;
#define PY_SSIZE_T_MAX INT_MAX
#define PY_SSIZE_T_MIN INT_MIN
#endif

using FRPC::ProtocolVersion_t;
using FRPC::Python::PyError_t;
using FRPC::Python::PyObjectWrapper_t;
using FRPC::Python::Fault;
using FRPC::Python::Builder_t;
using FRPC::Python::Feeder_t;

namespace {
    template <typename type>
    PyObject* asObject(type *o) {
        return reinterpret_cast<PyObject*>(o);
    }

    template <typename type>
    PyTypeObject* asTypeObject(type *o) {
        return reinterpret_cast<PyTypeObject*>(o);
    }

    template <typename Object>
    Object* newObject(PyTypeObject &type, PyObject *args, PyObject *kwds)
    {
        PyObject *object = type.tp_new(&type, args, kwds);
        if (!object) return 0;

        if (type.tp_init && type.tp_init(object, args, kwds)) {
            Py_DECREF(object);
            return 0;
        }

        // OK
        return reinterpret_cast<Object*>(object);
    }

    PyObject *stringToSignature(const std::string &signature) {
        PyObjectWrapper_t array(PyList_New(0));
        if (!array) return 0;
        PyObjectWrapper_t current(PyList_New(0));
        if (!current) return 0;
        if (PyList_Append(array, current)) return 0;

        int currentIndex = 0;

        for (std::string::const_iterator isignature = signature.begin();
             isignature != signature.end(); ++isignature, ++currentIndex) {
            std::string member("???");
            switch (*isignature) {
            case 'A':
                member = "array";
                break;

            case 'S':
                member = "struct";
                break;

            case 'B':
                member = "binary";
                break;

            case 'D':
                member = "dateTime";
                break;

            case 'b':
                member = "bool";
                break;

            case 'd':
                member = "double";
                break;

            case 'i':
                member = "int";
                break;

            case 's':
                member = "string";
                break;

            case ':':
                if (currentIndex != 1) {
                    PyErr_Format
                        (PyExc_ValueError,
                         "Return value separator should be second character "
                         " in signature part at position %zd in signature "
                         "\"%s\".",
                         (isignature - signature.begin()), signature.c_str());
                    return 0;
                }
                continue;

            case ',':
                // create new member and switch to it
                if (currentIndex < 2) {
                    PyErr_Format
                        (PyExc_ValueError,
                         "Premature end of signature part at position %zd "
                         "in signature \"%s\".",
                         (isignature - signature.begin()), signature.c_str());
                    return 0;
                }

                {
                    current = PyList_New(0);
                    if (!current) return 0;
                    if (PyList_Append(array, current)) return 0;
                    currentIndex = -1;
                }
                continue;

            default:
                PyErr_Format(PyExc_ValueError,
                             "Invalid character '%c' at position %zd"
                             " in signature \"%s\".",
                             *isignature, (isignature - signature.begin()),
                             signature.c_str());
                return 0;
            }

            // check
            if (currentIndex == 1) {
                PyErr_Format
                    (PyExc_ValueError,
                     "Second character in signature part should be ':' "
                     "not '%c' at position %zd in signature \"%s\".",
                     *isignature, (isignature - signature.begin()),
                     signature.c_str());
                return 0;
            }

            // append
            PyObjectWrapper_t pymember(PyString_FromString(member.c_str()));
            if (!pymember) return 0;
            if (PyList_Append(current, pymember)) return 0;
        }

        // OK
        return array.inc();
    }

    PyObject *stringToSignature(PyObject *signature) {
        Py_ssize_t size;
        char *buffer;
        if (PyString_AsStringAndSize(signature, &buffer, &size) == -1)
            return 0;
        return stringToSignature(std::string(buffer, size));
    }

    struct Method_t {
        Method_t(const std::string &name, PyObject *callback,
                 PyObject *signature, PyObject *help, PyObject *context = 0);

        Method_t(const std::string &name, PyObject *callback,
                 const std::string &signature, const std::string &help,
                 PyObject *context = 0);

        PyObject* operator()(PyObject *runtime, PyObject *args) const;

        std::string name;
        PyObjectWrapper_t callback;
        PyObjectWrapper_t signature;
        PyObjectWrapper_t help;
        PyObjectWrapper_t context;
    };

    typedef std::map<std::string, Method_t> MethodLookup_t;

    struct MethodRegistryObject {
        PyObject_HEAD

        PyObject *preProcess;
        PyObject *postProcess;
        PyObject *preRead;

        MethodLookup_t *lookup;
        PyObject *defaultMethod;
        PyObject *defaultListMethods;
        PyObject *defaultMethodHelp;
        PyObject *defaultMethodSignature;
        PyObject *headMethod;

        int introspectionEnabled;
    };

    // fwd
    class Server_t;

    struct ServerObject {
        PyObject_HEAD

        FRPC::Python::StringMode_t stringMode;

        int readTimeout;
        int writeTimeout;
        int keepAlive;
        int maxKeepalive;
        int useBinary;

        MethodRegistryObject *registry;

        // the server engine
        Server_t *server;
    };

    PyObject *emptyTuple = 0;
    PyObject *emptyDict = 0;
    PyObject *emptyString = 0;
    PyObject *Fault_message_format = 0;

    PyObjectWrapper_t None(Py_None, true);

    void fetchException(PyObjectWrapper_t &type, PyObjectWrapper_t &value,
                        PyObjectWrapper_t &traceback)
    {
        PyErr_Fetch(&*type, &*value, &*traceback);
        if (!type) type = None;
        if (!value) value = None;
        if (!traceback) traceback = None;
    }

    PyObject* makeFault(int faultCode, const char *faultMessage, ...) {
        char buf[512];
        va_list valist;
        va_start(valist, faultMessage);
        vsnprintf(buf, sizeof(buf), faultMessage, valist);
        va_end(valist);

        PyObjectWrapper_t faultArgs = Py_BuildValue("(is)", faultCode, buf);
        if (!faultArgs) return 0;

        return PyInstance_New(Fault, faultArgs, 0);
    }

    PyObject* makeFault(const std::string &msg, PyObjectWrapper_t type,
                        PyObjectWrapper_t value, PyObjectWrapper_t)
    {
        if (PyErr_GivenExceptionMatches(type, Fault)) {
            // we've got Fault => do not touch it
            return value.inc();
        }

        // create message string
        PyObjectWrapper_t formatArgs =
            Py_BuildValue("(s#O&O&)", msg.data(), msg.size(),
                          PyObject_Str, *type,
                          PyObject_Str, *value);
        if (!formatArgs) return 0;

        PyObjectWrapper_t message =
            PyString_Format(Fault_message_format, formatArgs);
        if (!message) return 0;

        PyObjectWrapper_t faultArgs =
            Py_BuildValue("(lO)", FRPC::MethodRegistry_t::FRPC_INTERNAL_ERROR,
                          *message);
        if (!faultArgs) return 0;

        return PyInstance_New(Fault, faultArgs, 0);
    }

    PyObject* makeFault(const std::string &msg) {
        PyObjectWrapper_t type;
        PyObjectWrapper_t value;
        PyObjectWrapper_t traceback;

        fetchException(type, value, traceback);
        return makeFault(msg, type, value, traceback);
    }

    PyObject* makeFaultStruct(int faultCode, const char *faultMessage, ...) {
        char buf[512];
        va_list valist;
        va_start(valist, faultMessage);
        vsnprintf(buf, sizeof(buf), faultMessage, valist);
        va_end(valist);

        return Py_BuildValue("{s:i,s:s}",
                             "faultCode", faultCode,
                             "faultString", buf);
    }

    PyObject* faultToStruct(PyObjectWrapper_t fault) {
        PyObjectWrapper_t faultCode
            (PyObject_GetAttrString(fault, "faultCode"));
        if (!faultCode) faultCode = 0;
        PyObjectWrapper_t faultString
            (PyObject_GetAttrString(fault, "faultString"));
        if (!faultString)
            faultString = PyString_FromString("<cannot fetch>");
        // we've got Fault => convert to structure
        return Py_BuildValue("{s:O,s:O}",
                             "faultCode", faultCode.object,
                             "faultString", faultString.object);
    }

    PyObject* makeFaultStruct(const std::string &msg, PyObjectWrapper_t type,
                              PyObjectWrapper_t value,
                              PyObjectWrapper_t)
    {
        if (PyErr_GivenExceptionMatches(type, Fault))
            return faultToStruct(value);

        // create message string
        PyObjectWrapper_t formatArgs =
            Py_BuildValue("(s#O&O&)", msg.data(), msg.size(),
                          PyObject_Str, *type,
                          PyObject_Str, *value);
        if (!formatArgs) return 0;

        PyObjectWrapper_t message =
            PyString_Format(Fault_message_format, formatArgs);
        if (!message) return 0;

        return Py_BuildValue("{s:l,s:O}",
                             "faultCode",
                             FRPC::MethodRegistry_t::FRPC_INTERNAL_ERROR,
                             "faultString", *message);
    }

    FRPC::Fault_t makeCPPFault(int faultCode,
                               const std::string &msg,
                               PyObjectWrapper_t type,
                               PyObjectWrapper_t value,
                               PyObjectWrapper_t)
    {
        // create message string
        PyObjectWrapper_t formatArgs =
            Py_BuildValue("(s#O&O&)", msg.data(), msg.size(),
                          PyObject_Str, *type,
                          PyObject_Str, *value);
        if (!formatArgs) throw PyError_t();

        PyObjectWrapper_t pyMessage =
            PyString_Format(Fault_message_format, formatArgs);

        char *faultStgring;
        Py_ssize_t faultStringSize;
        if (PyString_AsStringAndSize(pyMessage, &faultStgring,
                                     &faultStringSize))
            throw PyError_t();

        // OK
        return FRPC::Fault_t(faultCode,
                             std::string(faultStgring, faultStringSize));
    }

    FRPC::Fault_t makeCPPFault(int faultCode, const std::string &msg) {
        PyObjectWrapper_t type;
        PyObjectWrapper_t value;
        PyObjectWrapper_t traceback;

        fetchException(type, value, traceback);
        return makeCPPFault(faultCode, msg, type, value, traceback);
    }

    class Server_t : public FRPC::Writer_t {
    public:
        Server_t(ServerObject *serverObject)
            : FRPC::Writer_t(), serverObject(serverObject),
              io(0, serverObject->readTimeout, serverObject->writeTimeout,
                 -1 ,-1),
              outType(FRPC::Server_t::XML_RPC), closeConnection(true),
              contentLength(0), useChunks(false), headersSent(false),
              head(false), useBinary(serverObject->useBinary)
        {}

        ~Server_t() {}

        PyObject* serve(int fd, PyObjectWrapper_t addr);

    private:
        void readRequest(FRPC::DataBuilder_t &builder);

        /**
         * @brief says to server that all data was writed
         *
         */
        virtual void flush();

        /**
         * @brief write data to server
         * @param data pointer to data
         * @param size size of data
         */
        virtual void write(const char* data, unsigned int size);

        /**
         * @brief send response to client
         *
         */
        void sendResponse();

        /**
         * @brief send HTTP header with HTTPError_t to client
         *
         */
        void sendHttpError(const FRPC::HTTPError_t &httpError);


        /** Python server object. Needed to access settings and registry.
         */
        ServerObject *serverObject;

        FRPC::HTTPIO_t io;
        long outType;

        std::list<std::string> queryStorage;

        bool closeConnection;
        unsigned long contentLength;
        bool  useChunks;
        bool headersSent;
        bool head;
        ProtocolVersion_t protocolVersion;
        bool useBinary;
    };
}

/************************************************************************/
/****                             Server                             ****/
/************************************************************************/

#define DECL_METHOD(type, name) \
    PyObject* type ## _ ## name(type *self, PyObject *args)

#define DECL_METHOD_KWD(type, name) \
    PyObject* type ## _ ## name(type *self, PyObject *args, PyObject *kwds)

extern "C" {
    // method registry
    static PyObject* MethodRegistryObject_new(PyTypeObject *self,
                                              PyObject *args,
                                              PyObject *kwds);

    static void MethodRegistryObject_dealloc(MethodRegistryObject *self);

    static DECL_METHOD_KWD(MethodRegistryObject, register);

    static DECL_METHOD_KWD(MethodRegistryObject, registerDefault);

    static DECL_METHOD(MethodRegistryObject, registerHead);

    static DECL_METHOD(MethodRegistryObject, dispatch);

    static PyObject* dispatchCall(MethodRegistryObject *self,
                                  const char *name, PyObject *clientIP,
                                  PyObject *params);

    // introspection api
    static DECL_METHOD(MethodRegistryObject, system_listMethods);
    static DECL_METHOD(MethodRegistryObject, system_methodSignature);
    static DECL_METHOD(MethodRegistryObject, system_methodHelp);

    // other extensions
    static DECL_METHOD(MethodRegistryObject, system_multicall);

    // server
    static PyObject* ServerObject_new(PyTypeObject *self, PyObject *args,
                                       PyObject *kwds);
    static void ServerObject_dealloc(ServerObject *self);

    static DECL_METHOD(ServerObject, serve);
};

#define OFF(x) offsetof(MethodRegistryObject, x)

static PyMemberDef MethodRegistryObject_members[] = {
    {(char *)"preProcess",           T_OBJECT,    OFF(preProcess),           0},
    {(char *)"postProcess",          T_OBJECT,    OFF(postProcess),          0},
    {(char *)"preRead",              T_OBJECT,    OFF(preRead),              0},
    {(char *)"introspectionEnabled", T_INT,       OFF(introspectionEnabled), 0},
    {0}  // Sentinel
};

#undef OFF

static PyMethodDef MethodRegistryObject_methods[] = {
    {
        "register",
        reinterpret_cast<PyCFunction>(MethodRegistryObject_register),
        METH_VARARGS | METH_KEYWORDS,
        "Register method callback."
    }, {
        "registerDefault",
        reinterpret_cast<PyCFunction>(MethodRegistryObject_registerDefault),
        METH_VARARGS | METH_KEYWORDS,
        "Register default method callback."
    }, {
        "registerHead",
        reinterpret_cast<PyCFunction>(MethodRegistryObject_registerHead),
        METH_VARARGS,
        "Register HTTP HEAD method callback."
    }, {
        "dispatch",
        reinterpret_cast<PyCFunction>(MethodRegistryObject_dispatch),
        METH_VARARGS,
        "Dispatch method call to callback."
    }, {
        "__system.listMethods",
        reinterpret_cast<PyCFunction>(MethodRegistryObject_system_listMethods),
        METH_VARARGS,
        "Introspection API."
    }, {
        "__system.methodSignature",
        reinterpret_cast<PyCFunction>
        (MethodRegistryObject_system_methodSignature),
        METH_VARARGS,
        "Introspection API."
    }, {
        "__system.methodHelp",
        reinterpret_cast<PyCFunction>
        (MethodRegistryObject_system_methodHelp),
        METH_VARARGS,
        "Introspection API."
    }, {
        "__system.multicall",
        reinterpret_cast<PyCFunction>
        (MethodRegistryObject_system_multicall),
        METH_VARARGS,
        "Multicall support."
    },

    { 0, 0, 0, 0 } // sentinel
};

static PyTypeObject MethodRegistryObject_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                      /*ob_size*/
    "MethodRegistry",                       /*tp_name*/
    sizeof (MethodRegistryObject),          /*tp_basicsize*/
    0,                                      /*tp_itemsize*/
    /* methods */
    (destructor) MethodRegistryObject_dealloc,  /*tp_dealloc*/
    0,                                      /*tp_print*/
    0,                                      /*tp_getattr*/
    0,                                      /*tp_setattr*/
    0,                                      /*tp_compare*/
    0,                                      /*tp_repr*/
    0,                                      /*tp_as_number*/
    0,                                      /*tp_as_sequence*/
    0,                                      /*tp_as_mapping*/
    0,                                      /*tp_hash*/
    0,                                      /* tp_call */
    0,                                      /* tp_str */
    PyObject_GenericGetAttr,                /* tp_getattro */
    PyObject_GenericSetAttr,                /* tp_setattro */
    0,                                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                     /* tp_flags */
    0,                                      /* tp_doc */
    0,                                      /* tp_traverse */
    0,                                      /* tp_clear */
    0,                                      /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */
    MethodRegistryObject_methods,           /* tp_methods */
    MethodRegistryObject_members,           /* tp_members */
    0,                                      /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,                                      /* tp_descr_get */
    0,                                      /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    (initproc)0,                            /* tp_init */
    PyType_GenericAlloc,                    /* tp_alloc */
    MethodRegistryObject_new,                       /* tp_new */
    _PyObject_Del                           /* tp_free */
};


/************************************************************************/
/****                    Server logic implementation                 ****/
/************************************************************************/

namespace {
    const long BUFFER_SIZE = 1 << 16;

    std::string getException() {
        // fetch exception
        PyObjectWrapper_t type;
        PyObjectWrapper_t value;
        PyObjectWrapper_t tb;
        PyErr_Fetch(type.addr(), value.addr(), tb.addr());

        // prepar exception for format
        PyObjectWrapper_t format(PyString_FromString("%s: %s"));
        if (!format) return "Cannot format exception";

        PyObjectWrapper_t args
            (Py_BuildValue("(OO)", type.get(), value.get()));
        if (!args) return "Cannot format exception";

        // format exception
        PyObjectWrapper_t message(PyString_Format(format, args));
        if (!message) return "Cannot format exception";

        // convert message to utf-8 if unicode string
        if (PyUnicode_Check(message)) {
            message = PyUnicode_AsEncodedString(message, "utf-8", "replace");
            if (!message) return "Cannot decode exception from unicode";
        }

        // fetch string as C string
        Py_ssize_t size;
        char *buf;
        PyString_AsStringAndSize(message, &buf, &size);

        // OK
        return std::string(buf, size);
    }
}

PyObject* Server_t::serve(int fd, PyObjectWrapper_t addr) {

    // prepare query storage
    queryStorage.push_back(std::string());
    queryStorage.back().reserve(BUFFER_SIZE);
    contentLength = 0;
    closeConnection = false;
    headersSent = false;
    head = false;

    // call exception
    PyObjectWrapper_t type;
    PyObjectWrapper_t value;
    PyObjectWrapper_t traceback;

    io.setSocket(fd);

    if (addr == Py_None) {
        struct sockaddr_in clientaddr;
        socklen_t sinSize = sizeof (clientaddr);
        std::string clientIp = "unknown";
        if (!getpeername(fd, reinterpret_cast<struct sockaddr*>(&clientaddr),
                     &sinSize))
            clientIp = inet_ntoa(clientaddr.sin_addr);

        addr = Py_BuildValue("(si)", clientIp.c_str(),
                            ntohs(clientaddr.sin_port));
        if (!addr) return 0;
    }

    long requestCount = 0;

    do {
        Builder_t builder(0, serverObject->stringMode);

        try {
            // call preprocessor
            if (serverObject->registry->preRead != Py_None) {
                if (!PyObject_CallFunction(
                     serverObject->registry->preRead, (char *)"()")) {
            // exception thrown => fetch exception
                         fetchException(type, value, traceback);
                         makeFault("Unhandled exception in preread ",
                                            type, value, traceback);
                         return 0;
                     }
            }

            readRequest(builder);
        } catch (const FRPC::StreamError_t &streamError) {
            std::auto_ptr<FRPC::Marshaller_t> marshaller
                (FRPC::Marshaller_t::create
                 (((outType == FRPC::Server_t::BINARY_RPC)
                   ? FRPC::Marshaller_t::BINARY_RPC
                   : FRPC::Marshaller_t::XML_RPC), *this,protocolVersion));

            marshaller->packFault(FRPC::MethodRegistry_t::FRPC_PARSE_ERROR,
                                  streamError.message().c_str());
            marshaller->flush();
            continue;
        } catch (const FRPC::HTTPError_t &httpError) {
            sendHttpError(httpError);
            break;
        } catch (const FRPC::ProtocolError_t &pe) {
            PyObject *args = Py_BuildValue
                ("(is#)", pe.errorNum(), pe.message().data(),
                 pe.message().size());
            if (!args) return 0;
            PyErr_SetObject(FRPC::Python::ProtocolError, args);
            return 0;
        }

        try {
            if (head) {
                if (serverObject->registry->headMethod) {
                    PyObjectWrapper_t result
                        (PyObject_CallFunctionObjArgs
                         (serverObject->registry->headMethod, 0));
                    if (!result) {
                        throw FRPC::HTTPError_t
                            (FRPC::HTTP_INTERNAL_SERVER_ERROR,
                             "Error while running head method: <%s>.",
                             getException().c_str());
                    }

                    long int r = PyInt_AsLong(result);
                    if ((r == -1) && PyErr_Occurred()) {
                        throw FRPC::HTTPError_t
                            (FRPC::HTTP_INTERNAL_SERVER_ERROR,
                             "Head should return number: <%s>.",
                             getException().c_str());
                    }

                    if (r)
                        throw FRPC::HTTPError_t(FRPC::HTTP_SERVICE_UNAVAILABLE,
                                                "Service Unavailable");
                    flush();
                } else {
                    throw FRPC::HTTPError_t(FRPC::HTTP_METHOD_NOT_ALLOWED,
                                            "Method Not Allowed");
                }
            } else {
                std::auto_ptr<FRPC::Marshaller_t>
                    marshaller(FRPC::Marshaller_t::create
                               (((outType == FRPC::Server_t::BINARY_RPC)
                                ? FRPC::Marshaller_t::BINARY_RPC
                                : FRPC::Marshaller_t::XML_RPC), *this,
                                protocolVersion));
                Feeder_t feeder(marshaller.get(), "utf-8");

                PyObjectWrapper_t result
                    (dispatchCall(serverObject->registry,
                                  builder.getStringMethodName()->c_str(),
                                  addr, builder.getUnMarshaledData()));

                try {
                    if (PyObject_IsInstance(result, Fault) > 0) {
                        PyObjectWrapper_t pyFaultCode
                            (PyObject_GetAttrString(result, "faultCode"));
                        if (!pyFaultCode) throw PyError_t();

                        long int faultCode = PyInt_AsLong(pyFaultCode);
                        if ((faultCode == -1) && PyErr_Occurred())
                            throw PyError_t();

                        PyObjectWrapper_t pyFaultString
                            (PyObject_GetAttrString(result, "faultString"));
                        if (!pyFaultString) throw PyError_t();
                        char *faultString;
                        Py_ssize_t faultStringSize;
                        if (PyString_AsStringAndSize(pyFaultString,
                                                     &faultString,
                                                     &faultStringSize))
                            throw PyError_t();

                        marshaller->packFault(faultCode, faultString,
                                              faultStringSize);
                    } else {
                        marshaller->packMethodResponse();
                        feeder.feedValue(result);
                    }
                    marshaller->flush();
                } catch (const PyError_t &) {
                    // oops error during processing result
                    throw FRPC::HTTPError_t
                        (FRPC::HTTP_INTERNAL_SERVER_ERROR,
                         "Error while dispatching call: <%s>.",
                         getException().c_str());
                } catch (const FRPC::StreamError_t &streamError) {
                    // oops error during marshalling
                    PyErr_SetString(PyExc_RuntimeError,
                                   "Error marshalling result, bailing out.");
                    return 0;
                } catch (const FRPC::ProtocolError_t &pe) {
                    PyObject *args = Py_BuildValue
                        ("(is#)", pe.errorNum(), pe.message().data(),
                         pe.message().size());
                    if (!args) return 0;
                    PyErr_SetObject(FRPC::Python::ProtocolError, args);
                    return 0;
                }
            }
        } catch (const FRPC::HTTPError_t &httpError) {
            sendHttpError(httpError);
            break;
        } catch (const PyError_t &) {
            // oops, pass error upwards
            return 0;
        }

        requestCount++;
    } while (!(closeConnection || !serverObject->keepAlive
               || (requestCount >= serverObject->maxKeepalive)));

    return None.inc();
}

void Server_t::readRequest(FRPC::DataBuilder_t &builder) {
    closeConnection = false;
    contentLength = 0;
    headersSent = false;
    head = false;
    queryStorage.clear();
    queryStorage.push_back(std::string());
    queryStorage.back().reserve(BUFFER_SIZE);
    FRPC::HTTPHeader_t httpHead;

    std::string protocol;
    std::string transferMethod;
    std::string contentType;
    std::string uriPath;
    std::auto_ptr<FRPC::UnMarshaller_t> unmarshaller;

    // read header
    try {
        // ln is row number
        // read all lines until first non-empty
        for (;;) {
            // read line from the socket, we check security limits for url
            std::string line(io.readLine(true));

            if (line.empty()) continue;

            // break request line down
            std::vector<std::string> header(io.splitBySpace(line, 3));
            if (header.size() != 3) {
                // invalid request line
                // get rid of old method
                //lastMethod.erase();
                throw FRPC::HTTPError_t
                    (FRPC::HTTP_BAD_REQUEST, "Bad HTTP request: '%s'.",
                     line.substr(0, 30).c_str());
            }

            protocol =  header[2];
            // save request line parts
            if ((protocol != "HTTP/1.1") && (protocol != "HTTP/1.0")){
                throw FRPC::HTTPError_t
                    (FRPC::HTTP_HTTP_VERSION_NOT_SUPPORTED,
                     "Bad HTTP protocol version or type: '%s'.",
                     header[2].c_str());
            }

            uriPath = header[1];

//             if (!path.empty())
//             {
//                if (uriPath != path)
//                {
//                   throw HTTPError_t(HTTP_NOT_FOUND,
//                                   "Uri not found '%s'.",
//                                   header[1].c_str());
//                }
//             }

            transferMethod =  header[0];
            break;
        }

        // read header from the request
        io.readHeader(httpHead);

        if (transferMethod != "POST") {
            if(transferMethod == "HEAD") {
                head = true;
                closeConnection = true;
                return;
            } else {
                throw FRPC::HTTPError_t(FRPC::HTTP_METHOD_NOT_ALLOWED,
                                        "Method Not Allowed");
            }
        }

        // get content type from header
        httpHead.get(FRPC::HTTP_HEADER_CONTENT_TYPE, contentType);

        //create unmarshaller and datasink

        // what is supported by client
        outType = FRPC::Server_t::XML_RPC;
        useChunks = false;

        // what type client send
        std::string accept;
        if (!httpHead.get(FRPC::HTTP_HEADER_ACCEPT, accept)) {

            /* this is default
            if (accept.find("text/xml") != std::string::npos) {
                outType = FRPC::Server_t::XML_RPC;
                useChunks = false;
            }
            */

            if (accept.find("application/x-frpc") != std::string::npos) {
                if (useBinary) {
                    outType = FRPC::Server_t::BINARY_RPC;
                    useChunks = true;
                }
            }
        }

        // what type is request
        if (contentType.find("application/x-frpc") != std::string::npos) {
           unmarshaller = std::auto_ptr<FRPC::UnMarshaller_t>(
                    FRPC::UnMarshaller_t::create(
                        FRPC::UnMarshaller_t::BINARY_RPC,
                        builder));

        } else if (contentType.find("text/xml") != std::string::npos) {
           unmarshaller = std::auto_ptr<FRPC::UnMarshaller_t>(
                    FRPC::UnMarshaller_t::create(
                        FRPC::UnMarshaller_t::XML_RPC,
                        builder));

        } else {
            throw FRPC::StreamError_t("Unknown ContentType");
        }

        FRPC::DataSink_t data(*unmarshaller,
                              FRPC::UnMarshaller_t::TYPE_METHOD_CALL);

        // read body of request
        io.readContent(httpHead, data, true);

        unmarshaller->finish();
        protocolVersion = unmarshaller->getProtocolVersion();
        std::string connection;
        httpHead.get("Connection", connection);
        std::transform(connection.begin(), connection.end(),
                       connection.begin(), std::ptr_fun<int, int>(toupper));
        closeConnection = false;

        if (protocol == "HTTP/1.1") {
            closeConnection = (connection == "CLOSE");
        } else {
            closeConnection = (connection != "KEEP-ALIVE");
            useChunks = false;
        }
    } catch (const FRPC::Error_t &) {
        throw;
    }
}

void Server_t::flush() {
    if (!useChunks) {
        sendResponse();
    } else {
        sendResponse();
        io.sendData("0\r\n\r\n", 5);
    }
}

void Server_t::write(const char* data, unsigned int size) {
    contentLength += size;

    if (size > BUFFER_SIZE - queryStorage.back().size()) {
        if(useChunks)  {
            sendResponse();
            queryStorage.back().append(data, size);
        } else  {
            if (size > BUFFER_SIZE) {
                queryStorage.push_back(std::string(data, size));
            } else {
                queryStorage.back().append(data, size);
            }
        }
    } else {
        queryStorage.back().append(data, size);
    }
}

void Server_t::sendResponse() {
    if (!headersSent) {
        FRPC::HTTPHeader_t header;
        std::ostringstream os;

        //create header
        os << "HTTP/1.1" << ' ' << "200" << ' ' << "OK" << "\r\n";

        //os << "Host: " << url.host << ':' << url.port << "\r\n";
        switch (outType) {
        case FRPC::Server_t::XML_RPC:
            os << FRPC::HTTP_HEADER_CONTENT_TYPE << ": "
               << "text/xml"<< "\r\n";
            break;

        case FRPC::Server_t::BINARY_RPC:
            os << FRPC::HTTP_HEADER_CONTENT_TYPE << ": "
               << "application/x-frpc"<< "\r\n";
            break;

        default:
            throw FRPC::StreamError_t("Unknown protocol");
            break;
        }

        os << FRPC::HTTP_HEADER_ACCEPT << ": text/xml";
        if (useBinary)
            os << ", application/x-frpc";
        os << "\r\n";

        //append connection header
        os << FRPC::HTTP_HEADER_CONNECTION
           << (serverObject->keepAlive ? ": keep-alive" : ": close")
           << "\r\n";

        // write content-length or content-transfer-encoding when we can send
        // content

        if (!useChunks) {
            os << FRPC::HTTP_HEADER_CONTENT_LENGTH << ": " << contentLength
            << "\r\n";
        } else {
            os << FRPC::HTTP_HEADER_TRANSFER_ENCODING << ": chunked\r\n";
        }

        os << "Server:" << " Fast-RPC  Server Linux\r\n";

        // terminate header
        // append separator
        os << "\r\n";

        // send header
        io.sendData(os.str());

        headersSent = true;

        if (head) return;
    }

    if (useChunks) {
        // write chunk size
        std::ostringstream os;
        os << std::hex << queryStorage.back().size() << "\r\n";
        io.sendData(os.str());
        // write chunk
        io.sendData(queryStorage.back().data(),queryStorage.back().size());
        // write chunk terminator
        io.sendData("\r\n", 2);
        queryStorage.back().erase();
    } else {
        while (queryStorage.size() != 1) {
            io.sendData(queryStorage.back().data(),queryStorage.back().size());
            queryStorage.pop_back();
        }

        io.sendData(queryStorage.back().data(),queryStorage.back().size());
        queryStorage.back().erase();
    }
}

void Server_t::sendHttpError(const FRPC::HTTPError_t &httpError) {
    std::ostringstream os;
    //create header
    os << "HTTP/1.1" << ' ' << httpError.errorNum() << ' '
          << httpError.message() << "\r\n";
    os  << FRPC::HTTP_HEADER_ACCEPT << ": "
           << "text/xml, application/x-frpc"<< "\r\n";
    os << "Server:" << " Fast-RPC  Server Linux\r\n";

    // terminate header
    // append separator
    os << "\r\n";
    // send header
    io.sendData(os.str());
}


/************************************************************************/
/****                         MethodRegistry                         ****/
/************************************************************************/

/*
 * free resources associated with a boolean object
 */
void MethodRegistryObject_dealloc(MethodRegistryObject *self) {
    Py_XDECREF(self->preProcess);
    Py_XDECREF(self->postProcess);
    Py_XDECREF(self->preRead);
    delete self->lookup;
    Py_XDECREF(self->defaultMethod);
    Py_XDECREF(self->defaultListMethods);
    Py_XDECREF(self->defaultMethodHelp);
    Py_XDECREF(self->defaultMethodSignature);

    self->ob_type->tp_free(asObject(self));
}

namespace {
    int callableOrNone(PyObject *&c, const char *name) {
        if (!c) {
            c = Py_None;
        } else {
            if (!PyCallable_Check(c)) {
                PyErr_Format(PyExc_TypeError, "'%s' must be callable.", name);
                return -1;
            }
        }
        Py_INCREF(c);
        return 0;
    }

    void registerInternalMethod(MethodRegistryObject *self,
                                const std::string &name,
                                const std::string &signature,
                                const std::string &help)
    {
        Method_t m(name, asObject(self), signature, help);
        self->lookup->insert(MethodLookup_t::value_type(m.name, m));
    }
}

PyObject* MethodRegistryObject_new(PyTypeObject *type, PyObject *args,
                                   PyObject *kwds)
{
    // allocate memory
    assert(type && type->tp_alloc);
    MethodRegistryObject *self =
        reinterpret_cast<MethodRegistryObject*>(type->tp_alloc(type, 0));
    if (!self) return 0;

    // fill defaults (what if __init__ doesn't get called
    self->preProcess = 0;
    self->postProcess = 0;
    self->preRead = 0;
    self->defaultMethod = 0;
    self->defaultListMethods = 0;
    self->defaultMethodHelp = 0;
    self->defaultMethodSignature = 0;

    self->introspectionEnabled = true;

    static const char *kwlist[] = {"preProcess", "postProcess","preRead",
                                   "introspectionEnabled", 0};

    // parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, kwds,
                                     "|OOOi:fastrpc.MethodRegistry",
                                     (char **)kwlist,
                                     &self->preProcess, &self->postProcess,
                                     &self->preRead,
                                     &self->introspectionEnabled))
        return 0;

    if (callableOrNone(self->preProcess, "preProcess")) return 0;

    if (callableOrNone(self->postProcess, "postProcess")) {
        Py_XDECREF(self->preProcess);
        return 0;
    }
    if (callableOrNone(self->preRead, "preRead")) {
        Py_XDECREF(self->preProcess);
        Py_XDECREF(self->postProcess);
        return 0;
    }

    // initialize method lookup
    self->lookup = new MethodLookup_t();

    try {
        registerInternalMethod
            (self, "system.listMethods", "A:",
             ("Return an array of all available RPC methods on this "
              "server."));
        registerInternalMethod
            (self, "system.methodSignature", "s:s",
             ("Given the name of a method, return an array of "
              "legal signatures. Each signature is an array of "
              "strings. The first item of each signature is the "
              "return type, and any others items are parameter types."));
        registerInternalMethod
            (self, "system.methodHelp", "s:s",
             ("Given the name of a method, return a help string."));
        registerInternalMethod
            (self, "system.multicall", "A:A",
             ("Process an array of calls, and return an array of "
              "results. Calls should be structs of the form "
              "{'methodName': string, 'params': array}. Each result "
              "will either be a single-item array containg the "
              "result value, or a struct of the form {'faultCode': int, "
              "'faultString': string}. This is useful when you need "
              "to make lots of small calls without lots of round trips."));
    } catch (const PyError_t&) {
        return 0;
    }

    // OK
    return asObject(self);
}

DECL_METHOD_KWD(MethodRegistryObject, register) {
    // keyword list
    static const char *kwlist[] = { "name", "method", "signature", "doc",
                                    "context", 0 };

    char *name;
    PyObject *callback;
    PyObject *signature = emptyTuple;
    PyObject *help = emptyString;
    PyObject *context = 0;

    // parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "sO|OSO", (char **)kwlist,
                                     &name, &callback, &signature,
                                     &help, &context))
        return 0;

    // callback must me callable
    if (!PyCallable_Check(callback)) {
        return PyErr_Format(PyExc_TypeError,
                            "Callback object of method <%s> is of type <%s> "
                            "and is not callable.", name,
                            callback->ob_type->tp_name);
    }

    try {
        // add or replace method
        Method_t m(name, callback, signature, help, context);
        std::pair<MethodLookup_t::iterator, bool>
            i(self->lookup->insert(MethodLookup_t::value_type(name, m)));

        if (!i.second) i.first->second = m;
    } catch (const PyError_t&) {
        return 0;
    }

    // OK
    return None.inc();
}

DECL_METHOD_KWD(MethodRegistryObject, registerDefault) {
    struct Callback_t {
        char *name;
        PyObject **destination;
        PyObject *callback;
    };

    Callback_t callbacks[] = {
        { (char *)"default method callback",  &self->defaultMethod},
        { (char *)"default method list callback", &self->defaultListMethods },
        { (char *)"default method help callback", &self->defaultMethodHelp},
        { (char *)"default method signature callback",
                                                &self->defaultMethodSignature },
        {}
    };

    static const char *kwlist[] = {"method", "listMethods", "methodHelp",
                                   "methodSignature", 0};

    // parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OOO", (char **)kwlist,
                                     &callbacks[0].callback,
                                     &callbacks[1].callback,
                                     &callbacks[2].callback,
                                     &callbacks[3].callback))
        return 0;

    // callbacks must be callable
    for (Callback_t *callback = callbacks; callback->name; ++callback) {
        if (callback->callback
            && !PyCallable_Check(callback->callback)) {
            return PyErr_Format(PyExc_TypeError,
                                "Callback object of %s method is of "
                                "type <%s> and is not callable.",
                                callback->name,
                                callback->callback->ob_type->tp_name);
        }
    }

    // assign new values
    for (Callback_t *callback = callbacks; callback->name; ++callback) {
        // reasign methods
        if (callback->callback) {
            Py_XDECREF(*callback->destination);
            *callback->destination = callback->callback;
            Py_INCREF(*callback->destination);
        }
    }

    // OK
    return None.inc();
}


DECL_METHOD(MethodRegistryObject, registerHead) {
    PyObject *callback;

    // parse arguments
    if (!PyArg_ParseTuple(args, "O",  &callback))
        return 0;

    // callback must me callable
    if (!PyCallable_Check(callback)) {
        return PyErr_Format(PyExc_TypeError,
                            "Callback object of HTTP HEAD method is of "
                            "type <%s> and is not callable.",
                            callback->ob_type->tp_name);
    }

    // reasign head method
    Py_XDECREF(self->headMethod);
    self->headMethod = callback;
    Py_INCREF(self->headMethod);

    // OK
    return None.inc();
}

static DECL_METHOD(MethodRegistryObject, dispatch) {
    char *name;
    PyObject *clientIP;
    PyObject *params;

    // parse arguments
    if (!PyArg_ParseTuple(args, "sO!O:fastrpc.MethodRegistry.dispatch",
                          &name, &PyTuple_Type, &clientIP, &params))
        return 0;

    // process the call
    return dispatchCall(self, name, clientIP, params);
}

PyObject* dispatchCall(MethodRegistryObject *self, const char *name,
                       PyObject *clientIP, PyObject *params)
{
    // call exception
    PyObjectWrapper_t type;
    PyObjectWrapper_t value;
    PyObjectWrapper_t traceback;

    // method
    const Method_t *method = 0;

    // call result
    PyObjectWrapper_t result;

    FRPC::MethodRegistry_t::TimeDiff_t timeD;

    // call preprocessor
    if (self->preProcess != Py_None) {
        if (!PyObject_CallFunction(self->preProcess, (char *)"(sOO)",
                                   name, clientIP, params)) {
            // exception thrown => fetch exception
            fetchException(type, value, traceback);
            result = makeFault("Unhandled exception in postprocessor ",
                               type, value, traceback);
        }
    }

    if (!result) {
        // find method
        MethodLookup_t::const_iterator flookup = self->lookup->find(name);
        if (flookup != self->lookup->end()) {
            method = &flookup->second;
        } else if (!self->defaultMethod) {
            result = makeFault
                (FRPC::MethodRegistry_t::FRPC_NO_SUCH_METHOD_ERROR,
                 "Method '%s' not found.", name);
        }
    }

    if (!result) {
        // something to call :) call method or default method
        result = (method
                  ? (*method)(clientIP, params)
                  : PyObject_CallFunction(self->defaultMethod, (char *)"(sO)",
                                          name, params));

        if (!result) {
            // exception thrown => fetch exception
            fetchException(type, value, traceback);
        }
        // make something with the result
    }

    // call postprocessor
    if (self->postProcess != Py_None) {
        // build call parameters to postprocessor
        PyObjectWrapper_t fault;
        if (result) {
            // if result is instance of Fault => use result as fault
            if (PyObject_IsInstance(result, Fault))
                fault = result;
        } else {
            // if exception is Fault => use exception as fault
            if (PyErr_GivenExceptionMatches(type, Fault))
                fault = value;
        }


        FRPC::MethodRegistry_t::TimeDiff_t diff(timeD.diff());

        PyObjectWrapper_t postArgs
            (Py_BuildValue("(sOOd)", name, clientIP, params,
                           (diff.usecond / 1000000.0 + diff.second)));

        PyObjectWrapper_t postKwds
            (Py_BuildValue
             ("{s:O,s:O,s:O}",
              "result", *(fault || !result ? None : result),
              "fault", *(fault ? fault : None),
              "exc", *(fault || result ? None
                       : PyObjectWrapper_t
                       (Py_BuildValue("(OOO)", *type, *value, *traceback)))));

        PyObjectWrapper_t processedResult
             (PyObject_Call(self->postProcess, postArgs, postKwds));
        if (!processedResult) {
            // make fault from unhandled exception
            return makeFault("Unhandled exception in postprocessor ");
        }

        // return result of postprocessor if non-none
        if (processedResult != Py_None)
            return processedResult.inc();
    } else {
        // no postprocessor

        // if exception is Fault => return fault value
        if (PyErr_GivenExceptionMatches(type, Fault))
            return value.inc();
    }

    if (!result) {
        // make fault from unhandled exception
        result = makeFault("Unhandled exception ", type, value, traceback);
    }

    // OK
    return result.inc();
}

DECL_METHOD(MethodRegistryObject, system_listMethods) {
    // parse args (must be empty)
    PyObject *context;
    PyObject *params;
    if (!PyArg_ParseTuple(args, "OO", &context, &PyTuple_Type, &params))
        return 0;

    if (!self->introspectionEnabled)
        return makeFault
            (FRPC::MethodRegistry_t::FRPC_INTROSPECTION_DISABLED_ERROR,
             "Introspection API disabled for security reasons.");

    PyObjectWrapper_t result;
    int defaultSize = 0;

    if (self->defaultListMethods) {
        // we have some default method lister
        result = (PyObject_CallFunction(self->defaultListMethods,
                                       (char *)"()"));
        if (!result) return 0;
        defaultSize = PyList_Size(result);
        if (defaultSize == -1) return 0;
    }

    // create result tuple (or use defaultList)
    if (!result) {
        result = PyList_New(0);
        if (!result) return 0;
    }

    // fill this tuple with method names
    int i = 0;
    for (MethodLookup_t::const_iterator ilookup = self->lookup->begin();
         ilookup != self->lookup->end(); ++ilookup, ++i) {
        // create python method name
        PyObject *name = PyString_FromStringAndSize(ilookup->first.data(),
                                                    ilookup->first.length());
        if (!name) return 0;
        if (PyList_Append(result, name)) {
            Py_DECREF(name);
            return 0;
        }
    }

    // sort if some default method helps available
    if (defaultSize && (PyList_Sort(result) == -1))
        return 0;

    // OK
    return result.inc();
}

DECL_METHOD(MethodRegistryObject, system_methodSignature) {
    // parse args (must be empty)
    PyObject *context;
    PyObject *params;
    if (!PyArg_ParseTuple(args, "OO!", &context, &PyTuple_Type, &params))
        return 0;

    // parse args (must have only one string)
    const char *name;
    if (!PyArg_ParseTuple(params, "s", &name)) {
        PyErr_Clear();
        return makeFault(FRPC::MethodRegistry_t::FRPC_TYPE_ERROR,
                         "Method expects only one string.");
    }

    if (!self->introspectionEnabled)
        return makeFault
            (FRPC::MethodRegistry_t::FRPC_INTROSPECTION_DISABLED_ERROR,
             "Introspection API disabled for security reasons.");

    // find method's signature
    MethodLookup_t::const_iterator flookup = self->lookup->find(name);
    if (flookup == self->lookup->end()) {
        if (self->defaultMethodSignature) {
            // method not found but default fallback found
            PyObjectWrapper_t signature
                (PyObject_CallFunction(self->defaultMethodSignature,
                                       (char *)"(s)", name));
            // check for error or non-string signature
            if (!signature || !PyString_Check(signature))
                return signature.inc();

            // string => convert to array of arrays
            return stringToSignature(signature);
        }
        return makeFault(FRPC::MethodRegistry_t::FRPC_INDEX_ERROR,
                         "Method '%s' not found.", name);
    }

    // OK
    return flookup->second.signature.inc();
}

DECL_METHOD(MethodRegistryObject, system_methodHelp) {
    // parse args (must be empty)
    PyObject *context;
    PyObject *params;
    if (!PyArg_ParseTuple(args, "OO!", &context, &PyTuple_Type, &params))
        return 0;

    // parse args (must have only one string)
    const char *name;
    if (!PyArg_ParseTuple(params, "s", &name)) {
        PyErr_Clear();
        return makeFault(FRPC::MethodRegistry_t::FRPC_TYPE_ERROR,
                         "Method expects only one string.");
    }

    if (!self->introspectionEnabled)
        return makeFault
            (FRPC::MethodRegistry_t::FRPC_INTROSPECTION_DISABLED_ERROR,
             "Introspection API disabled for security reasons.");

    // find method's signature
    MethodLookup_t::const_iterator flookup = self->lookup->find(name);
    if (flookup == self->lookup->end()) {
        if (self->defaultMethodHelp) {
            // method not found but default fallback found
            return PyObject_CallFunction(self->defaultMethodHelp,
                                         (char *)"(s)", name);
        }
        return makeFault(FRPC::MethodRegistry_t::FRPC_INDEX_ERROR,
                         "Method '%s' not found.", name);
    }

    // OK
    return flookup->second.help.inc();
}

DECL_METHOD(MethodRegistryObject, system_multicall) {
    // parse args (runtime data contain clientIP)
    PyObject *clientIP;
    PyObject *params;
    if (!PyArg_ParseTuple(args, "OO!", &clientIP, &PyTuple_Type, &params))
        return 0;

    // parse args
    PyObject *methodCalls;
    if (!PyArg_ParseTuple(params, "O!:system.multicall",
                          &PyList_Type, &methodCalls))
        return 0;

    int size = PyList_Size(methodCalls);
    if (size < 0) return 0;

    PyObjectWrapper_t result = PyList_New(size);
    if (!result) return 0;

    for (int i = 0; i < size; ++i) {
        // get i-th element from the method call specification
        PyObject *methodCall = PyList_GetItem(methodCalls, i);
        if (!methodCall) {
            if (PyList_SetItem
                (result, i,
                 makeFaultStruct(FRPC::MethodRegistry_t::FRPC_TYPE_ERROR,
                                 "Cannot get method call.")))
                return 0;
            continue;
        }

        if (!PyDict_Check(methodCall)) {
            PyList_SetItem
                (result, i,
                 makeFaultStruct(FRPC::MethodRegistry_t::FRPC_TYPE_ERROR,
                                 "Expected structure with method call."));
            continue;
        }

        if (!PyDict_Check(methodCall)) {
            PyList_SetItem
                (result, i,
                 makeFaultStruct(FRPC::MethodRegistry_t::FRPC_TYPE_ERROR,
                                 "Method call should be structure."));
            continue;
        }

        PyObject *params = PyDict_GetItemString(methodCall, "params");
        if (!params) {
            if (PyList_SetItem
                (result, i,
                 makeFaultStruct(FRPC::MethodRegistry_t::FRPC_TYPE_ERROR,
                                 "Cannot get params.")))
                return 0;
            continue;
        }

        PyObjectWrapper_t tmp;
        if (PyList_Check(params)) {
            if (!(tmp = params = PyList_AsTuple(params)))
                return 0;
        } else if (!PyTuple_Check(params)) {
            if (PyList_SetItem
                (result, i,
                 makeFaultStruct(FRPC::MethodRegistry_t::FRPC_TYPE_ERROR,
                                 "Params must be an array.")))
                return 0;
            continue;
        }

        PyObject *methodName = PyDict_GetItemString(methodCall, "methodName");
        if (!methodName) {
            if (PyList_SetItem
                (result, i,
                 makeFaultStruct(FRPC::MethodRegistry_t::FRPC_TYPE_ERROR,
                                 "Cannot get methodName.")))
                return 0;
            continue;
        }

        char *name = PyString_AsString(methodName);
        if (!name) {
            PyObjectWrapper_t type;
            PyObjectWrapper_t value;
            PyObjectWrapper_t traceback;
            fetchException(type, value, traceback);

            PyObjectWrapper_t faultStruct
                (makeFaultStruct("Cannot convert methodName to string.",
                                 type, value, traceback));

            // result is fault struct {code, message}
            if (PyList_SetItem(result, i, faultStruct))
                return 0;

            faultStruct.inc();
            continue;
        }

        PyObjectWrapper_t callResult
            (dispatchCall(self, name, clientIP, params));

        if (PyObject_IsInstance(callResult, Fault)) {
            callResult = faultToStruct(callResult);
            if (!callResult) return 0;
        }

        if (PyList_SetItem(result, i, callResult))
            return 0;
        callResult.inc();
    }

    // OK
    return result.inc();
}

#define OFF(x) offsetof(ServerObject, x)

static PyMemberDef ServerObject_members[] = {
    {(char *)"registry",             T_OBJECT,    OFF(registry),       RO},
    {(char *)"readTimeout",          T_INT,       OFF(readTimeout),    RO},
    {(char *)"writeTimeout",         T_INT,       OFF(writeTimeout),   RO},
    {(char *)"keepAlive",            T_INT,       OFF(keepAlive),      RO},
    {(char *)"maxKeepalive",         T_INT,       OFF(maxKeepalive),   RO},
    {(char *)"useBinary",            T_INT,       OFF(useBinary),      RO},

    {0}  // Sentinel
};

#undef OFF

static PyMethodDef ServerObject_methods[] = {
    {
        "serve",
        reinterpret_cast<PyCFunction>(ServerObject_serve),
        METH_VARARGS,
        "Handle connection."
    },

    { 0, 0, 0, 0 } // sentinel
};

static PyTypeObject ServerObject_Type = {
    PyObject_HEAD_INIT(&PyType_Type)
    0,                                      /*ob_size*/
    "Server",                               /*tp_name*/
    sizeof (ServerObject),                  /*tp_basicsize*/
    0,                                      /*tp_itemsize*/
    /* methods */
    (destructor) ServerObject_dealloc,      /*tp_dealloc*/
    0,                                      /*tp_print*/
    0,                                      /*tp_getattr*/
    0,                                      /*tp_setattr*/
    0,                                      /*tp_compare*/
    0,                                      /*tp_repr*/
    0,                                      /*tp_as_number*/
    0,                                      /*tp_as_sequence*/
    0,                                      /*tp_as_mapping*/
    0,                                      /*tp_hash*/
    0,                                      /* tp_call */
    0,                                      /* tp_str */
    PyObject_GenericGetAttr,                /* tp_getattro */
    PyObject_GenericSetAttr,                /* tp_setattro */
    0,                                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,                     /* tp_flags */
    0,                                      /* tp_doc */
    0,                                      /* tp_traverse */
    0,                                      /* tp_clear */
    0,                                      /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    0,                                      /* tp_iter */
    0,                                      /* tp_iternext */
    ServerObject_methods,                   /* tp_methods */
    ServerObject_members,                   /* tp_members */
    0,                                      /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,                                      /* tp_descr_get */
    0,                                      /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    0,                                      /* tp_init */
    PyType_GenericAlloc,                    /* tp_alloc */
    ServerObject_new,                       /* tp_new */
    _PyObject_Del                           /* tp_free */
};

/** free resources associated with a server object
 */
void ServerObject_dealloc(ServerObject *self) {
    Py_XDECREF(self->registry);
    delete self->server;

    self->ob_type->tp_free(asObject(self));
}

PyObject* ServerObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    // allocate memory
    assert(type && type->tp_alloc);
    ServerObject *self =
        reinterpret_cast<ServerObject*>(type->tp_alloc(type, 0));
    if (!self) return 0;

    PyObjectWrapper_t selfHolder(asObject(self));

    static const char *kwlist[] = { "readTimeout", "writeTimeout", "keepAlive",
                                    "maxKeepalive", "introspectionEnabled",
                                    "callbacks", "stringMode", "useBinary", 0 };

    // set defaults
    self->readTimeout = 10000;
    self->writeTimeout = 1000;
    self->keepAlive = false;
    self->maxKeepalive = 0;
    self->stringMode = FRPC::Python::STRING_MODE_MIXED;
    self->useBinary = true;

    PyObject *callbacks = 0;
    PyObject *introspectionEnabled = 0;
    PyObject *useBinary = 0;

    char *stringMode = 0;

    // parse arguments
    if (!PyArg_ParseTupleAndKeywords(args, kwds,
                                     "|iiiiOOsO::fastrpc.Server",
                                     (char **)kwlist,
                                     &self->readTimeout, &self->writeTimeout,
                                     &self->keepAlive, &self->maxKeepalive,
                                     &introspectionEnabled, &callbacks,
                                     &stringMode, &useBinary))
        return 0;

    if (useBinary)
        self->useBinary = PyObject_IsTrue(useBinary);

    if ((self->stringMode = FRPC::Python::parseStringMode(stringMode))
        == FRPC::Python::STRING_MODE_INVALID)
        return 0;

    PyObjectWrapper_t registryKwds(PyDict_New());
    if (!registryKwds) return 0;

    // copy callbacks to keyword arguments
    if (callbacks) {
        if (PyObject *preProcess
            = PyDict_GetItemString(callbacks, "preProcess"))
            PyDict_SetItemString(registryKwds, "preProcess", preProcess);

        if (PyObject *postProcess
            = PyDict_GetItemString(callbacks, "postProcess"))
            PyDict_SetItemString(registryKwds, "postProcess", postProcess);

        if (PyObject *preRead
            = PyDict_GetItemString(callbacks, "preRead"))
            PyDict_SetItemString(registryKwds, "preRead", preRead);
    }

    // create registry
    if (introspectionEnabled)
        PyDict_SetItemString(registryKwds, "introspectionEnabled",
                             introspectionEnabled);

    if (!(self->registry = newObject<MethodRegistryObject>
          (MethodRegistryObject_Type, emptyTuple, registryKwds)))
        return 0;

    // create the server
    self->server = new Server_t(self);

    // OK
    return selfHolder.inc();
}

static DECL_METHOD(ServerObject, serve) {
    // parse arguments
    PyObject *file;
    PyObject *clientIP = Py_None;
    if (!PyArg_ParseTuple(args, "O|O!:fastrpc.Server.serve",
                          &file, &PyTuple_Type, &clientIP))
        return 0;

    // convert file to filedescriptor
    int fd = PyObject_AsFileDescriptor(file);
    if (fd == -1)
        return 0;

    // handle the connection
    return self->server->serve(fd, clientIP);
}

namespace {
    Method_t::Method_t(const std::string &name, PyObject *callback,
                       PyObject *signature, PyObject *help, PyObject *context)
        : name(name), callback(callback, true), signature(signature, true),
          help(help, true), context(context, true)
    {
        if (PyString_Check(signature)) {
            char *sig;
            Py_ssize_t sigSize;
            if (PyString_AsStringAndSize(signature, &sig, &sigSize))
                throw PyError_t();
            if (!(this->signature =
                  stringToSignature(std::string(sig, sigSize))))
                throw PyError_t();
        }
    }

    Method_t::Method_t(const std::string &name, PyObject *callback,
                       const std::string &signature, const std::string &help,
                       PyObject *context)
        : name(name), callback(callback, true), signature(), help(),
          context(context, true)
    {
        if (!(this->help = PyString_FromStringAndSize
              (help.data(), help.size())))
            throw PyError_t();

        if (!(this->signature = stringToSignature(signature)))
            throw PyError_t();
    }

    PyObject* Method_t::operator()(PyObject *runtime, PyObject *args) const {
        if (PyObject_TypeCheck(callback.object, &MethodRegistryObject_Type)) {
            // callback is just method registry => call method

            // prepare for call
            std::string method("__" + name);
            char *c_method = const_cast<char*>(method.c_str());
            return PyObject_CallMethod
                (callback.object, c_method, (char *)"(OO)", runtime, args);
        } else {
            // callback is realy something callback

            // use context or unwind call
            if (!context) return PyObject_CallObject(callback.object, args);

            return PyObject_CallFunctionObjArgs
                (callback.object, context.object, args, 0);
        }
    }
}

namespace FRPC { namespace Python {

    int initServer(PyObject *fastrpc_module) {
        // initialize empty helper structures
        if (!(emptyTuple = PyTuple_New(0))) return -1;
        if (!(emptyDict = PyDict_New())) return -1;
        if (!(emptyString = PyString_FromString(""))) return -1;

        Py_INCREF(&ServerObject_Type);
        if (PyModule_AddObject
            (fastrpc_module, "Server",
             asObject(&ServerObject_Type)))
            return -1;

        Py_INCREF(&MethodRegistryObject_Type);
        if (PyModule_AddObject
            (fastrpc_module, "MethodRegistry",
             asObject(&MethodRegistryObject_Type)))
            return -1;

        // create Fault format string
        if (!(Fault_message_format = PyString_FromString("%s%s: %s")))
            return -1;

        // OK
        return 0;
    }

} } // namespace FRPC::Python

