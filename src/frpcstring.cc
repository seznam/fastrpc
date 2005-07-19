/*
 * FILE          $Id: frpcstring.cc,v 1.1 2005-07-19 13:02:54 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2005
 * All Rights Reserved
 *
 * HISTORY
 *       
 */
#include "frpcstring.h"
#include "frpcpool.h"

#ifdef WIN32
#include <windows.h>
#include "frpcerror.h"
#endif //WIN32

namespace FRPC
{




String_t::~String_t()
{}

String_t::String_t(Pool_t &pool,std::string::value_type *pData, std::string::size_type dataSize)
        :Value_t(pool),value(pData,dataSize)
{
}

String_t::String_t(Pool_t &pool, const std::string &value)
        :Value_t(pool),value(value)
{
}


String_t::String_t(Pool_t &pool, const std::wstring &value_w)
        :Value_t(pool)

{
#ifdef WIN32

    LPCWSTR wszValue = value_w.c_str();
    int iMultiByteValueLen = WideCharToMultiByte(CP_UTF8, NULL, wszValue, 
                                                value_w.length(), NULL, 0, NULL, NULL);

    LPSTR szValue = new char [iMultiByteValueLen + 1];
    memset (szValue, 0, iMultiByteValueLen + 1);
    if (iMultiByteValueLen == WideCharToMultiByte(CP_UTF8, NULL, wszValue, 
                                                value_w.length(), szValue, 
                                                iMultiByteValueLen + 1, NULL, NULL))
        value = szValue;
    else
        throw TypeError_t("Cannot convert %S from multibyte to widechar.", value_w.c_str());


    delete szValue;
#else //WIN32
#endif //WIN32
}

String_t::operator const std::wstring () const
{
#ifdef WIN32
    std::wstring _value_w;
    LPCSTR szValue = value.c_str();
    int iWideCharValueLen = MultiByteToWideChar(CP_UTF8, NULL, szValue, 
                                                value.length(), NULL, 0);

    LPWSTR wszValue = new wchar_t [iWideCharValueLen + 1];
    memset ((LPVOID)szValue, 0, (iWideCharValueLen + 1) * sizeof(wchar_t));
    if (iWideCharValueLen == MultiByteToWideChar(CP_UTF8, NULL, szValue, 
                                                value.length(), wszValue, 
                                                iWideCharValueLen + 1))
        _value_w = wszValue;
    else
        throw TypeError_t("Cannot convert %s from widechar to multibyte.", value.c_str());

    delete szValue;
    
    return _value_w;
#else //WIN32
    std::wstring _value_w(L"");
    return _value_w;
#endif //WIN32
}

String_t::operator std::wstring ()
{
#ifdef WIN32
    std::wstring _value_w(L"");
    LPCSTR szValue = value.c_str();
    int iWideCharValueLen = MultiByteToWideChar(CP_UTF8, NULL, szValue, 
                                                value.length(), NULL, 0);

    LPWSTR wszValue = new wchar_t [iWideCharValueLen + 1];
    memset ((LPVOID)wszValue, 0, (iWideCharValueLen + 1) * sizeof(wchar_t));
    if (iWideCharValueLen == MultiByteToWideChar(CP_UTF8, NULL, szValue, 
                                                value.length(), wszValue, 
                                                iWideCharValueLen + 1))
        _value_w = wszValue;
    else
        throw TypeError_t("Cannot convert %s from widechar to multibyte.", value.c_str());

    delete wszValue;
    
    return _value_w;
#else //WIN32
    std::wstring _value_w(L"");
    return _value_w;
#endif //WIN32
}

std::string::size_type String_t::size()
{
    return value.size();
}


const std::string::value_type*  String_t::data()
{
    return  value.data();
}

const char* String_t::c_str()
{
return value.c_str();
}

std::string String_t::getString()
{
    return value;
}

Value_t& String_t::clone(Pool_t &newPool) const
{
    return newPool.String(value);
}

};
