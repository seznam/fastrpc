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
 * FILE          $Id: frpcbinmarshaller.h,v 1.5 2007-05-18 15:29:45 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCBINMARSHALLER_H
#define FRPCFRPCBINMARSHALLER_H

#include <frpcmarshaller.h>
#include <vector>
#include <frpcinternals.h>
#include <frpc.h>
#include <frpcwriter.h>
#include <frpcint.h>
#include <frpcstreamerror.h>

namespace FRPC {




/**
@brief Binary Marshaller (FastRPC)
@author Miroslav Talasek
*/
class BinMarshaller_t : public Marshaller_t {
public:
    BinMarshaller_t(Writer_t &writer,
                    const ProtocolVersion_t &protocolVersion);

    virtual ~BinMarshaller_t();

    virtual void packArray(unsigned int numOfItems);
    virtual void packBinary(const char* value, unsigned int size);
    virtual void packBool(bool value);
    virtual void packDateTime(short year, char month, char day, char hour,
                              char minute, char sec,
                              char weekDay, time_t unixTime, char timeZone);
    virtual void packDouble(double value);
    virtual void packFault(int errNumber, const char* errMsg, unsigned int size);
    virtual void packInt(Int_t::value_type value);
    virtual void packMethodCall(const char* methodName, unsigned int size);
    virtual void packString(const char* value, unsigned int size);
    virtual void packStruct(unsigned int numOfMembers);
    virtual void packStructMember(const char* memberName, unsigned int size);
    virtual void packMethodResponse();
    virtual void flush();

private:

    BinMarshaller_t();

    inline unsigned int getNumberSize(Int_t::value_type number) {

        if (protocolVersion.versionMajor < 2) {

            // + 1 => old marking
            if (!(number & INT8_MASK))
                return CHAR8 + 1;

            if (!(number & INT16_MASK))
                return SHORT16 + 1;

            if (!(number & INT24_MASK))
                return LONG24 + 1;

            if (!(number & INT32_MASK))
                return LONG32 + 1;

            throw StreamError_t("Number is too big for protocol version 1.0");
        } else {
            if (!(number & INT8_MASK))
                return CHAR8;

            if (!(number & INT16_MASK))
                return SHORT16;

            if (!(number & INT24_MASK))
                return LONG24;

            if (!(number & INT32_MASK))
                return LONG32;

            if (!(number & INT40_MASK))
                return LONG40;

            if (!(number & INT48_MASK))
                return LONG48;

            if (!(number & INT56_MASK))
                return LONG56;

            return LONG64;

        }
    }

    void packMagic();

    //vector<TypeStorage_t> vectEntity; //not used
    Writer_t  &writer;
    ProtocolVersion_t protocolVersion;
};

}

#endif
