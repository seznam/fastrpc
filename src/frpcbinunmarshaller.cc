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
 * FILE          $Id: frpcbinunmarshaller.cc,v 1.5 2007-05-16 08:50:09 mirecta Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#include "frpcbinunmarshaller.h"

#include <memory.h>
#include <frpcstreamerror.h>


#define FRPC_GET_DATA_TYPE( data ) (data >> 3)
#define FRPC_GET_DATA_TYPE_INFO( data ) (data & 0x07 )
namespace FRPC
{




BinUnMarshaller_t::~BinUnMarshaller_t()
{}


void BinUnMarshaller_t::finish()
{
    if(internalType !=NONE || entityStorage.size() > 0)
        throw StreamError_t("Stream not complette");

}

void BinUnMarshaller_t::unMarshall(const char *data, long size, char type)
{
    char magic[]={0xCA, 0x11, FRPC_MAJOR_VERSION, FRPC_MINOR_VERSION};



    while(true)
    {
        if(readData(data, size) != 0 )
            return;

        switch(internalType)
        {
        case MAGIC:
            {
                if(memcmp(mainBuff.data(), magic, 4) != 0)
                {
                    //bad magic or version
                    throw StreamError_t("Bad magic !!!");
                }
                mainBuff.erase();
                internalType = MAIN;

                dataWanted = 1;
            }
            break;

        case MAIN:
            {
                mainInternalType =  FRPC_GET_DATA_TYPE(mainBuff[0]);
                mainBuff.erase();

                if(mainInternalType != type && type != TYPE_ANY )
                {

                    if(mainInternalType != FAULT || type != TYPE_METHOD_RESPONSE)
                    {
                        throw StreamError_t("Bad main Type !!!");
                    }
                }

                if(mainInternalType == METHOD_CALL)
                    internalType = METHOD_NAME_LEN;
                else
                    internalType = NONE;

                if(mainInternalType == METHOD_RESPONSE)
                {
                    dataBuilder.buildMethodResponse();
                    mainInternalType = NONE;
                }
                dataWanted = 1;
            }
            break;
        case METHOD_NAME_LEN:
            {
                dataWanted = static_cast<unsigned char>(mainBuff[0]);
                internalType = METHOD_NAME;
                mainBuff.erase();
            }
            break;
        case METHOD_NAME:
            {
                dataBuilder.buildMethodCall(mainBuff);
                mainBuff.erase();
                mainInternalType = NONE;
                internalType = NONE;
                dataWanted = 1;
            }
            break;
        case MEMBER_NAME:
            {
                dataBuilder.buildStructMember(mainBuff);
                //we have member name
                entityStorage.back().member = true;

                mainBuff.erase();
                internalType = NONE;
                dataWanted = 1;
            }
            break;
        case NONE:
            {
                if(isNextMember())
                {
                    dataWanted = static_cast<unsigned char>(mainBuff[0]);
                    mainBuff.erase();
                    internalType = MEMBER_NAME;
                }
                else
                {
                    switch(FRPC_GET_DATA_TYPE(mainBuff[0]))
                    {
                    case BOOL:
                        {
                            dataBuilder.buildBool(FRPC_GET_DATA_TYPE_INFO(mainBuff[0]) & 0x01);
                            //decrement member count
                            internalType = NONE;
                            mainBuff.erase();
                            dataWanted = 1;
                            decrementMember();
                        }
                        break;
                    case INT:
                        {
                            internalType = INT;
                            dataWanted = FRPC_GET_DATA_TYPE_INFO(mainBuff[0]);
                            if(dataWanted > 4 || !dataWanted)
                                throw StreamError_t("Size of int is 0 or > 4 !!!");
                            mainBuff.erase();
                        }
                        break;
                    case DOUBLE:
                        {
                            internalType = DOUBLE;
                            dataWanted = 8;
                            mainBuff.erase();
                        }
                        break;
                    case DATETIME:
                        {
                            internalType = DATETIME;
                            dataWanted = 10;
                            mainBuff.erase();
                        }
                        break;

                    case STRING:
                        {
                            internalType = STRING;
                            typeEvent = LENGTH;
                            dataWanted = FRPC_GET_DATA_TYPE_INFO(mainBuff[0]);
                            if(!dataWanted)
                                throw StreamError_t("Size of string length is 0 !!!");
                            mainBuff.erase();
                        }
                        break;
                    case BINARY:
                        {
                            internalType = BINARY;
                            typeEvent = LENGTH;
                            dataWanted = FRPC_GET_DATA_TYPE_INFO(mainBuff[0]);
                            if(!dataWanted)
                                throw StreamError_t("Size of binary length is 0 !!!");
                            mainBuff.erase();
                            
                        }
                        break;
                    case ARRAY:
                        {
                            internalType = ARRAY;
                            dataWanted = FRPC_GET_DATA_TYPE_INFO(mainBuff[0]);
                            mainBuff.erase();
                        }
                        break;
                    case STRUCT:
                        {
                            internalType = STRUCT;
                            dataWanted = FRPC_GET_DATA_TYPE_INFO(mainBuff[0]);
                            mainBuff.erase();
                        }
                        break;
                    default:
                        throw StreamError_t("Don't known this type");
                        break;

                    }

                }

            }
            break;
            //Types
        case STRING:
            {
                if(typeEvent == LENGTH)
                {
                    //unpack string len
                    Number_t stringSize(mainBuff.data(), mainBuff.size());
                    dataWanted = stringSize.number;
                    mainBuff.erase();
                    if(!dataWanted)
                    {
                        if(mainInternalType == FAULT)
                        {
                            dataBuilder.buildFault(errNo, mainBuff);
                            mainInternalType = NONE;
                            internalType = NONE;
                           
                        }
                        else
                        {
                            dataBuilder.buildString(mainBuff);
                            internalType = NONE;
                            dataWanted = 1;
                            decrementMember();
                        }
                    }
                    else
                    {
                        typeEvent = DATA;
                    }
                }
                else
                {
                    //obtain whole string
                    if(mainInternalType == FAULT)
                    {
                        dataBuilder.buildFault(errNo, mainBuff);
                        mainInternalType = NONE;
                        internalType = NONE;
                        mainBuff.erase();
                    }
                    else
                    {
                        dataBuilder.buildString(mainBuff);
                        internalType = NONE;
                        dataWanted = 1;
                        decrementMember();
                        mainBuff.erase();
                    }
                }
            }
            break;
        case BINARY:
            {
                if(typeEvent == LENGTH)
                {
                    //unpack string len
                    Number_t stringSize(mainBuff.data(), mainBuff.size());
                    dataWanted = stringSize.number;
                    mainBuff.erase();
                    if(!dataWanted)
                    {
                        dataBuilder.buildBinary(mainBuff);
                        internalType = NONE;
                        dataWanted = 1;
                        decrementMember();
                    }
                    else
                    {
                        typeEvent = DATA; 
                    }
                }
                else
                {
                    //obtain whole data
                    dataBuilder.buildBinary(mainBuff);
                    internalType = NONE;
                    dataWanted = 1;
                    decrementMember();
                    mainBuff.erase();
                }


            }
            break;
        case INT:
            {
                //unpack value
                Number_t value(mainBuff.data(), mainBuff.size());
                if(mainInternalType == FAULT)
                    errNo = value.number;
                else
                {//call builder
                    //                     if(value.number == 63808)
                    //                     {
                    //                         printf("ee\n");
                    //                         fflush(stdout);
                    //                     }
                    dataBuilder.buildInt(value.number);
                }
                internalType = NONE;
                dataWanted = 1;
                //decrement member count
                decrementMember();
                mainBuff.erase();
            }
            break;
        case DOUBLE:
            {
                double value;
                memcpy((char*)&value,mainBuff.data(),8);

                //call builder
                dataBuilder.buildDouble(value);
                internalType = NONE;
                dataWanted = 1;
                //decrement member count
                decrementMember();
                mainBuff.erase();
            }
            break;
        case DATETIME:
            {
                DateTimeData_t dateTime;

                memcpy(dateTime.data,mainBuff.data(),10);
                //unpack 
                dateTime.unpack();
                
                if(dateTime.dateTime.year || dateTime.dateTime.month || dateTime.dateTime.day
                        || dateTime.dateTime.hour || dateTime.dateTime.minute || dateTime.dateTime.sec)
                {
                    //call builder
                    dataBuilder.buildDateTime(dateTime.dateTime.year + 1600, dateTime.dateTime.month,
                                              dateTime.dateTime.day, dateTime.dateTime.hour,
                                              dateTime.dateTime.minute,dateTime.dateTime.sec,
                                              dateTime.dateTime.weekDay,
                                              dateTime.dateTime.unixTime,
                                              dateTime.dateTime.timeZone);
                }
                else
                {
                    dataBuilder.buildDateTime(dateTime.dateTime.year, dateTime.dateTime.month,
                                              dateTime.dateTime.day, dateTime.dateTime.hour,
                                              dateTime.dateTime.minute,dateTime.dateTime.sec,
                                              dateTime.dateTime.weekDay,
                                              dateTime.dateTime.unixTime,
                                              dateTime.dateTime.timeZone);
                }
                internalType = NONE;
                dataWanted = 1;
                //decrement member count
                decrementMember();
                mainBuff.erase();

            }
            break;
        case STRUCT:
            {
                Number_t numOfMembers(mainBuff.data(), mainBuff.size());

                //call builder
                dataBuilder.openStruct(numOfMembers.number);
                if(numOfMembers.number != 0)
                {
                    //save event
                    entityStorage.push_back(TypeStorage_t(STRUCT, numOfMembers.number));
                }
                else
                {
                    dataBuilder.closeStruct();
                    decrementMember();
                }
                internalType = NONE;
                dataWanted = 1;
                mainBuff.erase();
            }
            break;
        case ARRAY:
            {
                //unpack number
                Number_t numOfItems(mainBuff.data(), mainBuff.size());

                //call builder
                dataBuilder.openArray(numOfItems.number);
                //save evnt
                if(numOfItems.number != 0)
                {
                    entityStorage.push_back(TypeStorage_t(ARRAY, numOfItems.number));
                }
                else
                {
                    dataBuilder.closeArray();
                    decrementMember();
                }
                internalType = NONE;
                dataWanted = 1;
                mainBuff.erase();

            }
            break;

        }
    }


}

}

