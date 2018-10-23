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
 *
 * DESCRIPTION
 * Python FastRPC support. Python 2/3 compatibility macros.
 *
 */


#ifndef PYTHONCOMPAT_H_
#define PYTHONCOMPAT_H_

#if PY_MAJOR_VERSION >= 3

using PyStrDataType_t = const char *;

// python string to string and size
# define STR_ASSTRANDSIZE(str, data, len) \
    data = PyUnicode_AsUTF8AndSize(str, &len); \
    if (data == NULL)

// Use the PyLong instead of deprecated PyInt
#define PyInt_Check PyLong_Check
#define PyInt_FromLong PyLong_FromLong
#define PyInt_AsLong PyLong_AsLong

#else

using PyStrDataType_t = char *;

// Python string to string and size
# define STR_ASSTRANDSIZE(str, data, len) \
    if (PyString_AsStringAndSize(str, &data, &len) < 0)

#endif



#endif // PYTHONCOMPAT_H_
