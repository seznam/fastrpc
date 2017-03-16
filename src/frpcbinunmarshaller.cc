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
 * FILE          $Id: frpcbinunmarshaller.cc,v 1.11 2010-06-10 15:21:04 mirecta Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *              Jan Klesnil <jan.klesnil@firma.seznam.cz>
 *
 * HISTORY
 *
 */
#include "frpcbinunmarshaller.h"
#include "frpcinternals.h"
#include <frpcstreamerror.h>
#include "frpctreebuilder.h"
#include <memory.h>

#define FRPC_GET_DATA_TYPE_INFO( data ) ((data) & 0x07 )

#ifdef _DEBUG
#define debugf(...) printf(__VA_ARGS__)
#else
static void noop() {}
#define debugf(...) noop()
#endif

namespace FRPC {

// 1 gig elements max. hard limit
static const size_t ELEMENT_SIZE_LIMIT = 1 << 30;

// unmarshaller state machine states
enum{S_MAGIC = 0, S_BODY, S_METHOD_NAME, S_METHOD_NAME_LEN,
     S_METHOD_CALL, S_METHOD_RESPONSE, S_FAULT,
     S_INT, S_BOOL, S_DOUBLE, S_STRING, S_DATETIME, S_BINARY, S_INTP8, S_INTN8,
     S_STRUCT, S_ARRAY, S_NULLTYPE,
     S_MEMBER_NAME, S_VALUE_TYPE, S_REAL_VALUE_TYPE, S_STRING_LEN, S_BINARY_LEN};

/** Decodes zigzag encoded integer back into native integer.
 */
static int64_t zigzagDecode(int64_t s) {
    uint64_t n = static_cast<uint64_t>(s);
    return static_cast<int64_t>((n >> 1) ^ (-(s & 1)));
}

static uint8_t getValueType(uint8_t data) {
    return ((data) >> 3);
}

/** Read value from type tag and check its value according to version */
static uint8_t getVersionedLengthSize(bool longer, uint8_t data) {
    if (longer) return (data & 0x7) + 1;
    uint8_t val = (data & 0x7);
    if (val == 0 || val > 4) {
        throw StreamError_t("Illegal element length");
    }
    return val;
}

static int64_t getInt64(const char *data, size_t size) {
    union {
        char tmp[8];
        int64_t number;
    };
    memset(tmp, 0, 8);
    memcpy(tmp, data, size);

#ifdef FRPC_BIG_ENDIAN
    //swap it
    SWAP_BYTE(tmp[7],tmp[0]);
    SWAP_BYTE(tmp[6],tmp[1]);
    SWAP_BYTE(tmp[5],tmp[2]);
    SWAP_BYTE(tmp[4],tmp[3]);
#endif

    return number;
}

static int32_t getInt32(const char *data, size_t size) {
    union {
        char tmp[4];
        int32_t number;
    };
    memset(tmp, 0, 4);
    memcpy(tmp, data, size);

#ifdef FRPC_BIG_ENDIAN
    //swap it
    SWAP_BYTE(tmp[3],tmp[0]);
    SWAP_BYTE(tmp[2],tmp[1]);
#endif

    return number;
}


// needs 8 bytes of data
static double getDouble(const char *data) {
#ifdef FRPC_BIG_ENDIAN
    char tmp[8] = {
        data[7], data[6], data[5], data[4], data[3], data[2], data[1], data[0]
    };

    return *reinterpret_cast<double*>(tmp);
#else
    return *reinterpret_cast<const double*>(data);
#endif
}

// needs 10 bytes of data
static DateTimeInternal_t getDateTime(const char *data) {
    DateTimeInternal_t dateTime;
    dateTime.year  = (data[9] << 3) | ((data[8] & 0xe0) >> 5);
    dateTime.month = (data[8] & 0x1e) >> 1;
    dateTime.day = ((data[8] & 0x01) << 4) |(((data[7] & 0xf0) >> 4));
    dateTime.hour = ((data[7] & 0x0f) << 1) | ((data[6] & 0x80) >> 7);
    dateTime.minute = ((data[6] & 0x7e) >> 1);
    dateTime.sec = ((data[6] & 0x01) << 5) | ((data[5] & 0xf8) >> 3);
    dateTime.weekDay = (data[5] & 0x07);
    dateTime.unixTime = getInt32(&data[1], 4);
    dateTime.timeZone = data[0];
    return dateTime;
}

// needs 14 bytes of data
static DateTimeInternal_t getDateTimeV3(const char *data) {
    DateTimeInternal_t dateTime;
    dateTime.year  = (data[13] << 3) | ((data[12] & 0xe0) >> 5);
    dateTime.month = (data[12] & 0x1e) >> 1;
    dateTime.day = ((data[12] & 0x01) << 4) |(((data[11] & 0xf0) >> 4));
    dateTime.hour = ((data[11] & 0x0f) << 1) | ((data[10] & 0x80) >> 7);
    dateTime.minute = ((data[10] & 0x7e) >> 1);
    dateTime.sec = ((data[10] & 0x01) << 5) | ((data[9] & 0xf8) >> 3);
    dateTime.weekDay = (data[9] & 0x07);
    int64_t time64 = 0;
    time64 = getInt64(&data[1], 8);
    dateTime.unixTime = time64;

    if (sizeof(time_t) < sizeof(time64)) {
        if (dateTime.unixTime != time64) {
            throw StreamError_t(
                        "time_t can't hold the received timestamp value");
        }
    }

    dateTime.timeZone = data[0];
    return dateTime;
}

class BinUnMarshaller_t::Driver_t {
public:
    Driver_t(BinUnMarshaller_t &self,
             const char *data,
             unsigned int size,
             bool stopOnUnknown = false)
        : self(self),
          input(data),
          inputSize(size),
          newDataWanted(0),
          finalizeValue(false),
          state(self.state),
          stopOnUnknown(stopOnUnknown)
    {
        if (!self.buffer.empty()) {
            size_t remains = self.dataWanted - self.buffer.size();
            uint64_t toRead = std::min<uint64_t>(remains, inputSize);
            self.buffer.append(&input[0], toRead);
            inputSize -= toRead;
            input += toRead;
        }
    }

