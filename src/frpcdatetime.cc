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
 * FILE          $Id: frpcdatetime.cc,v 1.9 2008-05-05 12:52:00 burlog Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *       
 */
#include <time.h>
#include <frpc.h>

#include "frpcdatetime.h"
#include "frpcpool.h"

namespace FRPC {

DateTime_t::~DateTime_t() {}

DateTime_t::DateTime_t(short year, char month, char day,
                       char hour, char minute, char sec, char weekDay,
                       time_t unixTime, int timeZone)
    : year(year), month(month), day(day), hour(hour), minute(minute),
      sec(sec), weekDay(weekDay), unixTime(unixTime), timeZone(timeZone)
{
    struct tm time_tm;
    memset(&time_tm, 0, sizeof(tm));
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon = month - 1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = minute;
    time_tm.tm_sec = sec;
    time_tm.tm_isdst = -1; // we know nothing about daylight savings time
    time_t time_l = mktime(&time_tm);
    struct tm *timeValid = localtime(&time_l);

    this->unixTime = time_l;
    this->weekDay = timeValid->tm_wday;
}

DateTime_t::DateTime_t(short year, char month, char day,
                       char hour, char min, char sec)
    : year(year), month(month), day(day), hour(hour), minute(minute), sec(sec)
{
    struct tm time_tm;
    memset(&time_tm, 0, sizeof(tm));
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon = month - 1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = minute;
    time_tm.tm_sec = sec;
    time_tm.tm_isdst = -1; // we know nothing about daylight savings time
    time_t time_l = mktime(&time_tm);
    struct tm *timeValid = localtime(&time_l);

    unixTime = time_l;
    weekDay = timeValid->tm_wday;
#ifdef HAVE_ALTZONE
    timeZone = (time_tm.tm_isdst > 0)? ::altzone: ::timezone;
#else
    timeZone = (time_tm.tm_isdst > 0)? ::timezone - 3600: ::timezone;
#endif
}

DateTime_t::DateTime_t(const time_t &unixTime)
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

#ifdef HAVE_ALTZONE
    timeZone = (time_tm->tm_isdst > 0)? ::altzone: ::timezone;
#else
    timeZone = (time_tm->tm_isdst > 0)? ::timezone - 3600: ::timezone;
#endif
}

DateTime_t::DateTime_t(time_t unixTime, int timeZone)
    : timeZone(timeZone)
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

DateTime_t::DateTime_t(const struct tm &tm)
    : timeZone(0)
{
    year = tm.tm_year + 1900;
    month = tm.tm_mon + 1;
    day = tm.tm_mday;
    hour = tm.tm_hour;
    minute = tm.tm_min;
    sec =  tm.tm_sec;
    weekDay = tm.tm_wday;
    this->unixTime = mktime(const_cast<struct tm *>(&tm));
}

DateTime_t::DateTime_t()
{
    time_t unix_time =  time(0);
    struct tm *time_tm = localtime(&unix_time);
    year = time_tm->tm_year + 1900;
    month = time_tm->tm_mon + 1;
    day = time_tm->tm_mday;
    hour =  time_tm->tm_hour;
    minute =  time_tm->tm_min;
    sec =  time_tm->tm_sec;
    weekDay =  time_tm->tm_wday;
    this->unixTime = unixTime;

#ifdef HAVE_ALTZONE
    timeZone = (time_tm->tm_isdst > 0)? ::altzone: ::timezone;
#else
    timeZone = (time_tm->tm_isdst > 0)? ::timezone - 3600: ::timezone;
#endif
}

DateTime_t::DateTime_t(const std::string &isoFormat)
    : timeZone(0)
{
    parseISODateTime(isoFormat.data(), isoFormat.size(), year, month, day, hour,
                     minute, sec, timeZone);
    struct tm time_tm;
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon = month - 1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = minute;
    time_tm.tm_sec = sec;
    time_tm.tm_isdst = -1; // we know nothing about daylight savings time
    time_t time_l = mktime(&time_tm);
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

int DateTime_t::getTimeZone() const
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
    return getISODateTime(year, month, day, hour, minute, sec, timeZone);
}

Value_t& FRPC::DateTime_t::clone(Pool_t &newPool) const
{
    return newPool.DateTime(year, month, day, hour, minute, sec, weekDay,
                            unixTime, timeZone);
}

bool FRPC::DateTime_t::isSaveLightDay() const {
    return localtime(&unixTime)->tm_isdst;
}

}
