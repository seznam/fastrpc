/*
 * FILE          $Id: frpcxmlmarshaller.h,v 1.1 2005-07-19 13:02:55 vasek Exp $
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
#ifndef FRPCFRPCXMLMARSHALLER_H
#define FRPCFRPCXMLMARSHALLER_H

#include <frpcmarshaller.h>
#include <vector>
#include <frpcwriter.h>
#include <frpcinternals.h>
//#define XML_HUMAN_FORMAT
namespace FRPC
{




/**
@author Miroslav Talasek
*/
class XmlMarshaller_t : public Marshaller_t
{
public:
    XmlMarshaller_t(Writer_t &writer):writer(writer),level(0)
    {}

    virtual ~XmlMarshaller_t();

    virtual void packArray(long numOfItems);
    virtual void packBinary(const char* value, long size);
    virtual void packBool(bool value);
    virtual void packDateTime(short year, char month, char day, char hour, char min, char sec, char weekDay, time_t unixTime, char timeZone);
    virtual void packDouble(double value);
    virtual void packFault(long errNumber, const char* errMsg, long size);
    virtual void packInt(long value);
    virtual void packMethodCall(const char* methodName, long size);
    virtual void packMethodResponse();
    virtual void packString(const char* value, long size);
    virtual void packStruct(long numOfMembers);
    virtual void packStructMember(const char* memberName, long size);
    virtual void flush();
    
private:
    XmlMarshaller_t();
    void writeEncodeBase64(const char *data, long len);
    void packMagic();
    void writeQuotedString(const char *data, long len);
    inline void packSpaces(long numSpaces)
    {
#ifdef XML_HUMAN_FORMAT
        for(int i=0; i< numSpaces; i++)
            writer.write(" ",1);
#endif
    }
    inline void decrementItem()
    {
        //is vaule any ittem or no ?
        if(!entityStorage.empty())
        {
            if(entityStorage.back().type == STRUCT)
            {
               packSpaces(level-1);
               writer.write("</member>\n",10);
               level--;
            }
            //decrement item count
            entityStorage.back().numOfItems--;
            //is value last item ?
            if(entityStorage.back().numOfItems == 0)
            {
                
                //close tag
                packSpaces(level-1);
                switch(entityStorage.back().type)
                {
                case ARRAY:
                    writer.write("</data>\n",8);
                    level--;
                    packSpaces(level-1);
                    writer.write("</array>\n",9);
                    level--;
                    packSpaces(level-1);
                    writer.write("</value>\n",9);
                    break;
                case STRUCT:
                    
                    writer.write("</struct>\n",10);
                    level--;
                    packSpaces(level-1);
                    writer.write("</value>\n",9);
                    break;
                }
                level--;
                entityStorage.pop_back();
                
                if(entityStorage.empty())
                {
                    packSpaces(level-1);
                    writer.write("</param>\n",9);
                    level--;
                }
                decrementItem();
            }
        }

    }
    std::vector<TypeStorage_t> entityStorage;
    Writer_t  &writer;
    long level;
    char mainType;


};

};

#endif
