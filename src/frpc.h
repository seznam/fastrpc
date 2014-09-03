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
 * FILE          $Id: frpc.h,v 1.12 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */



#ifndef FRPCFRPC_H
#define FRPCFRPC_H

#include <frpcplatform.h>

#include <frpcvalue.h>
#include <frpcarray.h>

#include <frpcbinary.h>
#include <frpcbool.h>
#include <frpcdatetime.h>
#include <frpcdouble.h>
#include <frpcint.h>
#include <frpcerror.h>
#include <frpcpool.h>
#include <frpcstring.h>
#include <frpcstruct.h>
#include <frpcnull.h>

#include <frpcerror.h>
#include <frpctypeerror.h>
#include <frpcindexerror.h>
#include <frpcstreamerror.h>
#include <frpcversion.h>

#include <string>
#include <bitset>
#include <set>


namespace FRPC {

void parseISODateTime(const char *data, long len, short &year, char &month,
                      char &day, char &hour,
                      char &minute, char &sec, int &timeZone);

std::string getISODateTime(short year, char month,
                           char day, char hour,
                           char minute, char sec, int timeZone);

/**
 * @short Dump FastRPC tree to string.
 * @param value FastRPC value.
 * @param outstr dump storage string.
 * @param level dump only to this level.
 * @param names mask all struct members with this names
 * @param pos mask all array members at these positions in top level array.
 * @return zero
 */
int FRPC_DLLEXPORT dumpFastrpcTree(const Value_t &value, std::string &outstr,
                                   int level, std::set<std::string> names,
                                   std::bitset<sizeof(unsigned long) * 8> pos);

/// old fashion dump without masks
int FRPC_DLLEXPORT dumpFastrpcTree(const Value_t &value, std::string &outstr,
                                   int level);

void printValue(const Value_t &value, long spaces = 0);

struct ProtocolVersion_t{
    ProtocolVersion_t(unsigned char versionMajor, unsigned char versionMinor);

    ProtocolVersion_t();

    //ProtocolVersion_t operator=(const ProtocolVersion_t& other );

    unsigned char versionMajor;
    unsigned char versionMinor;
};

/**
 @brief init fastrpc library
 * call once in the main thread 
 * init xmllib parser
 */
void FRPC_DLLEXPORT initFastrpc();

} // namespace FRPC

#endif
