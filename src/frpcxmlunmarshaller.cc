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
 * FILE          $Id: frpcxmlunmarshaller.cc,v 1.8 2007-05-22 13:30:59 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#include "frpcxmlunmarshaller.h"
#include <frpc.h>
#include <stdio.h>
#include <string.h>
#include <frpcstreamerror.h>
#include <memory.h>
#include <stdint.h>
#include <errno.h>
using namespace FRPC;



extern "C" {

    static void startDocumentXML(void *p) {
        // printf("Start Dokument\n");
    }

    static void endDocumentXML(void *p) {
        // printf("End Dokument\n");
    }

    static void startElementXML(void *p, const xmlChar *name, const xmlChar **attrs) {
        //ak nieje uz chyba
        if ( reinterpret_cast<XmlUnMarshaller_t*>(p)->
                exception != XmlUnMarshaller_t::EXC_NONE) {
            return ;
        }

        try {
            reinterpret_cast<XmlUnMarshaller_t*>(p)->setValueType((const char*)name);
            reinterpret_cast<XmlUnMarshaller_t*>(p)->localBuffer.erase();
        } catch (const StreamError_t &e) {
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exception = XmlUnMarshaller_t::EXC_STREAM;
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exErrMsg = e.message();
        }
        catch (std::bad_alloc &e) {
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exception = XmlUnMarshaller_t::EXC_BAD_ALLOC;
        } catch (...) {
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exception = XmlUnMarshaller_t::EXC_UNKNOWN;
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exErrMsg = "Unknown error";
        }
    }

    static void endElementXML(void *p, const xmlChar *name) {
        //ak nieje uz chyba
        if ( reinterpret_cast<XmlUnMarshaller_t*>(p)->
                exception != XmlUnMarshaller_t::EXC_NONE) {
            return ;
        }

        try {
            reinterpret_cast<XmlUnMarshaller_t*>(p)->
            setValueData(reinterpret_cast<XmlUnMarshaller_t*>(p)->
                         localBuffer.data(),
                         reinterpret_cast<XmlUnMarshaller_t*>(p)->
                         localBuffer.size());

            reinterpret_cast<XmlUnMarshaller_t*>(p)->closeEntity((const char*)name);
            //printf("Koniec: %s\n",name);
        } catch (const StreamError_t &e) {
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exception = XmlUnMarshaller_t::EXC_STREAM;
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exErrMsg = e.message();
        }
        catch (std::bad_alloc &e) {
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exception = XmlUnMarshaller_t::EXC_BAD_ALLOC;
        } catch (...) {
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exception = XmlUnMarshaller_t::EXC_UNKNOWN;
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exErrMsg = "Unknown error";
        }

    }

    static void charactersXML(void *p, const xmlChar *ch, int len) {
        //ak nieje uz chyba
        if ( reinterpret_cast<XmlUnMarshaller_t*>(p)->
                exception != XmlUnMarshaller_t::EXC_NONE) {
            return ;
        }
        try {
            reinterpret_cast<XmlUnMarshaller_t*>(p)->
            localBuffer.append(reinterpret_cast<const char*>(ch),len);
        } catch (const StreamError_t &e) {
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exception = XmlUnMarshaller_t::EXC_STREAM;
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exErrMsg = e.message();


        }
        catch (std::bad_alloc &e) {
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exception = XmlUnMarshaller_t::EXC_BAD_ALLOC;
        } catch (...) {
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exception = XmlUnMarshaller_t::EXC_UNKNOWN;
            reinterpret_cast<XmlUnMarshaller_t*>
            (p)->exErrMsg = "Unknown error";
        }

    }

}
namespace {
const unsigned char base64Table[256] = {
                                           // 000 - 007
                                           0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 008 - 015
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 016 - 023
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 024 - 031
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 032 - 039
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 040 - 047      '+'                     '/
                                           0x80, 0x80, 0x80, 0x3e, 0x80, 0x80, 0x80, 0x3f,
                                           // 048 - 055      '0'   '1'   '2'   '3'   '4'
                                           0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b,
                                           // 056 - 063
                                           //'5' '6'   '7'   '8'   '9'
                                           0x3c, 0x3d, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 064 - 072
                                           //    'A'   'B'   'C'   'D'   'E'   'F'   'G'
                                           0x80, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                           // 072 - 079
                                           //'H' 'I'   'J'   'K'   'L'   'M'   'N'   'O'
                                           0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
                                           // 080 - 087
                                           //'P' 'Q'   'R'   'S'   'T'   'U'   'V'   'W'
                                           0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
                                           // 088 - 095
                                           //'X' 'Y'   'Z'
                                           0x17, 0x18, 0x19, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 096 - 103
                                           //    'a'   'b'   'c'   'd'   'e'   'f'   'g'
                                           0x80, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
                                           // 104 - 111
                                           //'h' 'i'   'j'   'k'   'l'   'm'   'n'   'o'
                                           0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
                                           // 112 - 119
                                           //'p' 'q'   'r'   's'   't'   'u'   'v'   'w'
                                           0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30,
                                           // 120 - 127
                                           //'x' 'y'   'z'
                                           0x31, 0x32, 0x33, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 128 - 135
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 136 - 143
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 144 - 151
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 152 - 159
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 160 - 167
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 168 - 175
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 176 - 183
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 184 - 191
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 192 - 199
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 200 - 207
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 208 - 215
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 216 - 223
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 224 - 231
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 232 - 239
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 240 - 247
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                                           // 248 - 255
                                           0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
                                       };
const char *error_mapping[] = {
                                  "No error",
                                  "Internal error",
                                  "No memory",
                                  "XML_ERR_DOCUMENT_START",
                                  "Empty document",
                                  "XML_ERR_DOCUMENT_END",
                                  "Invalid hexadecimal character reference",
                                  "Invalid decimal character reference",
                                  "Invalid character reference",
                                  "Invalid character",
                                  "XML_ERR_CHARREF_AT_EOF",
                                  "XML_ERR_CHARREF_IN_PROLOG",
                                  "XML_ERR_CHARREF_IN_EPILOG",
                                  "XML_ERR_CHARREF_IN_DTD",
                                  "XML_ERR_ENTITYREF_AT_EOF",
                                  "XML_ERR_ENTITYREF_IN_PROLOG",
                                  "XML_ERR_ENTITYREF_IN_EPILOG",
                                  "XML_ERR_ENTITYREF_IN_DTD",
                                  "XML_ERR_PEREF_AT_EOF",
                                  "XML_ERR_PEREF_IN_PROLOG",
                                  "XML_ERR_PEREF_IN_EPILOG",
                                  "XML_ERR_PEREF_IN_INT_SUBSET",
                                  "XML_ERR_ENTITYREF_NO_NAME",
                                  "XML_ERR_ENTITYREF_SEMICOL_MISSING",
                                  "XML_ERR_PEREF_NO_NAME",
                                  "XML_ERR_PEREF_SEMICOL_MISSING",
                                  "Undeclared entity error",
                                  "Undeclared entity warning",
                                  "Unparsed Entity",
                                  "XML_ERR_ENTITY_IS_EXTERNAL",
                                  "XML_ERR_ENTITY_IS_PARAMETER",
                                  "Unknown encoding",
                                  "Unsupported encoding",
                                  "XML_ERR_STRING_NOT_STARTED",
                                  "XML_ERR_STRING_NOT_CLOSED",
                                  "Namespace declaration error",
                                  "XML_ERR_ENTITY_NOT_STARTED",
                                  "XML_ERR_ENTITY_NOT_FINISHED",
                                  "XML_ERR_LT_IN_ATTRIBUTE",
                                  "XML_ERR_ATTRIBUTE_NOT_STARTED",
                                  "XML_ERR_ATTRIBUTE_NOT_FINISHED",
                                  "XML_ERR_ATTRIBUTE_WITHOUT_VALUE",
                                  "XML_ERR_ATTRIBUTE_REDEFINED",
                                  "XML_ERR_LITERAL_NOT_STARTED",
                                  "XML_ERR_LITERAL_NOT_FINISHED",
                                  /* "XML_ERR_COMMENT_NOT_STARTED", <= eliminated on purpose */
                                  "XML_ERR_COMMENT_NOT_FINISHED",
                                  "XML_ERR_PI_NOT_STARTED",
                                  "XML_ERR_PI_NOT_FINISHED",
                                  "XML_ERR_NOTATION_NOT_STARTED",
                                  "XML_ERR_NOTATION_NOT_FINISHED",
                                  "XML_ERR_ATTLIST_NOT_STARTED",
                                  "XML_ERR_ATTLIST_NOT_FINISHED",
                                  "XML_ERR_MIXED_NOT_STARTED",
                                  "XML_ERR_MIXED_NOT_FINISHED",
                                  "XML_ERR_ELEMCONTENT_NOT_STARTED",
                                  "XML_ERR_ELEMCONTENT_NOT_FINISHED",
                                  "XML_ERR_XMLDECL_NOT_STARTED",
                                  "XML_ERR_XMLDECL_NOT_FINISHED",
                                  "XML_ERR_CONDSEC_NOT_STARTED",
                                  "XML_ERR_CONDSEC_NOT_FINISHED",
                                  "XML_ERR_EXT_SUBSET_NOT_FINISHED",
                                  "XML_ERR_DOCTYPE_NOT_FINISHED",
                                  "XML_ERR_MISPLACED_CDATA_END",
                                  "XML_ERR_CDATA_NOT_FINISHED",
                                  "XML_ERR_RESERVED_XML_NAME",
                                  "XML_ERR_SPACE_REQUIRED",
                                  "XML_ERR_SEPARATOR_REQUIRED",
                                  "XML_ERR_NMTOKEN_REQUIRED",
                                  "XML_ERR_NAME_REQUIRED",
                                  "XML_ERR_PCDATA_REQUIRED",
                                  "XML_ERR_URI_REQUIRED",
                                  "XML_ERR_PUBID_REQUIRED",
                                  "XML_ERR_LT_REQUIRED",
                                  "XML_ERR_GT_REQUIRED",
                                  "XML_ERR_LTSLASH_REQUIRED",
                                  "XML_ERR_EQUAL_REQUIRED",
                                  "XML_ERR_TAG_NAME_MISMATCH",
                                  "XML_ERR_TAG_NOT_FINISHED",
                                  "XML_ERR_STANDALONE_VALUE",
                                  "XML_ERR_ENCODING_NAME",
                                  "XML_ERR_HYPHEN_IN_COMMENT",
                                  "Invalid encoding",
                                  "XML_ERR_EXT_ENTITY_STANDALONE",
                                  "XML_ERR_CONDSEC_INVALID",
                                  "XML_ERR_VALUE_REQUIRED",
                                  "XML_ERR_NOT_WELL_BALANCED",
                                  "XML_ERR_EXTRA_CONTENT",
                                  "XML_ERR_ENTITY_CHAR_ERROR",
                                  "XML_ERR_ENTITY_PE_INTERNAL",
                                  "XML_ERR_ENTITY_LOOP",
                                  "XML_ERR_ENTITY_BOUNDARY",
                                  "Invalid URI",
                                  "XML_ERR_URI_FRAGMENT",
                                  "XML_WAR_CATALOG_PI",
                                  "XML_ERR_NO_DTD",
                                  "XML_ERR_CONDSEC_INVALID_KEYWORD", /* 95 */
                                  "XML_ERR_VERSION_MISSING", /* 96 */
                                  "XML_WAR_UNKNOWN_VERSION", /* 97 */
                                  "XML_WAR_LANG_VALUE", /* 98 */
                                  "XML_WAR_NS_URI", /* 99 */
                                  "XML_WAR_NS_URI_RELATIVE", /* 100 */
                                  "XML_ERR_MISSING_ENCODING" /* 101 */
                              };
}
XmlUnMarshaller_t::XmlUnMarshaller_t(DataBuilder_t & dataBuilder)
        :exception(0),dataBuilder(dataBuilder),internalType(NONE)
        , mainInternalType(NONE),internalValue(0),versionCheck(true) {


    memset(&callbacks, 0, sizeof(xmlSAXHandler));
    //callbacks.initialized = XML_SAX2_MAGIC;
    callbacks.startDocument = (startDocumentSAXFunc)&startDocumentXML;
    callbacks.endDocument =  (endDocumentSAXFunc)&endDocumentXML;
    callbacks.startElement =  (startElementSAXFunc)&startElementXML;
    callbacks.endElement = (endElementSAXFunc)&endElementXML;
    callbacks.characters = (charactersSAXFunc)&charactersXML;

    parser =  xmlCreatePushParserCtxt(&callbacks,this,0,0,0);


//    xmlCtxtResetPush(parser,0,0,0,"UTF-8");


    if (!parser)
        throw Error_t("Failed to create Xml parser");

}



