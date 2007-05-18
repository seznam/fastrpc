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
 * FILE          $Id: frpcxmlmarshaller.h,v 1.3 2007-05-18 15:29:46 mirecta Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
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
#include <frpcint.h>
//#define XML_HUMAN_FORMAT
namespace FRPC
{




/**
@author Miroslav Talasek
*/
class XmlMarshaller_t : public Marshaller_t
{
public:
    XmlMarshaller_t(Writer_t &writer);

    virtual ~XmlMarshaller_t();

    virtual void packArray(unsigned int numOfItems);
    virtual void packBinary(const char* value, unsigned int size);
    virtual void packBool(bool value);
    virtual void packDateTime(short year, char month, char day, char hour, char min, char sec, char weekDay, time_t unixTime, char timeZone);
    virtual void packDouble(double value);
    virtual void packFault(int errNumber, const char* errMsg, unsigned int size);
    virtual void packInt(Int_t::value_type value);
    virtual void packMethodCall(const char* methodName, unsigned int size);
    virtual void packMethodResponse();
    virtual void packString(const char* value, unsigned int size);
    virtual void packStruct(unsigned int numOfMembers);
    virtual void packStructMember(const char* memberName, unsigned int size);
    virtual void flush();
    
private:
    XmlMarshaller_t();
    void writeEncodeBase64(const char *data, unsigned int len);
    void packMagic();
    void writeQuotedString(const char *data, unsigned int len);
    inline void packSpaces(unsigned int numSpaces)
    {
#ifdef XML_HUMAN_FORMAT
        for(unsigned int i=0; i< numSpaces; i++)
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
    unsigned int level;
    char mainType;

};

};

#endif
