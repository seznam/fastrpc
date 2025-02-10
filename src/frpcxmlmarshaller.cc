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
 * FILE          $Id: frpcxmlmarshaller.cc,v 1.14 2011-01-14 11:16:12 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include <sstream>
#include <limits>
#include <iomanip>
#include <cstring>
#include <cstdio>

#include "frpc.h"
#include "frpclenerror.h"
#include "frpcxmlmarshaller.h"

namespace FRPC {

XmlMarshaller_t::XmlMarshaller_t(Writer_t &writer,
                                 const ProtocolVersion_t &protocolVersion)
    : writer(writer), level(0), mainType(), protocolVersion(protocolVersion) {

    if (protocolVersion.versionMajor > FRPC_MAJOR_VERSION) {
        throw Error_t("Not supported protocol version");
    }

}

XmlMarshaller_t::~XmlMarshaller_t() {
    entityStorage.clear();
}

void XmlMarshaller_t::packArray(unsigned int numOfItems) {

    //write correct spaces
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n",8);
        level++;
    }

    packSpaces(level);
    //write array tag
    writer.write("<value>\n",8);
    level++;
    packSpaces(level);
    writer.write("<array>\n",8);
    level++;


    //write correct spaces
    packSpaces(level);
    //write array tag
    writer.write("<data>\n",7);
    level++;
    if (numOfItems == 0) {
        level--;
        writer.write("</data>\n",8);
        level--;
        writer.write("</array>\n",9);
        level--;
        writer.write("</value>\n",9);
        if ( entityStorage.size() == 0)
            writer.write("</param>\n",9);
        decrementItem();
    } else {
        //entity to storage
        entityStorage.emplace_back(ARRAY, numOfItems);
    }
}

void XmlMarshaller_t::packBinary(const char* value, unsigned int size) {



    //write correct spaces
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n",8);
        level++;
    }
    //write correct spaces
    packSpaces(level);
    //write tags
    writer.write("<value><base64>",15);
    //write value base64
    //writer.write(value,size);
    writeEncodeBase64(writer, value, size);
    //write tags
    writer.write("</base64></value>\n",18);



    if (entityStorage.empty()) {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();

}

void XmlMarshaller_t::packBool(bool value) {

    char boolean = (value ? '1' : '0');
    //write correct spaces
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n",8);
        level++;
    }
    //write correct spaces
    packSpaces(level);
    writer.write("<value><boolean>",16);
    writer.write(&boolean,1);
    writer.write("</boolean></value>\n",19);



    if (entityStorage.empty()) {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();

}

void XmlMarshaller_t::packNull() {
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n",8);
        ++level;
    }
    packSpaces(level);
    writer.write("<value><nil/></value>\n",22);
    if (entityStorage.empty()) {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        --level;
    }
    decrementItem();
}

void XmlMarshaller_t::packBinaryRef(BinaryRefFeeder_t feeder) {
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n", 8);
        level++;
    }
    packSpaces(level);
    writer.write("<value><base64>", 15);
    writeEncodeBase64(writer, feeder.next);
    writer.write("</base64></value>\n",18);
    if (entityStorage.empty()) {
        packSpaces(level - 1);
        writer.write("</param>\n", 9);
        level--;
    }
    decrementItem();
}

void XmlMarshaller_t::packDateTime(short year, char month, char day, char hour,
                                   char minute, char sec, char,
                                   time_t, int timeZone) {
    std::string data = getISODateTime(year, month, day, hour, minute,
                                      sec, timeZone);

    //write correct spaces
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n",8);
        level++;
    }

    //write correct spaces
    packSpaces(level);
    writer.write("<value><dateTime.iso8601>",25);
    writer.write(data.data(), static_cast<uint32_t>(data.size()));
    writer.write("</dateTime.iso8601></value>\n",28);




    if (entityStorage.empty()) {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }

    decrementItem();


}

void XmlMarshaller_t::packDouble(double value) {
    std::ostringstream os;
    os << std::setprecision(std::numeric_limits<double>::digits10 + 2) << value;
    std::string buff = os.str();

    //write correct spaces
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n",8);
        level++;
    }

    //write correct spaces
    packSpaces(level);
    writer.write("<value><double>",15);
    writer.write(buff.data(), static_cast<uint32_t>(buff.size()));
    writer.write("</double></value>\n",18);

    if (entityStorage.empty()) {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();

}

