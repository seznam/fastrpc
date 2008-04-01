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
 * FILE          $Id: frpc.cc,v 1.13 2008-04-01 13:19:03 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */


#include <frpc.h>
#include <frpcinternals.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>



/**
* @brief dummy method for test in configure
*/
extern "C" {
    int FRPC_DLLEXPORT dummyFastRPC() {
        return 0;
    }
}

namespace FRPC {

/**
*@brief method render iso date time forma from parameters

*/
std::string getISODateTime(short year, char month,
                           char day, char hour,
                           char minute, char sec, int timeZone) {
    char dateTime[50];
    sprintf(dateTime, "%04d%02d%02dT%02d:%02d:%02d%c%02d%02d",
            year, month, day, hour, minute, sec,
            ((timeZone <= 0)? '+': '-'),
            abs(timeZone / 60 / 60), abs(timeZone / 60 % 60));

    return std::string(dateTime);
}

/**
@brief method parse date time iso format from string data and his length
and fill parameters
*/

void parseISODateTime(const char *data, long len, short &year, char &month,
                      char &day, char &hour, char &minute, char &sec,
                      int &timeZone) {

    year = month = day = hour = minute = sec = timeZone = 0;
    // iterators
    char *sit = const_cast<char*>(data);
    char *end = const_cast<char*>(data)+len;


    // skip leading spaces
    while (sit != end && isspace(*sit))
        ++sit;
    // get year
    std::string tmp;
    int i = 0;
    while (sit != end && i < 4 && isdigit(*sit)) {
        tmp += *sit;
        ++sit;
        ++i;
    }
    if (tmp.length() == 0)
        throw StreamError_t("Bad DATE format");
    year = atoi(tmp.c_str());

    // skip optional delimiter
    if (sit != end && *sit == '-')
        ++sit;
    // get month
    tmp.erase();
    i = 0;
    while (sit != end && i < 2 && isdigit(*sit)) {
        tmp += *sit;
        ++sit;
        ++i;
    }
    if (tmp.length() == 0)
        throw StreamError_t("Bad DATE format");
    month = atoi(tmp.c_str());

    // skip optional delimiter
    if (sit != end && *sit == '-')
        ++sit;
    // get day
    tmp.erase();
    i = 0;
    while (sit != end && i < 2 && isdigit(*sit)) {
        tmp += *sit;
        ++sit;
        ++i;
    }
    if (tmp.length() == 0)
        throw StreamError_t("Bad DATE format");
    day = atoi(tmp.c_str());

    // skip time delimiter
    if (sit != end && (*sit == 'T' || *sit == 't' || *sit == ' ' ))
        ++sit;
    else
        return;
    //throw StreamError_t("Bad DATE format no time");

    // get hour
    tmp.erase();
    i = 0;
    while (sit != end && i < 2 && isdigit(*sit)) {
        tmp += *sit;
        ++sit;
        ++i;
    }
    if (tmp.length() == 0)
        return;
    //throw StreamError_t("Bad DATE format");
    hour = atoi(tmp.c_str());

    // skip optional delimiter
    if (sit != end && *sit == ':')
        ++sit;
    // get min
    tmp.erase();
    i = 0;
    while (sit != end && i < 2 && isdigit(*sit)) {
        tmp += *sit;
        ++sit;
        ++i;
    }
    if (tmp.length() == 0)
        return;
    //throw StreamError_t("Bad DATE format");
    minute = atoi(tmp.c_str());

    // skip optional delimiter
    if (sit != end && *sit == ':')
        ++sit;
    // get sec
    tmp.erase();
    i = 0;
    while (sit != end && i < 2 && isdigit(*sit)) {
        tmp += *sit;
        ++sit;
        ++i;
    }
    if (tmp.length() == 0)
        return;
    //throw StreamError_t("Bad DATE format");
    sec = atoi(tmp.c_str());

    // if sec fraction
    if (sit != end && *sit == '.') {
        ++sit; //skip delimiter
        while (sit != end && isdigit(*sit))
            ++sit; //sec fraction is ignored
    }

    // timezone vars
    int tzsign, tzhour, tzmin;
    tzsign = 0;
    tzhour = 0;
    tzmin = 0;

    // timezone sign +/- or 'Z' for UTC
    if (sit != end) {
        tzsign = *sit;
        ++sit;
    }
    // if timezone specified
    if (tzsign == 'Z' || tzsign == 'z') {
        // Z means zero meridian GMT+0
        tzhour = 0;
        tzmin = 0;
    } else if (tzsign == '+' || tzsign == '-') {
        // get TZ hour
        tmp.erase();
        i = 0;
        while (sit != end && i < 2 && isdigit(*sit)) {
            tmp += *sit;
            ++sit;
            ++i;
        }
        if (tmp.length() == 0)
            return;
        //throw StreamError_t("Bad DATE format");
        tzhour = atoi(tmp.c_str());

        // skip optional delimiter
        if (sit != end && *sit == ':')
            ++sit;
        // get TZ min
        tmp.erase();
        i = 0;
        while (sit != end && i < 2 && isdigit(*sit)) {
            tmp += *sit;
            ++sit;
            ++i;
        }
        if (tmp.length() == 0)
            return;
        //throw StreamError_t("Bad DATE format");
        tzmin = atoi(tmp.c_str());
    }

    // skip trailing spaces
    while (sit != end && isspace(*sit))
        ++sit;
    if (sit != end)
        return;
    //throw StreamError_t("Bad DATE format"); //error, extra characters on line

    timeZone = (tzhour * 60 + tzmin) * ((tzsign == '+')? -60: 60);

}

/**
@brief method render fastrpc Value_t to string with level level
@param value  - fast rpc value
@param outstr - out string
@param level - level of render
@return always zero
*/
int dumpFastrpcTree(const Value_t &value, std::string &outstr, int level) {

    std::ostringstream out;

    switch (value.getType()) {
    case Int_t::TYPE: {
            out << Int(value).getValue();

        }
        break;

    case Bool_t::TYPE: {
            out << (Bool(value).getValue() ? "true" : "false");
        }
        break;

    case Double_t::TYPE: {
            out << Double(value).getValue();
        }
        break;

    case String_t::TYPE: {

            out << '"';
            if (String(value).size() > MAX_LEN)
                out << String(value).getString().substr(0,MAX_LEN) << "...";
            else
                out << String(value).getString();
            out << '"';


        }
        break;

    case Binary_t::TYPE: {

            out << "b\"";
            if (Binary(value).size() > MAX_LEN)
                out << Binary(value).getString().substr(0,MAX_LEN) << "...";
            else
                out << Binary(value).getString();
            out << '"';

        }
        break;

    case DateTime_t::TYPE: {
            const DateTime_t &dt = DateTime(value);
            char buff[50];
            sprintf(buff, "%04d%02d%02dT%02d:%02d:%02d%c%02d%02d",
                    dt.getYear(), dt.getMonth(), dt.getDay(), dt.getHour(),
                    dt.getMin(), dt.getSec(),
                    ((dt.getTimeZone() <= 0)? '+': '-'),
                    abs(dt.getTimeZone() / 60 / 60),
                    abs(dt.getTimeZone() / 60 % 60));

            out << buff;

        }
        break;

    case Struct_t::TYPE: {
            const Struct_t &structVal = Struct(value);
            bool first = true;
            out << '{';

            if (level) {
                for (Struct_t::const_iterator
                        i = structVal.begin();
                        i != structVal.end(); ++i) {

                    if (i->second && first != true)
                        out << ", ";
                    else
                        first = false;

                    std::string val_str;
                    dumpFastrpcTree(*(i->second), val_str, level - 1);
                    out << i->first << ": " << val_str;

                }
            } else
                out << "...";
            out << '}';


        }
        break;

    case Array_t::TYPE: {
            const Array_t &array = Array(value);
            bool first = true;
            out << '(';
            if (level) {

                for (Array_t::const_iterator
                        i = array.begin();
                        i != array.end(); ++i) {
                    if (*i && first != true)
                        out << ", ";
                    else
                        first = false;

                    std::string str;
                    dumpFastrpcTree(**i, str, level - 1);
                    out << str;
                }
            } else
                out << "...";
            out << ')';



        }
        break;


    }
    // vytvori vystupny retezec
    outstr = out.str();

    return 0;
}


//for debug
void printSpaces(long spaces) {
    for (long i= spaces ; i != 0; i--)

        printf(" ");
}


/**
@brief method print fastrpc value to stdout in human readable format
@param value  fastrpc value
@param spaces from left border of screen (for recursive)
use for debug
*/
void printValue(const Value_t &value, long spaces ) {
    switch (value.getType()) {
    case Int_t::TYPE: {
            std::ostringstream os;
            os << Int(value).getValue();
            printf("%s\n",os.str().c_str());
        }
        break;

    case Bool_t::TYPE: {
            printf("%s\n",(Bool(value).getValue())? "TRUE":"FALSE");
        }
        break;

    case Double_t::TYPE: {
            printf("%f\n",Double(value).getValue());
        }
        break;

    case String_t::TYPE: {

            printf("%s\n",String(value).c_str());

        }
        break;

    case Binary_t::TYPE: {
            std::ostringstream os;
            os << "binary " << Binary(value).size() << " bytes \n";
            printf("%s\n",os.str().c_str());
        }
        break;

    case DateTime_t::TYPE: {
            const DateTime_t &dt = DateTime(value);
            printf("%04d%02d%02dT%02d:%02d:%02d%c%02d%02d\n",
                    dt.getYear(), dt.getMonth(), dt.getDay(),
                    dt.getHour(), dt.getMin(), dt.getSec(),
                    ((dt.getTimeZone() <= 0)? '+': '-'),
                    abs(dt.getTimeZone() / 60 / 60),
                    abs(dt.getTimeZone() * 60 % 60));

        }
        break;

    case Struct_t::TYPE: {
            const Struct_t &structVal = Struct(value);


            printf("{\n");
            spaces ++;
            for (Struct_t::const_iterator
                    i = structVal.begin();
                    i != structVal.end(); ++i) {
                //printf(" ");

                printSpaces(spaces);
                printf("%s:",i->first.c_str());

                printValue(*(i->second),spaces);
            }
            spaces --;
            printSpaces(spaces);
            printf("}\n");

        }
        break;

    case Array_t::TYPE: {
            const Array_t &array = Array(value);
            printf("(\n");
            spaces ++;
            for (Array_t::const_iterator
                    i = array.begin();
                    i != array.end(); ++i) {
                printSpaces(spaces);
                printValue(**(i),spaces);
            }
            spaces --;
            printSpaces(spaces);
            printf(")\n");

        }
        break;


    }
}
ProtocolVersion_t::ProtocolVersion_t(unsigned char versionMajor,
                                     unsigned char versionMinor)
        :versionMajor(versionMajor),versionMinor(versionMinor) {}

ProtocolVersion_t::ProtocolVersion_t()
        :versionMajor(FRPC_MAJOR_VERSION),versionMinor(FRPC_MINOR_VERSION) {}
/*ProtocolVersion_t& ProtocolVersion_t::operator=(const ProtocolVersion_t& other ) {
    this->versionMajor = other.versionMajor;
    this->versionMinor = other.versionMinor;
    return *this;
}*/
} // namespace FRPC

