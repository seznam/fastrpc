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
 * FILE          $Id: frpcxmlmarshaller.h,v 1.7 2011-01-10 22:25:15 burlog Exp $
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
#include <functional>
#include <frpcwriter.h>
#include <frpcinternals.h>
#include <frpcint.h>
#include <frpc.h>

//#define XML_HUMAN_FORMAT

namespace FRPC {

/**
@author Miroslav Talasek
*/
class XmlMarshaller_t : public Marshaller_t
{
public:
    XmlMarshaller_t(Writer_t &writer,
                    const ProtocolVersion_t &protocolVersion);

    ~XmlMarshaller_t() override;

    void packArray(unsigned int numOfItems) override;
    void packBinary(const char* value, unsigned int size) override;
    void packBool(bool value) override;
    void packDateTime(short year, char month, char day, char hour,
                              char min, char sec, char weekDay, time_t unixTime,
                              int timeZone) override;
    void packDouble(double value) override;
    void packFault(int errNumber, const char* errMsg,
                           unsigned int size) override;
    void packInt(Int_t::value_type value) override;
    void packMethodCall(const char* methodName, unsigned int size) override;
    void packMethodResponse() override;
    void packString(const char* value, unsigned int size) override;
    void packStruct(unsigned int numOfMembers) override;
    void packStructMember(const char* memberName, unsigned int size) override;
    void flush() override;

    void packNull();

    void packBinaryRef(BinaryRefFeeder_t feeder);

    static void writeEncodeBase64(Writer_t &writer,
                                  const char *data, unsigned int len,
                                  bool rn = true);

#if __cplusplus >= 201103L
    using Chunks_t = std::function<BinaryRefFeeder_t::Chunk_t()>;

    static void writeEncodeBase64(Writer_t &writer,
                                  Chunks_t chunks,
                                  bool rn = true);
#endif

private:
    XmlMarshaller_t();
    void packMagic();
    void writeQuotedString(const char *data, unsigned int len);
#ifdef XML_HUMAN_FORMAT
    inline void packSpaces(unsigned int numSpaces)
    {
        for(unsigned int i=0; i< numSpaces; i++)
            writer.write(" ",1);
    }
#else
    void packSpaces(unsigned int /*numSpaces*/)
    {
    }
#endif

    void decrementItem()
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
    ProtocolVersion_t protocolVersion;
};

} // namespace FRPC

#endif
