/*
 * FILE          $Id: frpc.h,v 1.4 2006-10-31 11:19:41 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2005
 * All Rights Reserved
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

#include <frpcerror.h>
#include <frpctypeerror.h>
#include <frpcindexerror.h>
#include <frpcstreamerror.h>


#include <string>

namespace FRPC {

void parseISODateTime(const char *data, long len, short &year, char &month,
                      char &day, char &hour,
                      char &minute, char &sec, char &timeZone);

std::string getISODateTime(short year, char month,
                           char day, char hour,
                           char minute, char sec, char timeZone);

int FRPC_DLLEXPORT dumpFastrpcTree(const Value_t &value,
                    std::string &outstr, int level);

void printValue(Value_t &value, long spaces = 0);

} // namespace FRPC

#endif
