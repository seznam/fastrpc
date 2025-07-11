#include <string>
#include <limits>
#include <iostream>
#include <cstdlib>

#include "frpc.h"
#include "frpcwriter.h"
#include "frpcvalue.h"
#include "frpcarray.h"
#include "frpcdatetime.h"
#include "frpcpool.h"
#include "frpcint.h"
#include "frpcbinmarshaller.h"
#include "frpcbinunmarshaller.h"
#include "frpctreefeeder.h"
#include "frpctreebuilder.h"

size_t tests = 0;
size_t fails = 0;

bool expect(bool condition, const char *mark, const char *file, int line) {
    ++tests;
    if (!condition) {
        fails++;
        std::cerr << file << ":" << line
                  << ":1: error: FAILED TEST: " << mark << std::endl;
        return false;
    }

    return true;
}

#define TEST(condition) expect(condition, ""#condition"", __FILE__, __LINE__)

class StringWriter_t : public FRPC::Writer_t {
public:
    virtual void write(const char *data, unsigned int size ) {
        target.append(data, size);
    }

    virtual void flush() {
    }


    std::string target;
};

FRPC::Value_t &makeTestValue(FRPC::Pool_t &pool) {
    FRPC::Array_t &arr = pool.Array();
    arr.append(
            pool.DateTime(2100,2,1,
                          1,2,3,
                          1,
                          // this unix time is not equivalent to other parts
                          8102437600,
                          0))
       .append(pool.Int(0))
       .append(pool.Int(1))
       .append(pool.Int(-1))
       .append(pool.Int(2))
       .append(pool.Int(-2))
       .append(pool.Int(3))
       .append(pool.Int(-3))
       .append(pool.Int(std::numeric_limits<int64_t>::min()))
       .append(pool.Int(std::numeric_limits<int64_t>::max()))
       .append(pool.Int(std::numeric_limits<int64_t>::min() + 1))
       .append(pool.Null());
    return arr;
}

void reviewValue(FRPC::Value_t &val, int verMaj, int /*verMin*/)
{
    FRPC::Array_t &arr = FRPC::Array(val);
    FRPC::DateTime_t &dt = FRPC::DateTime(arr[0]);
    TEST(dt.getYear() == 2100);
    TEST(dt.getMonth() == 2);
    TEST(dt.getDay() == 1);
    TEST(dt.getHour() == 1);
    TEST(dt.getMin() == 2);
    TEST(dt.getSec() == 3);
    // we know v2 of the protocol can't transfer timestamp of this size
    if (verMaj > 3) {
        TEST(dt.getUnixTime() == 8102437600);
    }

    // ints
    TEST(FRPC::Int(arr[1]) ==  0);
    TEST(FRPC::Int(arr[2]) ==  1);
    TEST(FRPC::Int(arr[3]) == -1);
    TEST(FRPC::Int(arr[4]) ==  2);
    TEST(FRPC::Int(arr[5]) == -2);
    TEST(FRPC::Int(arr[6]) ==  3);
    TEST(FRPC::Int(arr[7]) == -3);

    TEST(FRPC::Int(arr[8]) == std::numeric_limits<int64_t>::min());
    TEST(FRPC::Int(arr[9]) == std::numeric_limits<int64_t>::max());
    TEST(FRPC::Int(arr[10]) == (std::numeric_limits<int64_t>::min() + 1));

    TEST(arr[11].isNull());
}

void testEncodeDecode(char major, char minor) {
    //
    FRPC::Pool_t pool;

    FRPC::Value_t &v = makeTestValue(pool);

    // ensure we're consistent on the input
    // enforcing v3 of the protocol here to test unixtime
    reviewValue(v, 3, 1);

    StringWriter_t sw;
    FRPC::ProtocolVersion_t pv(major, minor);
    FRPC::BinMarshaller_t bm(sw, pv);
    bm.packMethodResponse();
    FRPC::TreeFeeder_t feeder(bm);
    feeder.feedValue(v);
    bm.flush();

    // now unpack the buffer in the sw.target
    FRPC::TreeBuilder_t tb(pool);
    FRPC::BinUnMarshaller_t bum(tb);
    bum.unMarshall(sw.target.data(), static_cast<uint32_t>(sw.target.size()),
                   FRPC::UnMarshaller_t::TYPE_METHOD_RESPONSE);
    bum.finish();

    reviewValue(tb.getUnMarshaledData(), major, minor);
}

int main(int /*argc*/, char */*argv*/[]) {
    testEncodeDecode(2, 1);
    testEncodeDecode(3, 1);
    return fails ? EXIT_FAILURE : EXIT_SUCCESS;
}