    bool hasWantedData() const {
        return self.dataWanted <= self.buffer.size() + inputSize;
    }

    void update() {
        // empty buffer
        if (!self.buffer.empty()) {
            self.buffer.clear();
            self.buffer.reserve();   // TODO is it good?
        } else {
            input += self.dataWanted;
            inputSize -= self.dataWanted;
        }
        self.dataWanted = newDataWanted;
        newDataWanted = 0;
    }

    void finish() {
        if (inputSize > 0) {
            self.buffer.reserve(self.dataWanted);
            self.buffer.append(input, inputSize);
        }
    }

    uint64_t wanted() const { return self.dataWanted; }

    const char *data() const {
        if (self.buffer.empty())
            return input;
        else
            return self.buffer.data();
    }

    const char & operator [] (const unsigned int index) const {
        if (self.buffer.empty())
            return input[index];
        else
            return self.buffer[index];
    }

    void pushEntity(uint8_t type, uint32_t size) {
        BinUnMarshaller_t::StackElement_t e = {size, type};
        self.recursionStack.push_back(e);
    }

    bool isInsideStruct() const {
        return !self.recursionStack.empty()
                && self.recursionStack.back().type == STRUCT;
    }

    void decrementMember() {
        if (!finalizeValue) return;
        finalizeValue = false;

        while (!self.recursionStack.empty()) {
            self.recursionStack.back().members -= 1;

            if (self.recursionStack.back().members != 0)
                return;

            //call builder to close entity
            switch (self.recursionStack.back().type) {
            case STRUCT:
                self.dataBuilder.closeStruct();
                break;

            case ARRAY:
                self.dataBuilder.closeArray();
                break;

            default:
                break;
            }
            self.recursionStack.pop_back();
        }
    }

    // accessors
    ProtocolVersion_t& version() { return self.protocolVersion; }
    DataBuilder_t* dataBuilder() const { return &self.dataBuilder; }
    uint8_t& faultState() const { return  self.faultState; }
    int64_t& errNo() const { return self.errNo; }
    size_t remains() const { return inputSize; }

protected:
    BinUnMarshaller_t &self;
    const char *input;
    unsigned int inputSize;
public:
    uint64_t newDataWanted;
    bool finalizeValue;
    uint8_t &state;
    bool stopOnUnknown;
};

class BinUnMarshaller_t::FaultBuilder_t : public DataBuilderWithNull_t {
public:
    FaultBuilder_t(BinUnMarshaller_t::Driver_t &driver)
        : driver(driver)
    {}

    virtual void buildMethodResponse() {
        throw StreamError_t("Invalid state: method response");
    }

    virtual void buildBinary(const char* data, unsigned int size) {
        throw StreamError_t("Fault cannot contain binary value");
    }

    virtual void buildBinary(const std::string &data) {
        throw StreamError_t("Fault cannot contain binary value");
    }

    virtual void buildBool(bool value) {
        throw StreamError_t("Fault cannot contain boolean");
    }

