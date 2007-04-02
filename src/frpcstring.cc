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
 * FILE          $Id: frpcstring.cc,v 1.4 2007-04-02 15:28:20 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
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

String_t::operator std::wstring () const
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

std::string::size_type String_t::size() const
{
    return value.size();
}


const std::string::value_type*  String_t::data() const
{
    return  value.data();
}

const char* String_t::c_str() const
{
return value.c_str();
}

std::string String_t::getString() const
{
    return value;
}

Value_t& String_t::clone(Pool_t &newPool) const
{
    return newPool.String(value);
}

};
