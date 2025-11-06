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
 * FILE          $Id: frpcfault.h,v 1.4 2007-05-18 15:29:45 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCFAULT_H
#define FRPCFRPCFAULT_H

#include <frpcplatform.h>

#include <string>
#include <stdio.h>
#include <stdarg.h>

namespace FRPC {

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Fault_t : public std::exception {
public:
    Fault_t(int errNum, const std::string &errMsg)
        : errNum(errNum),
          errMsg(errMsg)
    {}

    ~Fault_t() throw();

    int errorNum() {
        return errNum;
    }

    /// Creates a fault with specified formatted string.
    static Fault_t format(int errNum, const char *format, ...) __attribute__((format(printf, 2, 3)));

    const std::string& message() {
        return errMsg;
    }
    int errorNum() const {
        return errNum;
    }

    const std::string& message() const {
        return errMsg;
    }

    virtual const char * what () const throw () {
        return  errMsg.c_str();
    }

private:
    Fault_t();
    int errNum;
    std::string errMsg;


};

}

#endif