    virtual void buildDateTime(short year, char month, char day,char hour,
                               char minute, char sec, char weekDay,
                               time_t unixTime, int timeZone)
    {
        throw StreamError_t("Fault cannot contain datatime");
    }

    virtual void buildDouble(double value) {
        throw StreamError_t("Fault cannot contain double");
    }

    virtual void buildFault(int, const char*, unsigned int) {
        throw StreamError_t("Fault cannot contain another fault");
    }

    virtual void buildFault(int errNumber, const std::string &errMsg) {
        throw StreamError_t("Fault cannot contain another fault");
    }

    virtual void buildInt(Int_t::value_type value) {
        if (driver.faultState() != 1) {
            throw StreamError_t("Only first value of fault can be int");
        }
        driver.errNo() = value;
        driver.faultState() = 2;
    }

    virtual void buildMethodCall(const char* methodName, unsigned int size) {
        throw StreamError_t("Invalid state: method call");
    }

    virtual void buildMethodCall(const std::string &methodName) {
        throw StreamError_t("Invalid state: method call");
    }

    virtual void buildString(const char* data, unsigned int size) {
        if (driver.faultState() != 2) {
            throw StreamError_t("Only second value of fault can be string");
        }
        driver.dataBuilder()->buildFault(driver.errNo(), data, size);
        driver.faultState() = 3;
    }

    virtual void buildString(const std::string &data) {
        if (driver.faultState() != 2) {
            throw StreamError_t("Only second value of fault can be string");
        }
        driver.dataBuilder()->buildFault(driver.errNo(), data);
        driver.faultState() = 3;
    }

    virtual void buildStructMember(const char *memberName, unsigned int size) {
        throw StreamError_t("Fault cannot contain struct");
    }

    virtual void buildStructMember(const std::string &memberName) {
        throw StreamError_t("Fault cannot contain struct");
    }

    virtual void closeArray() {
        throw StreamError_t("Fault cannot contain array");
    }

    virtual void closeStruct() {
        throw StreamError_t("Fault cannot contain struct");
    }

    virtual void openArray(unsigned int numOfItems) {
        throw StreamError_t("Fault cannot contain array");
    }

    virtual void openStruct(unsigned int numOfMembers) {
        throw StreamError_t("Fault cannot contain struct");
    }