//some constants

namespace {
   FRPC::Pool_t pool;
}

const FRPC::Int_t &FRPC::Int_t::FRPC_ZERO = pool.Int(0);
const FRPC::Int_t &FRPC::Int_t::FRPC_MINUS_ONE = pool.Int(-1);
const FRPC::Double_t &FRPC::Double_t::FRPC_ZERO = pool.Double(0);
const FRPC::Bool_t &FRPC::Bool_t::FRPC_TRUE = pool.Bool(true);
const FRPC::Bool_t &FRPC::Bool_t::FRPC_FALSE = pool.Bool(false);
const FRPC::String_t &FRPC::String_t::FRPC_EMPTY = pool.String("");
const FRPC::Struct_t &FRPC::Struct_t::FRPC_EMPTY = pool.Struct();
const FRPC::Array_t &FRPC::Array_t::FRPC_EMPTY = pool.Array();
const FRPC::Binary_t &FRPC::Binary_t::FRPC_EMPTY = pool.Binary("");
const FRPC::DateTime_t &FRPC::DateTime_t::FRPC_EPOCH = pool.DateTime(0, 0);
const FRPC::DateTime_t &
    FRPC::DateTime_t::FRPC_NULL = pool.DateTime(0, 0, 0, 0, 0, 0, 0, -1, 0);

