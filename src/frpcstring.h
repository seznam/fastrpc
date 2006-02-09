/*
 * FILE          $Id: frpcstring.h,v 1.2 2006-02-09 16:00:26 vasek Exp $
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
#ifndef FRPCSTRING_H
#define FRPCSTRING_H

#include <string>
#include <frpcvalue.h>


namespace FRPC
{
class Pool_t;

/**
@brief String type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT String_t : public Value_t
{
    friend class Pool_t;
public:
   enum{ TYPE = 0x04 };

    virtual ~String_t();
    /**
        @brief Getting type of value
        @return  @b unsigned @b short always 
        @li @b Binary_t::TYPE - identificator of binary value
    */
    virtual unsigned short getType() const
    {
        return TYPE;
    }
    /**
        @brief Getting typename of value
        @return @b const @b char* always
        @li @b "String" - typename of String_t
    */
    virtual const char* getTypeName() const
    {
        return "string";
    }
    /** 
        @brief Get data itself. Data are not "\0"-terminated.
        @return Pointer to the binary data. 
    */
    std::string::size_type size() const;
    /** 
        @brief Get data itself. Data are not "\0"-terminated.
        @return Pointer to the binary data. 
    */

    const std::string::value_type* data() const;
    /**
        @brief Get binary data as STL string.
        @return Binary data as string. 
    */
    
    std::string getString() const;
    /**
        @brief Get binary data as C string.
        @return Binary data as C string. 
    */
    const char* c_str() const;
    /**
        @brief Method to clone/copy Binary_t 
        @param newPool is reference of Pool_t which is used for allocate objects
    */
    virtual Value_t& clone(Pool_t &newPool) const;
    /**
        @brief operator const std::string
    */
    inline operator const std::string& () const
    {
        return value;
    }
   /**
        @brief operator const std::wstring
    */
    operator const std::wstring () const;
    /**
        @brief operator std::string
    */
    inline operator std::string& () 
    {
        return value;
    }
   /**
        @brief operator std::wstring
    */
    operator std::wstring ();


private:
    /**
        @brief Default constructor is disabled
    */
    String_t();
    /**
        @brief Constructor  from pointer to data and data size
        @param pool  -  is a reference to Pool_t used for allocating
        @param pData - is a unsigned char pointer to data 
        @param dataSize - is a size of data in bytes
    */
    String_t(Pool_t &pool, std::string::value_type *pData, std::string::size_type dataSize);
    /**
       @brief Constructor from std::string value
       @param pool  -  is a reference to Pool_t used for allocating
       @param value  - is a std::string of data
    */
    String_t(Pool_t &pool, const std::string &value);
    /**
       @brief Constructor from std::wstring value
       @param pool  -  is a reference to Pool_t used for allocating
       @param value  - is a std::wstring of data
    */
    String_t(Pool_t &pool, const std::wstring &value);

    std::string value;///internal storage
};
/**
    @brief Inline method
    
    Used to retype Value_t to String_t 
    @return  If Value_t  can  retype to String_t return reference to String_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t
    
*/
inline FRPC_DLLEXPORT String_t& String(Value_t &value)
{
    String_t *string_v = dynamic_cast<String_t*>(&value);

    if(!string_v)
        throw TypeError_t("Type is %s but not string",value.getTypeName());
    
    return *string_v;
}

/**
    @brief Inline method
    
    Used to retype Value_t to String_t 
    @return  If Value_t  can  retype to String_t return reference to String_t
    @n If Value_t can't retype to Int_t: throw exception TypeError_t
    
*/
inline FRPC_DLLEXPORT const String_t& String(const Value_t &value)
{
    const String_t *string_v = dynamic_cast<const String_t*>(&value);

    if(!string_v)
        throw TypeError_t("Type is %s but not string",value.getTypeName());
    
    return *string_v;
}


};

#endif
