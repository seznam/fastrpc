/*
 * FILE          $Id: frpcarray.h,v 1.2 2006-02-09 16:00:26 vasek Exp $
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
#ifndef FRPCFRPCARRAY_H
#define FRPCFRPCARRAY_H

#include <frpcvalue.h>
#include <vector>

namespace FRPC
{
class Pool_t;
/**
@brief Array type can storage any type of Value_t
@author Miroslav Talasek
*/
class FRPC_DLLEXPORT Array_t : public Value_t
{
    friend class Pool_t;
public:

    /**
       @brief Array_t iterator
    */
    typedef std::vector<Value_t*>::iterator iterator;
    /**
       @brief Array_t const_iterator
    */
    typedef std::vector<Value_t*>::const_iterator const_iterator;
    /**
      @brief Array_t size_type
    */
    typedef std::vector<Value_t*>::size_type size_type;

    enum{TYPE = 0x0B};
    /**
        @brief  Default destructor
    */
    virtual ~Array_t();
    /**
       @brief Method to clone/copy Array_t 
       @param newPool is reference of Pool_t which is used for allocate objects
     * @return reference to new Array_t as Value_t
    */

    virtual Value_t& clone(Pool_t &newPool) const;
    /**
        @brief Getting type of value
        @return  @b unsigned @b short always 
        @li @b Array_t::TYPE - identificator of array value
    */
    virtual unsigned short getType() const
    {
        return TYPE;
    }
    /**
        @brief Getting typename of value
        @return @b const @b char * always
        @li @b "Array" - typename of Array_t
    */
    virtual const char* getTypeName() const
    {
        return "array";
    }

    /**
        @brief operator []
        @return reference to Value_t or exception IndexError_t if index is out of range
    */
    Value_t& operator[] (size_type index);

    /**
        @brief operator []
        @return reference to Value_t or exception IndexError_t if index is out of range
    */
    const Value_t& operator[] (size_type index) const;

    /**
        @brief getting iterator to first item
        @return Array_t::iterator - position to first item
    */
    iterator begin();
    /**
        @brief getting iterator to last item
        @return Array_t::iterator - position to last item
    */
    iterator end();

    /**
        @brief getting iterator to first item
        @return Array_t::iterator - position to first item
    */
    const_iterator begin() const;
    /**
        @brief getting iterator to last item
        @return Array_t::iterator - position to last item
    */
    const_iterator end() const;

    /**
        @brief getting number of items in Array_t
        @return  Array_t::size_type -  number of items in Array_t
    */
    size_type size() const;
    /**
        @brief checking if Array_t is empty
        @return bool
        @li @b TRUE if the Array_t is empty @n
        @li @b FALSE if the Array_t isn't empty
        
    */
    bool empty() const;
    /**
        @brief delete all items in Array_t
    */
    void clear();
    /**
        @brief append Value_t to end of Array_t
        @param value is new Value_t 
    */
    void push_back(Value_t &value);
    /**
        @brief append Value_t to end of Array_t
        @param value is new Value_t
        @return Array_t& reference with apended value 
    */
    Array_t& append(Value_t &value);
    /**
        @brief check array for items
        @param items is std::string contains  signatures as
        @li @b  s - string
        @li @b  i - int
        @li @b  b - bool
        @li @b  d - double
        @li @b  B - binary
        @li @b  D - dateTime
        @li @b  A - array
        @li @b  S - struct  
        
        If items not correct function throw TypeError_t.
      @n This method is using for checking input parameters in methods.
    */  
    void checkItems(const std::string &items) const;
    

private:
    /**
    @brief Default constructor is disabled
    */
    Array_t();
    /**
        @brief Costructor empty Array_t 
        @param pool is a reference to Pool_t used for allocating
    */
    Array_t(Pool_t &pool);
    /**
        @brief Costructor Array_t with one Value_t item
        @param pool is a reference to Pool_t used for allocating
        @param item  is a new item
    */
    Array_t(Pool_t &pool, Value_t &item );


    std::vector<Value_t*> arrayData;///Internal array data

};
/** 
    @brief Inline method
    
    Used to retype Value_t to Array_t
    @param value is reference to Value_t 
    @return  If Value_t  can  retype to Array_t return reference to Array_t
    @n If Value_t can't retype to Array_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT Array_t& Array(Value_t &value)
{
    Array_t *array = dynamic_cast<Array_t*>(&value);

    if(!array)
        throw TypeError_t("Type is %s but not array",value.getTypeName());
    return *array;
}

/** 
    @brief Inline method
    
    Used to retype Value_t to Array_t
    @param value is reference to Value_t 
    @return  If Value_t  can  retype to Array_t return reference to Array_t
    @n If Value_t can't retype to Array_t throw exception TypeError_t
*/
inline FRPC_DLLEXPORT const Array_t& Array(const Value_t &value)
{
    const Array_t *array = dynamic_cast<const Array_t*>(&value);

    if(!array)
        throw TypeError_t("Type is %s but not array",value.getTypeName());
    return *array;
}

};

#endif
