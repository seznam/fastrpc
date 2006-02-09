/*
 * FILE          $Id: frpcdatetime.cc,v 1.3 2006-02-09 16:00:26 vasek Exp $
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
#include "frpcdatetime.h"
#include "frpcpool.h"
#include <time.h>
#include <frpc.h>

namespace FRPC
{



DateTime_t::~DateTime_t()
{}

DateTime_t::DateTime_t(Pool_t &pool, short year, char month, char day,
                       char hour, char minute, char sec, char weekDay, time_t unixTime, char timeZone)
        :Value_t(pool),year(year),month(month),day(day),hour(hour),minute(minute),sec(sec),weekDay(weekDay),
        unixTime(unixTime),timeZone(timeZone)
{
    struct tm time_tm;
    memset(&time_tm, 0, sizeof(tm));
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon = month - 1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = minute;
    time_tm.tm_sec = sec;
    long time_l = mktime(&time_tm);
    struct tm *timeValid = localtime(&time_l);

    this->unixTime =  time_l;
    this->weekDay = timeValid->tm_wday;

}



DateTime_t::DateTime_t(Pool_t &pool, time_t unixTime)
        :Value_t(pool)
{
    struct tm *time_tm = localtime(&unixTime);
    year = time_tm->tm_year + 1900;
    month = time_tm->tm_mon + 1;
    day = time_tm->tm_mday;
    hour =  time_tm->tm_hour;
    minute =  time_tm->tm_min;
    sec =  time_tm->tm_sec;
    weekDay =  time_tm->tm_wday;
    this->unixTime = unixTime;
}



DateTime_t::DateTime_t(Pool_t &pool, tm &dateTime)
        :Value_t(pool)
{
    /// @todo implement me
}

DateTime_t::DateTime_t(Pool_t &pool)
        :Value_t(pool)
{
    long unix_time =  time(0);
    struct tm *time_tm = localtime(&unix_time);
    year = time_tm->tm_year + 1900;
    month = time_tm->tm_mon + 1;
    day = time_tm->tm_mday;
    hour =  time_tm->tm_hour;
    minute =  time_tm->tm_min;
    sec =  time_tm->tm_sec;
    weekDay =  time_tm->tm_wday;
    this->unixTime = unixTime;
}

DateTime_t::DateTime_t(Pool_t &pool, const std::string &isoFormat)
        :Value_t(pool)
{
    parseISODateTime(isoFormat.data(),isoFormat.size(),year,month,day,hour,minute,sec,timeZone);
    struct tm time_tm;
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon = month - 1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = minute;
    time_tm.tm_sec = sec;
    long time_l = mktime(&time_tm);
    struct tm *timeValid = localtime(&time_l);

    this->unixTime =  time_l;
    this->weekDay = timeValid->tm_wday;
}


short DateTime_t::getDay() const
{
    return day;
}



short DateTime_t::getHour() const
{
    return hour;
}



short DateTime_t::getMin() const
{
    return minute;
}



short DateTime_t::getMonth() const
{
    return month;
}



short DateTime_t::getSec() const
{
    return sec;
}



short DateTime_t::getTimeZone() const
{
    return timeZone;
}



short DateTime_t::getYear() const
{
    return year;
}



time_t  DateTime_t::getUnixTime() const
{
    return unixTime;
}


short DateTime_t::getDayOfWeek() const
{
    return weekDay;
}

std::string DateTime_t::isoFormat() const
{
    return getISODateTime(year,month,day,hour,minute,sec,timeZone);
}

Value_t& FRPC::DateTime_t::clone(Pool_t &newPool) const
{
    return newPool.DateTime(year, month, day, hour, minute, sec, weekDay, unixTime,
                            timeZone);
}
}
