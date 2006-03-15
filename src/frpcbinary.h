/*
 * FILE          $Id: frpcbinary.h,v 1.3 2006-03-15 08:54:12 vasek Exp $
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
#ifndef FRPCBINARY_H
#define FRPCBINARY_H

#include <string>
#include <frpcvalue.h>


namespace FRPC
{
class Pool_t;

/**
@brief Binary type
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT  Binary_t : public Value_t
{
    friend class Pool_t;
public:
    enum{ TYPE = 0x06 };

    virtual ~Binary_t();

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
        @li @b "Binary" - typename of Binary_t
    */
    virtual const char* getTypeName() const
    {
        return "binary";
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

private:
    /**
        @brief Default constructor is disabled
    */
    Binary_t();
    /**
        @brief Constructor  from pointer to data and data size
        @param pool  -  is a reference to Pool_t used for allocating
        @param pData - is a unsigned char pointer to data 
        @param dataSize - is a size of data in bytes
    */
    Binary_t(Pool_t &pool, std::string::value_type *pData, std::string::size_type dataSize);
    /**
       @brief Constructor from std::string value
       @param pool  -  is a reference to Pool_t used for allocating
       @param value  - is a std::string of data
    */
    Binary_t(Pool_t &pool, const std::string &value);

    std::string value;///internal storage

};
/**
    @brief Inline method
    
    Used to retype Value_t to Binary_t 
    @return  If Value_t  can  retype to Binary_t return reference to Binary_t
    @n If Value_t can't retype to Binary_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT Binary_t& Binary(Value_t &value)
{
    Binary_t *binary = dynamic_cast<Binary_t*>(&value);

    if(!binary)
        throw TypeError_t("Type is %s but not binary",value.getTypeName());
    return *binary;
}

/**
    @brief Inline method
    
    Used to retype Value_t to Binary_t 
    @return  If Value_t  can  retype to Binary_t return reference to Binary_t
    @n If Value_t can't retype to Binary_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT const Binary_t& Binary(const Value_t &value)
{
    const Binary_t *binary = dynamic_cast<const Binary_t*>(&value);

    if(!binary)
        throw TypeError_t("Type is %s but not binary",value.getTypeName());
    return *binary;
}

}

#endif
