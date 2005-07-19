/*
 * FILE          $Id: frpcbinunmarshaller.h,v 1.1 2005-07-19 13:02:53 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2002
 * All Rights Reserved
 *
 * HISTORY
 *       
 */
#ifndef FRPCFRPCBINUNMARSHALLER_H
#define FRPCFRPCBINUNMARSHALLER_H

#include <frpcunmarshaller.h>
#include <frpcdatabuilder.h>
#include <frpcinternals.h>

#include <vector>
#include <string>

namespace FRPC
{

/**
@author Miroslav Talasek
*/
class BinUnMarshaller_t : public UnMarshaller_t
{
public:
    BinUnMarshaller_t(DataBuilder_t & dataBuilder)
            :dataBuilder(dataBuilder),internalType(MAGIC),typeEvent(NONE)
            ,mainInternalType(NONE),
            dataWanted(4),readPosition(0)
    {}

    virtual ~BinUnMarshaller_t();

    virtual void unMarshall(const char *data, long size, char type);
    virtual void finish();

private:
    BinUnMarshaller_t();
    inline int readData(const char *data, long size)
    {

        long readSize = (dataWanted > (size-readPosition))?size - readPosition:dataWanted;
        
        if(readPosition == size)
        {
            readPosition = 0;
            return -1;
        } 


        mainBuff.append(&data[readPosition], readSize);
        dataWanted -= readSize;
        readPosition += readSize;

        if(dataWanted == 0)
        {
            return 0;
        }
        else
        {
            readPosition = 0;
            return -1;
        }
        //ak velkost poskytnutych dat je == readPosition tak readposition= 0

    }

    /**
        @brief this method check if next item is struct member or value
    */
    inline bool isNextMember()
    {
        if(entityStorage.size() < 1)
            return false;

        if(entityStorage.back().type != STRUCT)
            return false;

        if(entityStorage.back().member)
            return false;

        return true;
    }
    //decrement member count if any ARRAY or STRUCT exist
    inline void decrementMember()
    {
        if(entityStorage.size() < 1)
            return ;

        //decrement member count
        entityStorage.back().numOfItems--;
        //if struct want member name
        if(entityStorage.back().type == STRUCT)
            entityStorage.back().member = false;

        if(entityStorage.back().numOfItems != 0 )
            return;
        //call builder to close entity
        switch(entityStorage.back().type)
        {

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
    long dataWanted;
    long readPosition;
    long errNo;
};

};

#endif