void XmlMarshaller_t::packFault(int errNumber, const char* errMsg, unsigned int size) {

    //magic
    packMagic();
    writer.write("<methodResponse>\n",17);
    level++;
    //tag fault
    packSpaces(level);
    writer.write("<fault>\n",8);
    level++;

    entityStorage.emplace_back(FAULT, 0);

    packStruct(2);
    packStructMember("faultCode",9);
    packInt(errNumber);
    packStructMember("faultString",11);
    packString(errMsg,size);

    packSpaces(level - 1);
    writer.write("</fault>\n",9);
    level--;

    packSpaces(level - 1);
    writer.write("</methodResponse>\n",18);

    mainType = FAULT;

}

void XmlMarshaller_t::packInt(Int_t::value_type value) {

    std::ostringstream buff;
    buff << value;
    //write correct spaces
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n",8);
        level++;
    }

    //write correct spaces
    packSpaces(level);

    Int_t::value_type absValue = value < 0 ? -value :value;

    if ((absValue & INT31_MASK)) {
        if (protocolVersion.versionMajor < 2)
            throw StreamError_t("Number is too big for protocol version 1.0");

        writer.write("<value><i8>",11);
        writer.write(buff.str().data(), static_cast<uint32_t>(buff.str().size()));
        writer.write("</i8></value>\n",14);
    } else {
        writer.write("<value><i4>",11);
        writer.write(buff.str().data(), static_cast<uint32_t>(buff.str().size()));
        writer.write("</i4></value>\n",14);
    }




    if (entityStorage.empty()) {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();
}

void XmlMarshaller_t::packMethodCall(const char* methodName, unsigned int size) {


    if (size > 255 || size == 0)
        throw LenError_t::format(
            "Lenght of method name is %d not in interval (1-255)", size);

    //pack MAgic header
    packMagic();

    writer.write("<methodCall>\n",13);
    level++;
    packSpaces(level);
    writer.write("<methodName>",12);
    writeQuotedString(methodName,size);
    //writer.write(methodName,nameSize);
    writer.write("</methodName>\n",14);
    writer.write("<params>\n",9);
    level++;

    mainType = METHOD_CALL;

}

void XmlMarshaller_t::packMethodResponse() {
    packMagic();
    writer.write("<methodResponse>\n",17);
    level++;
    packSpaces(level);
    writer.write("<params>\n",9);
    level++;

    mainType = METHOD_RESPONSE;

}

void XmlMarshaller_t::packString(const char* value, unsigned int size) {


    //write correct spaces
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n",8);
        level++;
    }
    //write correct spaces
    packSpaces(level);
    //write tags
    writer.write("<value><string>",15);
    //write value
    writeQuotedString(value,size);
    //writer.write(value,strSize);
    //write tags
    writer.write("</string></value>\n",18);

    if (entityStorage.empty()) {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();

}

void XmlMarshaller_t::packStruct(unsigned int numOfMembers) {
    //write correct spaces
    packSpaces(level);
    if (entityStorage.empty()) {
        writer.write("<param>\n",8);
        level++;
    }

    packSpaces(level);
    //write array tag
    writer.write("<value>\n",8);
    level++;
    packSpaces(level);
    writer.write("<struct>\n",9);
    level++;
    if (numOfMembers == 0) {
        level--;
        writer.write("</struct>\n",10);
        level--;
        writer.write("</value>\n",9);
        if ( entityStorage.size() == 0)
            writer.write("</param>\n",9);
        decrementItem();
    } else {

        //entity to storage
        entityStorage.push_back(TypeStorage_t(STRUCT,numOfMembers));
    }

}

