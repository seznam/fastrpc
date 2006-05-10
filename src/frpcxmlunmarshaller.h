/*
 * FILE          $Id: frpcxmlunmarshaller.h,v 1.2 2006-05-10 08:20:11 vasek Exp $
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
