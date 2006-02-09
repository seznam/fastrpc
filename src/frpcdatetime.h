/*
 * FILE          $Id: frpcdatetime.h,v 1.3 2006-02-09 16:00:26 vasek Exp $
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
#ifndef FRPCDATETIME_H
#define FRPCDATETIME_H

#include <frpcvalue.h>
#include <time.h>
#include <string>




namespace FRPC
{
class Pool_t;

/**
@brief DateTime type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT DateTime_t : public Value_t
{
    friend class Pool_t;
    
public:
    enum{ TYPE = 0x05 };
    /**
        @brief Default destructor
    */
    virtual ~DateTime_t();
    /**
        @brief Getting type of value
        @return  @b unsigned @b short always 
        @li @b DateTime_t::TYPE - identificator of datetime value 
    */
    virtual unsigned short getType() const
    {
        return TYPE;
    }
    /**
        @brief Getting typename of value
        @return @b const @b char* always
        @li @b "DateTime" - typename of DateTime_t
    */
    virtual const char* getTypeName() const
    {
        return "dateTime";
    }
    /**
        @brief Get localtime day number.
        @return Day (1-31). */
    short getDay() const;
    /**
        @brief Get localtime hours.
        @return Hour (0-23). 
    */
    short getHour() const;
    /**
        @brief Get localtime minutes.
        @return Minute (0-59).
    */
    short getMin() const;
    /**
        @brief Get localtime month number.
        @return Month (1-12).
    */
    short getMonth() const;
    /**
        @brief Get localtime seconds.
        @return Second (0-59).
    */
    short getSec() const;
    /**
        @brief Get localtime shift in sec relative to GMT (should be local timezone).

        Eastern timezones from GMT will report positive offset numbers,
        western timezones from GMT will report negative offsets.
        For example Central European Time (CET=GMT+0100) would have +3600.
        @return Localtime offset to GMT in secs.
    */
    short getTimeZone() const;
    /**
        @brief Get localtime year number.
        @return Year (1600-3647).
    */
    short getYear() const;
    /**
        @brief Get common unix time number (in UTC).
        @return Number of secs from 1970-01-01 00:00:00 UTC.
    */
    time_t getUnixTime() const;
    /**
        @brief  Get localtime day of week.
        @return Day of week (0=sunday, 1=monday, ..., 6=saturday).
    */
    short getDayOfWeek() const;
    /**
        @brief Method to clone/copy DateTime_t 
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    virtual Value_t& clone(Pool_t &newPool) const;
    /**
        @brief get iso format.
        @return iso format string
    */
    std::string isoFormat() const;
    
private:
    /**
        @brief Constructor  from now time 
    @   param pool  -  is a reference to Pool_t used for allocating
    */
    DateTime_t(Pool_t &pool);
    /**
        @brief Constructor.  Create specified date
        @param pool  -  is a reference to Pool_t used for allocating
        @param year - Year is offset (0 - 2047) to zero-year 1600 
        (0=1600, ... 370 = 1970, ...)
        @param month - Month is 1 - 12
        @param day -   Day is 1 - 31
        @param hour -  Hour is 0 - 23
        @param min -   Minute is 0 - 59
        @param sec -   Second is 0 - 59
        @param weekDay
        @param unixTime
        @param timeZone
    */
    DateTime_t(Pool_t &pool, short year, char month, char day,
                     char hour, char min, char sec, char weekDay, time_t unixTime, char timeZone);

    /**
        @brief Constructor from common unix time number.
        @param pool  -  is a reference to Pool_t used for allocating
        @param unixTime Number of secs from 1970-01-01 00:00:00 UTC.
    */
    DateTime_t(Pool_t &pool, time_t unixTime);
    
     /**
        @brief Constructor from common unix time structure.
        @param pool  -  is a reference to Pool_t used for allocating
        @param dateTime unix date time structure.
    */
    DateTime_t(Pool_t &pool, tm &dateTime);
     /**
        @brief Constructor from ISO format.
        @param pool  -  is a reference to Pool_t used for allocating
        @param isoFormat DateTime in iso format.
    */
    DateTime_t(Pool_t &pool, const std::string &isoFormat);
    
    
    short year;        ///year
    char month;        ///month
    char day;          ///day
    char hour;         ///hour
    char minute;          ///minute
    char sec;          /// second
    char weekDay;      ///day of week
    time_t        unixTime;     ///long unix time
    char          timeZone;     ///time zone  
};
/**
    @brief Inline method
    
    Used to retype Value_t to DateTime_t
    @return  If Value_t  can  retype to DateTime_t return reference to DateTime_t
    @n If Value_t can't retype to DateTime_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT DateTime_t& DateTime(Value_t &value)
{
    DateTime_t *dateTime = dynamic_cast<DateTime_t*>(&value);

    if(!dateTime)
        throw TypeError_t("Type is %s but not dateTime",value.getTypeName());
    
    return *dateTime;
}

/**
    @brief Inline method
    
    Used to retype Value_t to DateTime_t
    @return  If Value_t  can  retype to DateTime_t return reference to DateTime_t
    @n If Value_t can't retype to DateTime_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT const DateTime_t& DateTime(const Value_t &value)
{
    const DateTime_t *dateTime = dynamic_cast<const DateTime_t*>(&value);

    if(!dateTime)
        throw TypeError_t("Type is %s but not dateTime",value.getTypeName());
    
    return *dateTime;
}

};

#endif
