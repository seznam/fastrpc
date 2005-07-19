/*
 * FILE          $Id: frpcxmlunmarshaller.h,v 1.1 2005-07-19 13:02:55 vasek Exp $
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
#ifndef FRPCFRPCXMLUNMARSHALLER_H
#define FRPCFRPCXMLUNMARSHALLER_H

#include <frpcplatform.h>

#include <frpcunmarshaller.h>
#include <frpcdatabuilder.h>
#include <frpcinternals.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <frpcerror.h>
#include <string.h>



extern "C"
{
    //SAX interface
    static void startDocumentXML(void *p);
    static void endDocumentXML(void *p);

    static void startElementXML(void *p, const xmlChar *name, const xmlChar **attrs);
    static void endElementXML(void *p, const xmlChar *name);
    static void charactersXML(void *p, const  xmlChar *ch, int len);


}

namespace FRPC
{
/**
@author Miroslav Talasek
*/
class XmlUnMarshaller_t : public UnMarshaller_t
{
public:

    XmlUnMarshaller_t(DataBuilder_t & dataBuilder);


    virtual ~XmlUnMarshaller_t();
    void setValueType(const char *name);
    void setValueData(const char *data, long len);
    void closeEntity(const char *name);

    virtual void unMarshall(const char *data, long size, char type);
    virtual void finish();
    //friend class CallbacksInit_t;
    enum {EXC_NONE = 0,EXC_UNKNOWN, EXC_BAD_ALLOC, EXC_STREAM
         };
    char exception;
    std::string exErrMsg;
    long exErrNum;
    std::string localBuffer;

private:

    const std::string decodeBase64(const char *data, long len);
    inline char getValueType(const char *name)
    {
        long len = strlen(name);
        if(len < 2)
            return NONE;

        switch(name[0])
        {
        case 'i':
            {
                if (name[1] == 'n')
                {
                    //int
                    if(strcmp(name,"int") == 0)
                        return INT;
                }
                else if (name[1] == '4' && len == 2)
                {
                    return INT;
                }
                else
                    return NONE;
            }
            break;
        case 'd':
            {
                if(name[1] == 'o')
                {
                    //double
                    if(strcmp(name,"double") == 0)
                        return DOUBLE;
                }
                else if (name[1] == 'a')
                {
                    //date time
                    if(strcmp(name,"dateTime.iso8601") == 0)
                        return DATETIME;
                }
                else
                    return NONE;
            }
            break;
        case 's':
            {
                if(len < 4 )
                    return NONE;

                if(name[3] == 'i')
                {
                    //string
                    if(strcmp(name,"string") == 0)
                        return STRING;
                }
                else if(name[3] == 'u')
                {
                    // struct
                    if(strcmp(name,"struct") == 0)
                        return STRUCT;
                }
                else
                    return NONE;

            }
            break;
        case 'b':
            {

                if(name[1] == 'a')
                {
                    //binary
                    if(strcmp(name,"base64") == 0)
                        return BINARY;
                }
                else if (name[1] == 'o')
                {//bool
                    if(strcmp(name,"boolean") == 0)
                        return BOOL;
                }
                else
                    return NONE;
            }
            break;


        case 'a':
            {
                //array
                if(strcmp(name,"array") == 0)
                    return ARRAY;
            }
            break;
        case 'n':
            {
                //membername structs
                if(strcmp(name,"name") == 0)
                    return MEMBER_NAME;
            }
            break;
        case 'm':
            {
                if (len < 7)
                    return NONE;
                switch(name[6])
                {
                case 'N':
                    {
                        //method name
                        if(strcmp(name,"methodName") == 0)
                            return METHOD_NAME;
                    }
                    break;
                case 'C':
                    {
                        //methodCall
                        if(strcmp(name,"methodCall") == 0)
                            return METHOD_CALL;
                    }
                    break;
                case 'R':
                    {
                        //method response
                        if(strcmp(name,"methodResponse") == 0)
                            return METHOD_RESPONSE;
                    }
                    break;

                default:
                    return NONE;
                }
            }
            break;
        case 'f':
            {
                //fault
                if(strcmp(name,"fault") == 0)
                    return FAULT;
            }
            break;
        default:
            return NONE;
        }
        return NONE;
    }

    //static void initCallbacks();

    DataBuilder_t &dataBuilder;

    char internalType;
    char mainInternalType;
    long internalValue;


    char wantType;

    xmlParserCtxtPtr parser;
    xmlSAXHandler callbacks;


};

};

#endif