XmlUnMarshaller_t::~XmlUnMarshaller_t() {

    xmlFreeParserCtxt(parser);
}
void XmlUnMarshaller_t::finish() {

    unMarshall(0,0,NONE);
    if (internalType !=NONE )
        throw StreamError_t("Stream not complette");

}

ProtocolVersion_t XmlUnMarshaller_t::getProtocolVersion() {
    return protocolVersion;
}

void XmlUnMarshaller_t::unMarshall( const char *data, unsigned int size, char type) {

    int terminate = (size == 0)?1:0;
    wantType = type;
    //try obtain version from xml
    std::string versionStr("protocolVersion=\"");
    std::string buffer(data,80);
    std::string::size_type idx = buffer.find(versionStr);
    if (size && versionCheck) {
        if (idx != std::string::npos) {
            protocolVersion.versionMajor = buffer.at(idx
                                           + versionStr.size() ) - 0x30;
            protocolVersion.versionMinor = buffer.at(idx
                                           + versionStr.size() + 2 ) - 0x30;
        } else {
            protocolVersion = ProtocolVersion_t(1,0);
        }
        versionCheck = false;
    }
    //printf("%s",data);
    //fflush(stdout);

    // xmlSwitchEncoding(parser, XML_CHAR_ENCODING_UTF8);
    /* xmlSwitchEncoding(parser,XML_CHAR_ENCODING_NONE);
     
     xmlSwitchToEncoding(parser,
    xmlGetCharEncodingHandler(XML_CHAR_ENCODING_UTF8));  */

    if (xmlParseChunk(parser, data, size , terminate)) {
        const char *msg;
        long code =  parser->errNo;
        if (code < 0 || code >= static_cast<long>(sizeof(error_mapping))) {
            msg =  "Unknown";
        } else
            msg =  error_mapping[code];

        throw StreamError_t("Parser error:< %s >",msg);
    }

    switch (exception) {

    case EXC_STREAM:
        throw StreamError_t(exErrMsg.c_str());
        break;
    case EXC_UNKNOWN:
        throw Error_t(exErrMsg.c_str());
        break;
    case EXC_BAD_ALLOC:
        throw std::bad_alloc();
        break;

    default:
        break;
    }


}

