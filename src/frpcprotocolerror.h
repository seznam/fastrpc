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
 * FILE          $Id: frpcprotocolerror.h,v 1.4 2007-05-18 15:29:45 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCPROTOCOLERROR_H
#define FRPCFRPCPROTOCOLERROR_H

#include <frpcplatform.h>
#include <frpcerror.h>
#include <string>
#include <stdio.h>
#include <stdarg.h>

namespace FRPC {

/**
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT ProtocolError_t: public Error_t {
public:
    ProtocolError_t(int errNum, const std::string &msg)
        : Error_t(msg),
          errNum(errNum)
    {}

    /**
    @brief Constructs ProtocolError_t from format string and arguments
    @param errNum is an error number
    @param format is const char* format string
    @param ... is other arguments
     */
    static ProtocolError_t format(int errNum,const char *format, ...) __attribute__((format(printf, 2, 3)));

    /**
     * @brief constructor with only errnum
     * used in child clases
     * */
    ProtocolError_t(int errNum):errNum(errNum) {}


    ~ProtocolError_t() throw();


    /**
    * @brief Getting error message
    * @return long is error number
     * */
    int errorNum() {
        return errNum;
    }

    int errorNum()  const {
        return errNum;
    }
protected:
    ProtocolError_t();

    int errNum;


};

};

#endif
