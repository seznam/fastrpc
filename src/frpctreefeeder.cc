/*
 * FILE          $Id: frpctreefeeder.cc,v 1.1 2005-07-19 13:02:54 vasek Exp $
 *
 * DESCRIPTION   
 *
 * AUTHOR        
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * Copyright (C) Seznam.cz a.s. 2002
 * All Rights Reserved
 *
 * HISTORY
 *       
 */
#include "frpctreefeeder.h"
//#include "frpcinternals.h"

namespace FRPC
{

void TreeFeeder_t::feedValue(Value_t &value)
{
    switch(value.getType())
    {
    case Int_t::TYPE:
        {
            marshaller.packInt(Int(value).getValue());
        }
        break;

    case Bool_t::TYPE:
        {
            marshaller.packBool(Bool(value).getValue());
        }
        break;

    case Double_t::TYPE:
        {
            marshaller.packDouble(Double(value).getValue());
        }
        break;

    case String_t::TYPE:
        {
            String_t &str = String(value);

            marshaller.packString(str.data(), str.size());
        }
        break;

    case Binary_t::TYPE:
        {
            Binary_t &bin = Binary(value);

            marshaller.packBinary(bin.data(), bin.size());
        }
        break;

    case DateTime_t::TYPE:
        {
            DateTime_t &dt = DateTime(value);

            marshaller.packDateTime(dt.getYear(), dt.getMonth(), dt.getDay(),
                                    dt.getHour(), dt.getMin(), dt.getSec(),
                                    dt.getDayOfWeek(), dt.getUnixTime(), dt.getTimeZone());

        }
        break;

    case Struct_t::TYPE:
        {
            Struct_t &structVal = Struct(value);


            marshaller.packStruct(structVal.size());

            for(Struct_t::iterator i = structVal.begin(); i != structVal.end(); ++i)
            {
                marshaller.packStructMember(i->first.data(), i->first.size());

                feedValue(*(i->second));
            }

        }
        break;

    case Array_t::TYPE:
        {
            Array_t &array = Array(value);
            marshaller.packArray(array.size());
            for(Array_t::iterator i = array.begin(); i != array.end(); ++i)
            {
                feedValue(**i);
            }

        }
        break;


    }

}


TreeFeeder_t::~TreeFeeder_t()
{}


}
;