namespace {
inline char getValueType(const char *name) {
    long len = strlen(name);
    if (len < 2)
        return NONE;

    switch (name[0]) {
    case 'i': {
        if (name[1] == 'n') {
            //int
            if (strcmp(name,"int") == 0)
                return INT;
        } else if (name[1] == '4' && len == 2) {
            return INT;
        } else if (name[1] == '8' && len == 2) {
            return INT;
        } else
            return NONE;
    }
    break;
    case 'd': {
        if (name[1] == 'o') {
            //double
            if (strcmp(name,"double") == 0)
                return DOUBLE;
        } else if (name[1] == 'a') {
            //date time
            if (strcmp(name,"dateTime.iso8601") == 0)
                return DATETIME;
        } else
            return NONE;
    }
    break;
    case 's': {
        if (len < 4 )
            return NONE;

        if (name[3] == 'i') {
            //string
            if (strcmp(name,"string") == 0)
                return STRING;
        } else if (name[3] == 'u') {
            // struct
            if (strcmp(name,"struct") == 0)
                return STRUCT;
        } else
            return NONE;

    }
    break;
    case 'b': {

        if (name[1] == 'a') {
            //binary
            if (strcmp(name,"base64") == 0)
                return BINARY;
        } else if (name[1] == 'o') {//bool
            if (strcmp(name,"boolean") == 0)
                return BOOL;
        } else
            return NONE;
    }
    break;


    case 'a': {
        //array
        if (strcmp(name,"array") == 0)
            return ARRAY;
    }
    break;
    case 'n': {
        //membername structs
        if (strcmp(name,"name") == 0)
            return MEMBER_NAME;
    }
    break;
    case 'm': {
        if (len < 7)
            return NONE;
        switch (name[6]) {
        case 'N': {
            //method name
            if (strcmp(name,"methodName") == 0)
                return METHOD_NAME;
        }
        break;
        case 'C': {
            //methodCall
            if (strcmp(name,"methodCall") == 0)
                return METHOD_CALL;
        }
        break;
        case 'R': {
            //method response
            if (strcmp(name,"methodResponse") == 0)
                return METHOD_RESPONSE;
        }
        break;

        default:
            return NONE;
        }
    }
    break;
    case 'f': {
        //fault
        if (strcmp(name,"fault") == 0)
            return FAULT;
    }
    break;
    case 'v': {
        // value: default to string
        if (strcmp(name, "value") == 0)
            return STRING;
    }
    break;
    default:
        return NONE;
    }
    return NONE;
}
}

