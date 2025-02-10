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
 * FILE          $Id: frpcdatetime.cc,v 1.12 2011-02-16 07:22:39 burlog Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#include <ctime>
#include <cstring>

#include "frpc.h"
#include "frpcdatetime.h"
#include "frpcpool.h"
#include "frpcconfig.h"

namespace FRPC {

DateTime_t::~DateTime_t() = default;

DateTime_t::DateTime_t(int16_t year, char month, char day,
                       char hour, char minute, char sec, char weekDay,
                       time_t unixTime, int timeZone)
    : year(year), month(month), day(day), hour(hour), minute(minute),
      sec(sec), weekDay(weekDay), unixTime(unixTime), timeZone(timeZone)
{
    if (LibConfig_t::getInstance()->getDatetimeValidationPolicy()) {
        struct tm time_tm = {};
        time_tm.tm_year = year - 1900;
        time_tm.tm_mon = month - 1;
        time_tm.tm_mday = day;
        time_tm.tm_hour = hour;
        time_tm.tm_min = minute;
        time_tm.tm_sec = sec;
        time_tm.tm_isdst = -1; // we know nothing about daylight savings time
        time_t time_l = mktime(&time_tm);
        struct tm timeValid = {};
        localtime_r(&time_l, &timeValid);

        this->unixTime = time_l;
        this->weekDay = static_cast<char>(timeValid.tm_wday);
    }
}

DateTime_t::DateTime_t(short year, char month, char day,
                       char hour, char min, char sec)
    : year(year), month(month), day(day), hour(hour), minute(min), sec(sec)
{
    struct tm time_tm = {};
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon = month - 1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = minute;
    time_tm.tm_sec = sec;
    time_tm.tm_isdst = -1; // we know nothing about daylight savings time
    time_t time_l = mktime(&time_tm);
    struct tm timeValid = {};
    localtime_r(&time_l, &timeValid);

    unixTime = time_l;
    weekDay = static_cast<char>(timeValid.tm_wday);
#ifdef HAVE_ALTZONE
    timeZone = (time_tm.tm_isdst > 0)? ::altzone: ::timezone;
#else
    timeZone = static_cast<int32_t>((time_tm.tm_isdst > 0)? ::timezone - 3600: ::timezone);
#endif
}

DateTime_t::DateTime_t(const time_t &unixTime)
{
    struct tm time_tm = {};
    localtime_r(&unixTime, &time_tm);
    year = static_cast<int16_t>(time_tm.tm_year + 1900);
    month = static_cast<char>(time_tm.tm_mon + 1);
    day = static_cast<char>(time_tm.tm_mday);
    hour =  static_cast<char>(time_tm.tm_hour);
    minute =  static_cast<char>(time_tm.tm_min);
    sec =  static_cast<char>(time_tm.tm_sec);
    weekDay =  static_cast<char>(time_tm.tm_wday);
    this->unixTime = unixTime;

#ifdef HAVE_ALTZONE
    timeZone = (time_tm.tm_isdst > 0)? ::altzone: ::timezone;
#else
    timeZone = static_cast<int32_t>((time_tm.tm_isdst > 0)? ::timezone - 3600: ::timezone);
#endif
}

DateTime_t::DateTime_t(time_t unixTime, int timeZone)
    : unixTime(unixTime), timeZone(timeZone)
{
    struct tm time_tm = {};
    localtime_r(&unixTime, &time_tm);
    year = static_cast<int16_t>(time_tm.tm_year + 1900);
    month = static_cast<char>(time_tm.tm_mon + 1);
    day = static_cast<char>(time_tm.tm_mday);
    hour =  static_cast<char>(time_tm.tm_hour);
    minute =  static_cast<char>(time_tm.tm_min);
    sec =  static_cast<char>(time_tm.tm_sec);
    weekDay =  static_cast<char>(time_tm.tm_wday);
}

DateTime_t::DateTime_t(const struct tm &tm)
    : year(static_cast<int16_t>(tm.tm_year + 1900)),
      month(static_cast<char>(tm.tm_mon + 1)),
      day(static_cast<char>(tm.tm_mday)),
      hour(static_cast<char>(tm.tm_hour)),
      minute(static_cast<char>(tm.tm_min)),
      sec(static_cast<char>(tm.tm_sec)),
      weekDay(static_cast<char>(tm.tm_wday)),
      unixTime(mktime(const_cast<struct tm *>(&tm))),
      timeZone(0)
{}

DateTime_t::DateTime_t() {
    time_t unix_time =  time(nullptr);
    struct tm time_tm = {};
    localtime_r(&unix_time, &time_tm);
    year = static_cast<int16_t>(time_tm.tm_year + 1900);
    month = static_cast<char>(time_tm.tm_mon + 1);
    day = static_cast<char>(time_tm.tm_mday);
    hour =  static_cast<char>(time_tm.tm_hour);
    minute =  static_cast<char>(time_tm.tm_min);
    sec =  static_cast<char>(time_tm.tm_sec);
    weekDay =  static_cast<char>(time_tm.tm_wday);
    this->unixTime = mktime(&time_tm);;

#ifdef HAVE_ALTZONE
    timeZone = (time_tm.tm_isdst > 0)? ::altzone: ::timezone;
#else
    timeZone = static_cast<int32_t>((time_tm.tm_isdst > 0)? ::timezone - 3600: ::timezone);
#endif
}

DateTime_t::DateTime_t(const std::string &isoFormat)
    : timeZone(0)
{
    parseISODateTime(isoFormat.data(), isoFormat.size(), year, month, day, hour,
                     minute, sec, timeZone);
    struct tm time_tm = {};
    time_tm.tm_year = year - 1900;
    time_tm.tm_mon = month - 1;
    time_tm.tm_mday = day;
    time_tm.tm_hour = hour;
    time_tm.tm_min = minute;
    time_tm.tm_sec = sec;
    time_tm.tm_isdst = -1; // we know nothing about daylight savings time
    time_t time_l = mktime(&time_tm);
    struct tm timeValid = {};
    localtime_r(&time_l, &timeValid);

    this->unixTime =  time_l;
    this->weekDay = static_cast<char>(timeValid.tm_wday);
}

DateTime_t::DateTime_t(short year, char month, char day,
                       char hour, char minute, char sec,
                       time_t unixTime)
    : year(year), month(month), day(day), hour(hour), minute(minute),
      sec(sec), weekDay(), unixTime(unixTime), timeZone()
{}

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
    struct tm timeValid;
    localtime_r(&unixTime, &timeValid);
    return timeValid.tm_isdst;
}

} // namespace FRPC
