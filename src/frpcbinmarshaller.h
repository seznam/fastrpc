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
 * FILE          $Id: frpcbinmarshaller.h,v 1.9 2010-04-21 08:48:03 edois Exp $
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
#include <frpcinternals.h>
#include <frpc.h>
#include <frpcwriter.h>
#include <frpcint.h>
#include <frpcnull.h>
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

    ~BinMarshaller_t() override;

    void packArray(unsigned int numOfItems) override;
    void packBinary(const char* value, unsigned int size) override;
    void packBool(bool value) override;
    void packDateTime(short year, char month, char day, char hour,
                              char minute, char sec, char weekDay,
                              time_t unixTime, int timeZone) override;
    void packDouble(double value) override;
    void packFault(int errNumber, const char* errMsg,
                           unsigned int size) override;
    void packInt(Int_t::value_type value) override;
    void packMethodCall(const char* methodName, unsigned int size) override;
    void packString(const char* value, unsigned int size) override;
    void packStruct(unsigned int numOfMembers) override;
    void packStructMember(const char* memberName, unsigned int size) override;
    void packMethodResponse() override;
    void flush() override;

    void packNull();

    void packBinaryRef(BinaryRefFeeder_t feeder);

private:

    BinMarshaller_t();

    void packMagic();

    Writer_t  &writer;
    ProtocolVersion_t protocolVersion;
};

} // namespace FRPC

#endif