void XmlUnMarshaller_t::setValueType(const char *name) {



    char type;
    switch (type = getValueType(name)) {
    case INT:
    case BOOL:
    case DATETIME:
    case STRING:
    case BINARY:
    case MEMBER_NAME:
    case METHOD_NAME:
    case DOUBLE:
    case NONE:
        if (mainInternalType == TYPE_METHOD_RESPONSE) {
            dataBuilder.buildMethodResponse();
            mainInternalType = NONE;
        }

        internalType = type;
        break;
    case STRUCT:
        if (mainInternalType == TYPE_METHOD_RESPONSE) {
            dataBuilder.buildMethodResponse();
            mainInternalType = NONE;
        }
        if (mainInternalType != TYPE_FAULT)
            dataBuilder.openStruct(0);
        break;
    case ARRAY:
        if (mainInternalType == TYPE_METHOD_RESPONSE) {
            dataBuilder.buildMethodResponse();
            mainInternalType = NONE;
        }
        dataBuilder.openArray(0);
        break;
    case FAULT:
    case METHOD_CALL:
    case METHOD_RESPONSE:
        mainInternalType = type;
        if (mainInternalType != wantType && wantType != TYPE_ANY ) {
            if (mainInternalType != TYPE_FAULT || wantType != TYPE_METHOD_RESPONSE) {
                throw StreamError_t("Bad main Type !!!");
            }
        }
        break;
    default:

        break;

    }

}

