/*
 * FILE          $Id: frpcxmlmarshaller.cc,v 1.2 2005-07-25 06:10:48 vasek Exp $
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
#include "frpcxmlmarshaller.h"
#include <string.h>
#include <stdio.h>
#include <frpclenerror.h>
#include <frpc.h>
namespace FRPC
{




XmlMarshaller_t::~XmlMarshaller_t()
{
    entityStorage.clear();
}


void XmlMarshaller_t::packArray(long numOfItems)
{

    //write correct spaces
    packSpaces(level);
    if(entityStorage.empty())
    {
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
    if(numOfItems == 0)
    {
        level--;
        writer.write("</data>\n",8);
        level--;
        writer.write("</array>\n",9);
        level--;
        writer.write("</value>\n",9);
        if( entityStorage.size() == 0)
            writer.write("</param>\n",9);
        decrementItem();
    }
    else
    {
        //entity to storage
        entityStorage.push_back(TypeStorage_t(ARRAY,numOfItems));
    }
}

void XmlMarshaller_t::packBinary(const char* value, long size)
{



    //write correct spaces
    packSpaces(level);
    if(entityStorage.empty())
    {
        writer.write("<param>\n",8);
        level++;
    }
    //write correct spaces
    packSpaces(level);
    //write tags
    writer.write("<value><base64>",15);
    //write value base64
    //writer.write(value,size);
    writeEncodeBase64(value, size);
    //write tags
    writer.write("</base64></value>\n",18);



    if(entityStorage.empty())
    {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();

}

void XmlMarshaller_t::packBool(bool value)
{

    char boolean = (value==true)?'1':'0';
    //write correct spaces
    packSpaces(level);
    if(entityStorage.empty())
    {
        writer.write("<param>\n",8);
        level++;
    }
    //write correct spaces
    packSpaces(level);
    writer.write("<value><boolean>",16);
    writer.write(&boolean,1);
    writer.write("</boolean></value>\n",19);



    if(entityStorage.empty())
    {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();

}

void XmlMarshaller_t::packDateTime(short year, char month, char day, char hour, char minute, char sec,
                                   char weekDay, time_t unixTime, char timeZone)
{
    std::string data = getISODateTime(year,month,day,hour,minute,sec,timeZone);

    //write correct spaces
    packSpaces(level);
    if(entityStorage.empty())
    {
        writer.write("<param>\n",8);
        level++;
    }

    //write correct spaces
    packSpaces(level);
    writer.write("<value><dateTime.iso8601>",25);
    writer.write(data.data(),data.size());
    writer.write("</dateTime.iso8601></value>\n",28);




    if(entityStorage.empty())
    {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }

    decrementItem();


}

void XmlMarshaller_t::packDouble(double value)
{
    char buff[50];

    sprintf(buff,"%f",value);

    //write correct spaces
    packSpaces(level);
    if(entityStorage.empty())
    {
        writer.write("<param>\n",8);
        level++;
    }

    //write correct spaces
    packSpaces(level);
    writer.write("<value><double>",15);
    writer.write(buff,strlen(buff));
    writer.write("</double></value>\n",18);




    if(entityStorage.empty())
    {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();

}

void XmlMarshaller_t::packFault(long errNumber, const char* errMsg, long size)
{
    long msgSize;
    //obtain a right lenght of string
    msgSize = (size == -1)? strlen(errMsg):size;
    //magic
    packMagic();
    writer.write("<methodResponse>\n",17);
    level++;
    //tag fault
    packSpaces(level);
    writer.write("<fault>\n",8);
    level++;

    entityStorage.push_back(TypeStorage_t(FAULT,0));

    packStruct(2);
    packStructMember("faultCode",9);
    packInt(errNumber);
    packStructMember("faultString",11);
    packString(errMsg,msgSize);

    packSpaces(level - 1);
    writer.write("</fault>\n",9);
    level--;

    packSpaces(level - 1);
    writer.write("</methodResponse>\n",18);

    mainType = FAULT;

}

void XmlMarshaller_t::packInt(long value)
{
    char buff[50];

    sprintf(buff,"%ld",value);

    //write correct spaces
    packSpaces(level);
    if(entityStorage.empty())
    {
        writer.write("<param>\n",8);
        level++;
    }

    //write correct spaces
    packSpaces(level);
    writer.write("<value><int>",12);
    writer.write(buff,strlen(buff));
    writer.write("</int></value>\n",15);




    if(entityStorage.empty())
    {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();


}

void XmlMarshaller_t::packMethodCall(const char* methodName, long size)
{
    long nameSize;
    //obtain a right lenght of string
    nameSize = (size == -1)? strlen(methodName):size;

    if(nameSize > 255 || nameSize == 0)
        throw LenError_t("Lenght of method name is %d not in interval (1-255)",nameSize);

    //pack MAgic header
    packMagic();

    writer.write("<methodCall>\n",13);
    level++;
    packSpaces(level);
    writer.write("<methodName>",12);
    writeQuotedString(methodName,nameSize);
    //writer.write(methodName,nameSize);
    writer.write("</methodName>\n",14);
    writer.write("<params>\n",9);
    level++;

    mainType = METHOD_CALL;

}

void XmlMarshaller_t::packMethodResponse()
{
    packMagic();
    writer.write("<methodResponse>\n",17);
    level++;
    packSpaces(level);
    writer.write("<params>\n",9);
    level++;

    mainType = METHOD_RESPONSE;

}

void XmlMarshaller_t::packString(const char* value, long size)
{
    long strSize;
    //obtain a right lenght of string
    strSize = (size == -1)? strlen(value):size;

    //write correct spaces
    packSpaces(level);
    if(entityStorage.empty())
    {
        writer.write("<param>\n",8);
        level++;
    }
    //write correct spaces
    packSpaces(level);
    //write tags
    writer.write("<value><string>",15);
    //write value
    writeQuotedString(value,strSize);
    //writer.write(value,strSize);
    //write tags
    writer.write("</string></value>\n",18);

    if(entityStorage.empty())
    {
        packSpaces(level - 1);
        writer.write("</param>\n",9);
        level--;
    }
    decrementItem();

}

void XmlMarshaller_t::packStruct(long numOfMembers)
{
    //write correct spaces
    packSpaces(level);
    if(entityStorage.empty())
    {
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
    if(numOfMembers == 0)
    {
        level--;
        writer.write("</struct>\n",10);
        level--;
        writer.write("</value>\n",9);
        if( entityStorage.size() == 0)
            writer.write("</param>\n",9);
        decrementItem();
    }
    else
    {

        //entity to storage
        entityStorage.push_back(TypeStorage_t(STRUCT,numOfMembers));
    }

}

void XmlMarshaller_t::packStructMember(const char* memberName, long size)
{
    long nameSize;
    //obtain a right lenght of string
    nameSize = (size == -1)? strlen(memberName):size;

    if(size > 255 || nameSize == 0)
        throw LenError_t("Lenght of member name is %d not in interval (1-255)",nameSize);

    packSpaces(level);
    writer.write("<member>\n",9);
    level++;

    packSpaces(level);
    writer.write("<name>",6);
    writeQuotedString(memberName,nameSize);
    //writer.write(memberName,nameSize);
    writer.write("</name>\n",8);

}
void XmlMarshaller_t::flush()
{

    switch(mainType)
    {

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


void XmlMarshaller_t::packMagic()
{
    char magic[]="<?xml version=\"1.0\"?>\n";
    //write magic
    writer.write(magic,strlen(magic));

}

void XmlMarshaller_t::writeEncodeBase64(const char *data, long len)
{
    static const char table[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    std::string src(data,len);

    if (src.empty())
    {

        return ;
    }




    size_t lineLen = 0;
    std::string::const_iterator end = src.end();
    for (std::string::const_iterator isrc = src.begin();
            isrc != end; )
    {
        unsigned char input[3];
        int n = 0;
        for (; (isrc != end) && (n < 3) ; isrc++, n++)
            input[n] = *isrc;

        if (n)
        {
            writer.write(&table[input[0] >> 2],1);
            writer.write(&table[((input[0] & 0x03) << 4) | (input[1] >> 4)],1);
            if (n > 1)
                writer.write(&table[((input[1] & 0x0F) << 2) | (input[2] >> 6)],1);
            else
                writer.write("=",1);
            if (n > 2)
                writer.write(&table[input[2] & 0x3F],1);
            else
                writer.write("=",1);
            lineLen += 4;
            if (lineLen > 72)
            {
                writer.write("\r\n",2);
                lineLen = 0;
            }
        }
    }

    if (lineLen)
    {
        writer.write("\r\n",2);
        lineLen = 0;
    }


    return ;
}
void XmlMarshaller_t::writeQuotedString(const char *data, long len)
{
    for(int i = 0; i < len; i++)
    {
        switch(data[i])
        {
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

};
