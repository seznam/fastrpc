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
 * FILE          $Id: frpcstreamerror.h,v 1.3 2007-05-18 15:29:46 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCSTREAMERROR_H
#define FRPCFRPCSTREAMERROR_H

#include <frpcerror.h>

namespace FRPC
{

/**
@brief StreamError_t used in Unmarshall
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT StreamError_t : public Error_t
{
public:
    StreamError_t(const std::string &msg) : Error_t(msg) {}

    static StreamError_t format(const char *fmt, ...) __attribute__((format(printf, 1, 2)));

    ~StreamError_t() throw();
private:
     /**
        @brief Default constructor is disabled
    */
     StreamError_t();
};

};

#endif
