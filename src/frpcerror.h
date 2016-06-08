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
 * FILE          $Id: frpcerror.h,v 1.4 2007-05-18 15:29:45 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCERROR_H
#define FRPCFRPCERROR_H

#include <frpcplatform.h>

#include <string>
#include <stdio.h>
#include <stdio.h>
#include <stdarg.h>

namespace FRPC {

/**
@brief API Error
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Error_t : public std::exception {
public:

    Error_t(const std::string &msg) : msg(msg) {}

    /** Constructs the Error_t object from format string and parameters.
     */
    static Error_t format(const char *format, ...) __attribute__((format(printf, 1, 2)));

    /**
        @brief Getting error message
        @return std::string is error message
    */
    virtual const char * what () const throw() {
        return msg.c_str();
    }

    const std::string message() {
        return msg;
    }

    const std::string message()  const {
        return msg;
    }
    /**
        @brief Default destructor
    */
    ~Error_t() throw();

protected:
    /**
        @brief Default constructor
    */
    Error_t() {}
    std::string msg;
};

}

#endif