void XmlMarshaller_t::packStructMember(const char* memberName, unsigned int size) {


    if (size > 255 || size == 0)
        throw LenError_t::format(
            "Lenght of member name is %d not in interval (1-255)", size);

    packSpaces(level);
    writer.write("<member>\n",9);
    level++;

    packSpaces(level);
    writer.write("<name>",6);
    writeQuotedString(memberName,size);
    //writer.write(memberName,nameSize);
    writer.write("</name>\n",8);

}
void XmlMarshaller_t::flush() {

    switch (mainType) {

    case METHOD_CALL:
        packSpaces(level-1);
        writer.write("</params>\n",10);
        level--;
        packSpaces(level-1);
        writer.write("</methodCall>\n",14);
        break;
    case METHOD_RESPONSE:
        packSpaces(level-1);
        writer.write("</params>\n",10);
        level--;
        packSpaces(level-1);
        writer.write("</methodResponse>\n",18);
        break;
    }
    writer.flush();
    level = 0;
    entityStorage.clear();
}


void XmlMarshaller_t::packMagic() {
    if (protocolVersion.versionMajor < 2) {
        char magic[]="<?xml version=\"1.0\"?>\n";
        //write magic
        writer.write(magic, static_cast<uint32_t>(strlen(magic)));
    } else if (protocolVersion.versionMajor == 2 && protocolVersion.versionMinor == 0) {
        char magic[]="<?xml version=\"1.0\"?>\n<!--protocolVersion=\"2.0\"-->\n";
        //write magic
        writer.write(magic, static_cast<uint32_t>(strlen(magic)));
    } else {
        char magic[]="<?xml version=\"1.0\"?>\n<!--protocolVersion=\"2.1\"-->\n";
        //write magic
        writer.write(magic, static_cast<uint32_t>(strlen(magic)));
    }

}

void XmlMarshaller_t::writeEncodeBase64(Writer_t &writer,
                                        const char *data, unsigned int len,
                                        bool rn)
{
    bool written = false;
    writeEncodeBase64(writer, [&] () -> BinaryRefFeeder_t::Chunk_t {
        if (written) return {nullptr, 0u};
        written = true;
        return {reinterpret_cast<const uint8_t *>(data), len};
    }, rn);
}

void XmlMarshaller_t::writeEncodeBase64(Writer_t &writer,
                                        Chunks_t chunks,
                                        bool rn)
{
    static const char table[]
        = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    auto write_first_sextet = [&] (uint8_t octet0) {
        writer.write(&table[octet0 >> 2], 1);
    };
    auto write_second_sextet = [&] (uint8_t octet0, uint8_t octet1) {
        writer.write(&table[((octet0 & 0x03) << 4) | (octet1 >> 4)], 1);
    };
    auto write_third_sextet = [&] (uint8_t octet1, uint8_t octet2) {
        writer.write(&table[((octet1 & 0x0F) << 2) | (octet2 >> 6)], 1);
    };
    auto write_fourth_sextet = [&] (uint8_t octet2) {
        writer.write(&table[octet2 & 0x3F], 1);
    };

    int input_len = 0;
    size_t lineLen = 0;
    uint8_t input[3];
    for (;;) {
        auto chunk = chunks();
        if (chunk.data == nullptr) break;
        std::size_t i = 0lu;
        for (;;) {
            while ((i < chunk.size) && (input_len != 3))
                input[input_len++] = chunk.data[i++];
            if (input_len < 3) break;
            input_len = 0;

            write_first_sextet(input[0]);
            write_second_sextet(input[0], input[1]);
            write_third_sextet(input[1], input[2]);
            write_fourth_sextet(input[2]);
            if (rn) {
                lineLen += 4;
                if (lineLen > 72) {
                    writer.write("\r\n", 2);
                    lineLen = 0;
                }
            }
        }
    }

    switch (input_len) {
    case 1:
        write_first_sextet(input[0]);
        write_second_sextet(input[0], 0);
        writer.write("==", 2);
        break;
    case 2:
        write_first_sextet(input[0]);
        write_second_sextet(input[0], input[1]);
        write_third_sextet(input[1], 0);
        writer.write("=", 1);
        break;
    default:
        break;
    }
    if (lineLen && rn) writer.write("\r\n", 2);
}

void XmlMarshaller_t::writeQuotedString(const char *data, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        switch (data[i]) {
        case '<':
            writer.write("&lt;",4);
            break;
        case '>':
            writer.write("&gt;",4);
            break;
        case '"':
            writer.write("&quot;",6);
            break;
        case '&':
            writer.write("&amp;",5);
            break;
        default:
            writer.write(&data[i],1);
            break;
        }
    }
}

} // namespace FRPC
