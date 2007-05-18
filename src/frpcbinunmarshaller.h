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
 * FILE          $Id: frpcbinunmarshaller.h,v 1.3 2007-05-18 15:29:45 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#ifndef FRPCFRPCBINUNMARSHALLER_H
#define FRPCFRPCBINUNMARSHALLER_H

#include <frpcunmarshaller.h>
#include <frpcdatabuilder.h>
#include <frpcinternals.h>
#include <frpc.h>
#include <vector>
#include <string>

namespace FRPC {

/**
@author Miroslav Talasek
*/
class BinUnMarshaller_t : public UnMarshaller_t {
public:
    BinUnMarshaller_t(DataBuilder_t & dataBuilder)
            :dataBuilder(dataBuilder),internalType(MAGIC),typeEvent(NONE)
            ,mainInternalType(NONE),
            dataWanted(4),readPosition(0) {}

    virtual ~BinUnMarshaller_t();

    virtual void unMarshall(const char *data, unsigned int size, char type);
    virtual void finish();
    virtual ProtocolVersion_t getProtocolVersion() {
        return protocolVersion;
    }

private:
    BinUnMarshaller_t();
    inline int readData(const char *data, unsigned int size) {

        unsigned int readSize = (dataWanted > (size-readPosition))?size
                                - readPosition:dataWanted;

        if (readPosition == size) {
            readPosition = 0;
            return -1;
        }


        mainBuff.append(&data[readPosition], readSize);
        dataWanted -= readSize;
        readPosition += readSize;

        if (dataWanted == 0) {
            return 0;
        } else {
            readPosition = 0;
            return -1;
        }
        //ak velkost poskytnutych dat je == readPosition tak readposition= 0

    }

    /**
        @brief this method check if next item is struct member or value
    */
    inline bool isNextMember() {
        if (entityStorage.size() < 1)
            return false;

        if (entityStorage.back().type != STRUCT)
            return false;

        if (entityStorage.back().member)
            return false;

        return true;
    }
    //decrement member count if any ARRAY or STRUCT exist
    inline void decrementMember() {
        if (entityStorage.size() < 1)
            return ;

        //decrement member count
        entityStorage.back().numOfItems--;
        //if struct want member name
        if (entityStorage.back().type == STRUCT)
            entityStorage.back().member = false;

        if (entityStorage.back().numOfItems != 0 )
            return;
        //call builder to close entity
        switch (entityStorage.back().type) {

        case STRUCT:
            dataBuilder.closeStruct();

            break;

        case ARRAY:
            dataBuilder.closeArray();

            break;

        default:
            break;
        }
        entityStorage.pop_back();
        decrementMember();
    }

    DataBuilder_t &dataBuilder;

    std::vector<TypeStorage_t> entityStorage;
    char internalType;
    char typeEvent;
    char mainInternalType;
    std::string mainBuff;
    //long dataReaded;
    unsigned int dataWanted;
    unsigned int readPosition;
    unsigned int errNo;
    ProtocolVersion_t protocolVersion;
};

};

#endif
