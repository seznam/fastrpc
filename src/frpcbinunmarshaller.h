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
 * FILE          $Id: frpcbinunmarshaller.h,v 1.4 2008-03-14 10:29:14 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *              Jan Klesnil <jan.klesnil@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCBINUNMARSHALLER_H
#define FRPCFRPCBINUNMARSHALLER_H

#include <frpcunmarshaller.h>
#include <frpcdatabuilder.h>
#include <frpc.h>
#include <vector>
#include <string>

namespace FRPC {

class BinUnMarshaller_t : public UnMarshaller_t {
public:
    class Driver_t;
    class FaultBuilder_t;
    friend class Driver_t;
    friend class FaultBuilder_t;

    BinUnMarshaller_t(DataBuilder_t & dataBuilder)
        : dataBuilder(dataBuilder),
          dataWanted(4), // size of magic and version header
          state(0),
          faultState(0)
    {}

    virtual ~BinUnMarshaller_t();

    virtual void unMarshall(const char *data, unsigned int size, char type);
    virtual void finish();
    virtual ProtocolVersion_t getProtocolVersion() {
        return protocolVersion;
    }

    size_t unMarshallKnown(const char *data, unsigned int size, char type);
    size_t recursionLevel() const { return recursionStack.size(); }

    void resetToFaultState();

protected:
    BinUnMarshaller_t();

    struct StackElement_t {
        uint32_t members;
        uint8_t type;
    };

    DataBuilder_t &dataBuilder;
    std::vector<StackElement_t> recursionStack;
    std::string buffer;
    uint64_t dataWanted;
    int64_t errNo;
    uint8_t state;
    uint8_t faultState;
    ProtocolVersion_t protocolVersion;
    uint64_t _reserved1;
    uint64_t _reserved2;
};

}

#endif
