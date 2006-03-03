/*
 * FILE          $Id: frpc.cc,v 1.4 2006-03-03 11:07:14 mirecta Exp $
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


#include <frpc.h>
#include <frpcinternals.h>

using namespace FRPC;

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
                           char minute, char sec, char timeZone)
{
    char dateTime[50];


    if(timeZone > 0)
    {
        sprintf(dateTime,"%04d%02d%02dT%02d:%02d:%02d+%02d%02d",year
                ,month ,
                day ,hour,minute,sec,
                (timeZone*15)/60,(timeZone*15)%60);
    }
    else
    {
        sprintf(dateTime,"%04d%02d%02dT%02d:%02d:%02d-%02d%02d",year
                ,month ,
                day, hour ,minute ,sec,
                abs((timeZone*15))/60,abs((timeZone*15)%60));
    }

    return std::string(dateTime);
}

/**
@brief method parse date time iso format from string data and his length
and fill parameters
*/

void parseISODateTime(const char *data, long len, short &year, char &month,
                      char &day, char &hour,
                      char &minute, char &sec, char &timeZone)
{

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
    while (sit != end && i < 4 && isdigit(*sit))
    {
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
    while (sit != end && i < 2 && isdigit(*sit))
    {
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
    while (sit != end && i < 2 && isdigit(*sit))
    {
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
    while (sit != end && i < 2 && isdigit(*sit))
    {
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
    while (sit != end && i < 2 && isdigit(*sit))
    {
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
    while (sit != end && i < 2 && isdigit(*sit))
    {
        tmp += *sit;
        ++sit;
        ++i;
    }
    if (tmp.length() == 0)
        return;
    //throw StreamError_t("Bad DATE format");
    sec = atoi(tmp.c_str());

    // if sec fraction
    if (sit != end && *sit == '.')
    {
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
    if (sit != end)
    {
        tzsign = *sit;
        ++sit;
    }
    // if timezone specified
    if (tzsign == 'Z' || tzsign == 'z')
    {
        // Z means zero meridian GMT+0
        tzhour = 0;
        tzmin = 0;
    }
    else if (tzsign == '+' || tzsign == '-')
    {
        // get TZ hour
        tmp.erase();
        i = 0;
        while (sit != end && i < 2 && isdigit(*sit))
        {
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
        while (sit != end && i < 2 && isdigit(*sit))
        {
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

    int sign = (tzsign == '+')?1:-1;

    timeZone = (tzhour * 60 + tzmin)/15 * sign;

}

/**
@brief method render fastrpc Value_t to string with level level
@param value1  - fast rpc value
@param outstr - out string 
@param level - level of render
@return always zero 
*/
int dumpFastrpcTree(const Value_t &value1,
                    std::string &outstr, int level)
{

    std::ostrstream out;
    Value_t &value = const_cast<Value_t&>(value1);

    switch(value.getType())
    {
    case Int_t::TYPE:
        {
            out << Int(value).getValue();

        }
        break;

    case Bool_t::TYPE:
        {
            out << (Bool(value).getValue() ? "true" : "false");
        }
        break;

    case Double_t::TYPE:
        {
            out << Double(value).getValue();
        }
        break;

    case String_t::TYPE:
        {

            out << '"';
            if (String(value).size() > MAX_LEN)
                out << String(value).getString().substr(0,MAX_LEN) << "...";
            else
                out << String(value).getString();
            out << '"';


        }
        break;

    case Binary_t::TYPE:
        {

            out << "b\"";
            if (Binary(value).size() > MAX_LEN)
                out << Binary(value).getString().substr(0,MAX_LEN) << "...";
            else
                out << Binary(value).getString();
            out << '"';

        }
        break;

    case DateTime_t::TYPE:
        {
            DateTime_t &dt = DateTime(value);
            char buff[50];
            if(dt.getTimeZone() > 0)
                sprintf(buff,"%04d%02d%02dT%02d:%02d:%02d+%02d%02d",dt.getYear(),
                        dt.getMonth(), dt.getDay(),
                        dt.getHour(), dt.getMin(), dt.getSec(),
                        (dt.getTimeZone()*15)/60,
                        (dt.getTimeZone()*15)%60);
            else
                sprintf(buff,"%04d%02d%02dT%02d:%02d:%02d-%02d%02d",dt.getYear(),
                        dt.getMonth(), dt.getDay(),
                        dt.getHour(), dt.getMin(), dt.getSec(),
                        abs((dt.getTimeZone()*15)/60),
                        abs((dt.getTimeZone()*15)%60));
            out << buff;

        }
        break;

    case Struct_t::TYPE:
        {
            Struct_t &structVal = Struct(value);
            bool first = true;
            out << '{';

            if (level)
            {
                for(Struct_t::iterator i = structVal.begin(); i != structVal.end(); ++i)
                {

                    if (i->second && first != true)
                        out << ", ";
                    else
                        first = false;

                    std::string val_str;
                    dumpFastrpcTree(*(i->second), val_str, level - 1);
                    out << i->first << ": " << val_str;

                }
            }
            else
                out << "...";
            out << '}';


        }
        break;

    case Array_t::TYPE:
        {
            Array_t &array = Array(value);
            bool first = true;
            out << '(';
            if (level)
            {

                for(Array_t::iterator i = array.begin(); i != array.end(); ++i)
                {
                    if (*i && first != true)
                        out << ", ";
                    else
                        first = false;
                    
                    std::string str;
                    dumpFastrpcTree(**i, str, level - 1);
                    out << str;
                }
            }
            else
                out << "...";
            out << ')';



        }
        break;


    }
    // ukonci string stream
    out << '\0';
    // vytvo�e vstupn�et�ec
    outstr = out.str();
    out.freeze(false);
    return 0;
}


//for debug
void printSpaces(long spaces)
{
    for(long i= spaces ; i != 0; i--)

        printf(" ");
}


/**
@brief method print fastrpc value to stdout in human readable format
@param value  fastrpc value 
@param spaces from left border of screen (for recursive)
use for debug 
*/
void printValue(Value_t &value, long spaces )
{
    switch(value.getType())
    {
    case Int_t::TYPE:
        {
            printf("%ld\n",Int(value).getValue());
        }
        break;

    case Bool_t::TYPE:
        {
            printf("%s\n",(Bool(value).getValue())? "TRUE":"FALSE");
        }
        break;

    case Double_t::TYPE:
        {
            printf("%f\n",Double(value).getValue());
        }
        break;

    case String_t::TYPE:
        {

            printf("%s\n",String(value).c_str());

        }
        break;

    case Binary_t::TYPE:
        {

            printf("binary %d bytes\n",Binary(value).size());

        }
        break;

    case DateTime_t::TYPE:
        {
            DateTime_t &dt = DateTime(value);
            if(dt.getTimeZone() > 0)
                printf("%04d%02d%02dT%02d:%02d:%02d+%02d%02d\n",dt.getYear(),
                       dt.getMonth(), dt.getDay(),
                       dt.getHour(), dt.getMin(), dt.getSec(),
                       (dt.getTimeZone()*15)/60,
                       (dt.getTimeZone()*15)%60);
            else
                printf("%04d%02d%02dT%02d:%02d:%02d-%02d%02d\n",dt.getYear(),
                       dt.getMonth(), dt.getDay(),
                       dt.getHour(), dt.getMin(), dt.getSec(),
                       abs((dt.getTimeZone()*15)/60),
                       abs((dt.getTimeZone()*15)%60));

        }
        break;

    case Struct_t::TYPE:
        {
            Struct_t &structVal = Struct(value);


            printf("{\n");
            spaces ++;
            for(Struct_t::iterator i = structVal.begin(); i != structVal.end(); ++i)
            {
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

    case Array_t::TYPE:
        {
            Array_t &array = Array(value);
            printf("(\n");
            spaces ++;
            for(Array_t::iterator i = array.begin(); i != array.end(); ++i)
            {
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

} // namespace FRPC