    virtual void buildNull() {
        throw StreamError_t("Fault cannot contain null");
    }

protected:
    BinUnMarshaller_t::Driver_t &driver;
};

static void unMarshallInternal(BinUnMarshaller_t::Driver_t &d, char reqType) {
    BinUnMarshaller_t::FaultBuilder_t faultBuilder(d);
    DataBuilder_t *dataBuilder = (d.faultState() == 0) ? d.dataBuilder()
                                                       : &faultBuilder;
    unsigned char magic[]={0xCA, 0x11};

    for (; d.hasWantedData(); d.decrementMember(), d.update()) {
        switch (d.state) {
        case S_MAGIC: {
            if (memcmp(d.data(), magic, 2) != 0) {
                throw StreamError_t("Bad magic !!!");
            }
            d.version().versionMajor = d[2];
            d.version().versionMinor = d[3];
            if (d.version().versionMajor > 3 || d.version().versionMajor < 1) {
                throw StreamError_t("Unsupported protocol version !!!");
            }

            d.newDataWanted = 1;
            d.state = S_BODY;
        }
        break;

        case S_BODY: {
            char msgType = getValueType(d[0]);

            // filter stream types not requested
            do {
                if (reqType == UnMarshaller_t::TYPE_ANY) break;
                if (msgType == reqType) break;
                if (msgType == FAULT &&
                    reqType == UnMarshaller_t::TYPE_METHOD_RESPONSE)
                    break;

                throw StreamError_t("Encountered unexpected stream message type");
            } while (false);

            // filter invalid stream values
            switch (msgType) {
            case FAULT:
                d.faultState() = 1;
                dataBuilder = &faultBuilder;
                break;
            case METHOD_RESPONSE:
                dataBuilder->buildMethodResponse();
                break;
            case METHOD_CALL: break;
            default:
                throw StreamError_t("Invalid stream message type");
            }

            d.newDataWanted = 1;
            d.state = (msgType == METHOD_CALL) ? S_METHOD_NAME_LEN : S_VALUE_TYPE;
        }
        break;
        case S_METHOD_NAME_LEN: {
            d.newDataWanted = static_cast<uint8_t>(d[0]);

            if (!d.newDataWanted)
                throw StreamError_t("Bad call name");

            d.state = S_METHOD_NAME;
        }
        break;
        case S_METHOD_NAME: {
            dataBuilder->buildMethodCall(d.data(), d.wanted());
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;
        case S_MEMBER_NAME: {
            dataBuilder->buildStructMember(d.data(), d.wanted());
            debugf( "struct member: %s \n",
                    std::string(d.data(), d.wanted()).c_str());

            d.newDataWanted = 1;
            d.state = S_REAL_VALUE_TYPE;
        }
        break;
        case S_VALUE_TYPE: {
            if (d.isInsideStruct()) {
                d.newDataWanted = static_cast<uint8_t>(d[0]);

                if (d.newDataWanted == 0)
                    throw StreamError_t("Struct member name length is zero");

                d.state = S_MEMBER_NAME;
                break;
            }
            // fall-through

        case S_REAL_VALUE_TYPE:
            switch (getValueType(d[0])) {
            case BOOL: {
                if (d[0] & 0x6) {
                    throw StreamError_t("Invalid bool value");
                }
                dataBuilder->buildBool(d[0] & 0x01);
                debugf("bool\n");
                d.finalizeValue = true;
                d.newDataWanted = 1;
                d.state = S_VALUE_TYPE;
            }
            break;
            case NULLTYPE: {
                if (d.version().versionMajor == 1) {
                    throw StreamError_t("Unknown value type");
                }
                TreeBuilder_t *treeBuilder(dynamic_cast<TreeBuilder_t*>(dataBuilder));
                if (treeBuilder) {
                    treeBuilder->buildNull();
                } else {
                    dynamic_cast<DataBuilderWithNull_t*>(dataBuilder)->buildNull();
                }
                d.finalizeValue = true;
                d.newDataWanted = 1;
                d.state = S_VALUE_TYPE;
            }
            break;
            case INT: {
                d.newDataWanted = getVersionedLengthSize(
                            d.version().versionMajor > 2, d[0]);
                debugf("int length: %lu\n", d.newDataWanted);
                d.state = S_INT;
            }
            break;
            case INTN8: {
                d.newDataWanted = FRPC_GET_DATA_TYPE_INFO(d[0]) + 1;
                debugf("negative int length: %lu\n", d.newDataWanted);
                d.state = S_INTN8;
            }
            break;
            case INTP8: {
                d.newDataWanted = FRPC_GET_DATA_TYPE_INFO(d[0]) + 1;
                debugf("positive int length: %lu\n", d.newDataWanted);
                d.state = S_INTP8;
            }
            break;
            case DOUBLE: {
                d.newDataWanted = 8;
                debugf("double size 8 \n");
                d.state = S_DOUBLE;
            }
            break;
            case DATETIME: {
                d.newDataWanted = (d.version().versionMajor > 2) ? 14 : 10;
                debugf("datetime size %lu\n", d.newDataWanted);
                d.state = S_DATETIME;
            }
            break;

            case STRING: {
                d.newDataWanted = getVersionedLengthSize(
                            d.version().versionMajor >= 2, d[0]);
                debugf("string length size: %lu\n", d.newDataWanted);
                d.state = S_STRING_LEN;
            }
            break;

            case BINARY: {
                d.newDataWanted = getVersionedLengthSize(
                            d.version().versionMajor >= 2, d[0]);
                debugf("binary size size: %lu\n", d.newDataWanted);
                d.state = S_BINARY_LEN;
            }
            break;
            case ARRAY: {
                d.newDataWanted = getVersionedLengthSize(
                            d.version().versionMajor >= 2, d[0]);

                debugf("array size size: %lu\n", d.newDataWanted);
                d.state = S_ARRAY;
            }
            break;
            case STRUCT: {
                d.newDataWanted = getVersionedLengthSize(
                            d.version().versionMajor >= 2, d[0]);

                debugf("struct size size: %lu\n", d.newDataWanted);
                d.state = S_STRUCT;
            }
            break;
            default:
                if (d.stopOnUnknown) {
                    return;
                }
                throw StreamError_t("Unknown value type");
                break;

            }
        }
        break;
        case S_STRING_LEN: {
            //unpack string len
            d.newDataWanted = getInt64(d.data(), d.wanted());

            if (d.newDataWanted >= ELEMENT_SIZE_LIMIT)
                throw StreamError_t("String entity too large");

            debugf("string  size: %lu\n", d.newDataWanted);
            d.state = S_STRING;
        }
        break;
        case S_STRING: {
            // report whole string
            dataBuilder->buildString(d.data(), d.wanted());
            d.finalizeValue = true;
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;
        case S_BINARY_LEN: {
            //unpack string len
            d.newDataWanted = getInt64(d.data(), d.wanted());

            if (d.newDataWanted >= ELEMENT_SIZE_LIMIT)
                throw StreamError_t("Binary entity too large");

            debugf( "binary size: %lu\n", d.newDataWanted);
            d.state = S_BINARY;
        }
        break;
        case S_BINARY: {
            // report whole data
            dataBuilder->buildBinary(d.data(), d.wanted());
            d.finalizeValue = true;
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;
        case S_INT: {
            int64_t number = getInt64(d.data(), d.wanted());
            if (d.version().versionMajor > 2) {
                number = zigzagDecode(number);
            }

            dataBuilder->buildInt(number);
            debugf( "int number: %li\n", number);
            d.finalizeValue = true;
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;

        case S_INTN8: {
            //unpack value
            dataBuilder->buildInt(-getInt64(d.data(), d.wanted()));
            d.finalizeValue = true;
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;

        case S_INTP8: {
            //unpack value
            dataBuilder->buildInt(getInt64(d.data(), d.wanted()));
            d.finalizeValue = true;
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;

        case S_DOUBLE: {
            dataBuilder->buildDouble(getDouble(d.data()));
            d.finalizeValue = true;
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;

        case S_DATETIME: {
            DateTimeInternal_t dateTime = (d.version().versionMajor > 2)
                    ? getDateTimeV3(d.data())
                    : getDateTime(d.data());

            if (dateTime.year || dateTime.month || dateTime.day
                    || dateTime.hour || dateTime.minute || dateTime.sec)
            {
                dateTime.year += 1600;
            }

            dataBuilder->buildDateTime(
                        dateTime.year, dateTime.month, dateTime.day,
                        dateTime.hour, dateTime.minute, dateTime.sec,
                        dateTime.weekDay, dateTime.unixTime,
                        dateTime.timeZone * 15 * 60);

            d.finalizeValue = true;
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;

        case S_STRUCT: {
            int64_t acc = getInt64(d.data(), d.wanted());

            // we don't accept negative sizes here
            if (acc < 0)
                throw StreamError_t("Struct entity invalid size");

            if (acc >> 32) {
                throw StreamError_t("Struct too large !!!");
            }

            if (static_cast<size_t>(acc) >= ELEMENT_SIZE_LIMIT)
                throw StreamError_t("Struct entity too large");

            dataBuilder->openStruct(acc);

            if (acc != 0) {
                d.pushEntity(STRUCT, acc);
            } else {
                dataBuilder->closeStruct();
                d.finalizeValue = true;
            }
            debugf( "struct size: %li \n", acc);
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;

        case S_ARRAY: {
            //unpack number
            int64_t acc = getInt64(d.data(), d.wanted());

            // we don't accept negative sizes here
            if (acc < 0)
                throw StreamError_t("Array entity invalid size");

            if (acc >> 32) {
                throw StreamError_t("Array too long !!!");
            }

            if (static_cast<size_t>(acc) >= ELEMENT_SIZE_LIMIT)
                    throw StreamError_t("Array entity too large");

            dataBuilder->openArray(acc);

            if (acc != 0) {
                d.pushEntity(ARRAY, acc);
            } else {
                dataBuilder->closeArray();
                d.finalizeValue = true;
            }
            debugf( "array size: %li\n", acc);
            d.newDataWanted = 1;
            d.state = S_VALUE_TYPE;
        }
        break;
        }
    }
}

BinUnMarshaller_t::~BinUnMarshaller_t() {}

void BinUnMarshaller_t::finish() {
    debugf("finish: state = %u, storage.size = %zu\n", state, recursionStack.size());
    if (state != S_VALUE_TYPE || recursionStack.size() > 0)
        throw StreamError_t("Stream not complete");
}

void BinUnMarshaller_t::unMarshall(
        const char *data, unsigned int size, char type)
{
    Driver_t driver(*this, data, size);
    try {
        unMarshallInternal(driver, type);
    }
    catch (...) {
        driver.finish();
        throw;
    }
    driver.finish();
}

size_t BinUnMarshaller_t::unMarshallKnown(
        const char *data, unsigned int size, char type)
{
    Driver_t driver(*this, data, size, true);
    try {
        unMarshallInternal(driver, type);
        if (driver.state == S_VALUE_TYPE) {
            return size - driver.remains();
        }
    }
    catch (...) {
        driver.finish();
        throw;
    }
    driver.finish();
    return size;
}

void BinUnMarshaller_t::resetToFaultState() {
    recursionStack.clear();
    buffer.clear();
    faultState = 1;
    dataWanted = 1;
    state = S_VALUE_TYPE;
}

}