const std::string XmlUnMarshaller_t::decodeBase64(const char *data, long len) {
    std::string src(data,len);

    if (src.empty()) {

        return src;
    }

    std::string tmpDest;
    tmpDest.reserve((src.length() * 3) / 4);

    std::string::const_iterator end = src.end();
    for (std::string::const_iterator isrc = src.begin();
            isrc != end; ) {
        unsigned char a[4] = { '=', '=', '=', '=' };
        unsigned char b[4];
        int i = 0;
        for (; (isrc != end) && (i < 4); isrc++) {
            if (isspace(*isrc))
                continue;
            b[i] = base64Table[a[i] = *isrc];
            i++;
        }

        // when nothing read terminate this loop
        if (!i)
            break;

        unsigned char o[4];
        o[0] = (b[0] << 2) | (b[1] >> 4);
        o[1] = (b[1] << 4) | (b[2] >> 2);
        o[2] = (b[2] << 6) | b[3];
        int len = (a[2] == '=') ? 1 : ((a[3] == '=') ? 2 : 3);
        tmpDest.append((char *)o, len);
        if (len < 3)
            break;
    }

    return  tmpDest;
}


void XmlUnMarshaller_t::setValueData(const char *data, unsigned int len) {


    if (len == 1) {
        if (data[0] == '\n') {
            return;
        }
    }
    switch (internalType) {
    case NONE: {}
    break;
    case METHOD_NAME: {
        dataBuilder.buildMethodCall(data,len);
        mainInternalType = NONE;
        internalType = NONE;
    }
    break;
    case MEMBER_NAME: {
        if (mainInternalType != FAULT)
            dataBuilder.buildStructMember(data, len);
        internalType = NONE;
    }
    break;
    case DOUBLE: {
        std::string doubleStr(data,len);
        char *end;
        double value = strtod(doubleStr.c_str(),&end);
        if (*end)
            throw StreamError_t("Integrity xml Double error !!!");

        //biuld double
        dataBuilder.buildDouble(value);
        internalType = NONE;

    }
    break;
    case INT: {
        std::string intStr(data,len);
        char *end ;
        long value = strtol(intStr.c_str(),&end,10);
        if (*end)
            throw StreamError_t("Integrity xml Int error !!!");

        if ((value == LONG_MIN || value == LONG_MAX ) && errno == ERANGE) {
            throw StreamError_t("Unsupported size of int (too big) !!!");
        }
        if (mainInternalType == FAULT)
            //it is errNum
            internalValue = value;
        else
            //build int
            dataBuilder.buildInt(value);
        internalType = NONE;
    }
    break;
    case STRING: {
        if (mainInternalType == FAULT) {
            dataBuilder.buildFault(internalValue, data, len);
            //mainInternalType = NONE;
        } else {
            dataBuilder.buildString(data, len);
        }
        internalType = NONE;
    }
    break;
    case BINARY: {

        std::string binary = decodeBase64(data,len);
        dataBuilder.buildBinary(binary.data(), binary.size());
        internalType = NONE;
    }
    break;
    case BOOL: {
        dataBuilder.buildBool((data[0] == '1'));
        internalType = NONE;
    }
    break;
    case DATETIME: {

        // date vars
        short year;
        char timeZone, month, day, hour, minute, sec;

        parseISODateTime(data, len, year, month, day, hour, minute, sec, timeZone);

        dataBuilder.buildDateTime(year,month,day,hour,minute,sec,-1,-1,
                                  timeZone);
        internalType = NONE;
    }

    }


}
void XmlUnMarshaller_t::closeEntity(const char *name) {
    char type;
    internalType = NONE;

    if (mainInternalType == FAULT)
        return;

    switch (type = getValueType(name)) {
    case STRUCT:
        dataBuilder.closeStruct();
        break;
    case ARRAY:
        dataBuilder.closeArray();
        break;
    default:
        break;
    }
}
