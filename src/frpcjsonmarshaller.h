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
 * FILE             $Id: frpcjsonmarshaller.h,v 1.1 2011-01-10 22:27:04 burlog Exp $
 *
 * DESCRIPTION      JSON marshaller
 *
 * PROJECT          FastRPC library.
 *
 * AUTHOR           Michal Bukovsky <michal.bukovsky@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2011
 * All Rights Reserved
 *
 * HISTORY
 *       2011-01-06 (bukovsky)
 *                  First draft.
 */

#ifndef FRPC_FRPCJSONMARSHALLER_H
#define FRPC_FRPCJSONMARSHALLER_H

#include <string>
#include <vector>
#include <utility>

#include <frpc.h>
#include <frpcmarshaller.h>

namespace FRPC {

class Writer_t;
struct ProtocolVersion_t;

/** 
 * @short
 */
class JSONMarshaller_t: public Marshaller_t {
public:
    /** 
     * @short C'tor.
     * @param writer 
     * @param protocolVersion 
     */
    JSONMarshaller_t(Writer_t &writer,
                    const ProtocolVersion_t &protocolVersion);

    virtual void flush();

    virtual void packMethodCall(const char *methodName, unsigned int size);
    virtual void packMethodResponse();
    virtual void packFault(int errNumber, const char *errMsg,
                           unsigned int size);

    virtual void packArray(unsigned int numOfItems);
    virtual void packStruct(unsigned int numOfMembers);
    virtual void packStructMember(const char *memberName, unsigned int size);
    virtual void packBinary(const char *value, unsigned int size);
    virtual void packBool(bool value);
    virtual void packDouble(double value);
    virtual void packInt(Int_t::value_type value);
    virtual void packString(const char *value, unsigned int size);
    virtual void packDateTime(short year, char month, char day, char hour,
                              char min, char sec, char weekDay, time_t unixTime,
                              int tz);

    void packNull();

    void packBinaryRef(BinaryRefFeeder_t feeder);

private:
    enum State_t { ARRAY = ']', STRUCT = '}'};

    Writer_t  &writer;                                  //!< output writer
    std::vector<std::pair<State_t, unsigned int> > ctx; //!< ctx stack
};

} // namespace FRPC

#endif /* FRPC_FRPCJSONMARSHALLER_H */

