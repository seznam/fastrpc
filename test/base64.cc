#include <iostream>
#include <cassert>
#include <memory>

#include "frpcbase64.h"
#include "frpcb64writer.h"

using namespace FRPC;

class MyWriter_t: public Writer_t {
public:

    void write(const char *d, unsigned int size ) {
        data.append(d, size);
    }

    void flush(){

    }

    std::string data;
};

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

std::string encode_b64(std::string source) {
    MyWriter_t mywriter;
    Base64Writer_t writer((Writer_t &)mywriter);

    writer.write(source.c_str(), source.size());
    writer.flush();

    return mywriter.data;
}

void test_simple_decode(std::string value, unsigned line_num) {
    std::string value_b64(encode_b64(value));

    std::string value_decoded(Base64::decode(value_b64.c_str(), value_b64.size()));

    if (value_decoded != value) {
        std::cerr << "failed test on line " << line_num << std::endl;
        std::cerr << value << std::endl;
        std::cerr << value_b64 << std::endl;
        TEST(value_decoded == value);
    }
}

void test_stream_decode(std::string value, unsigned line_num) {
    std::string value_b64(encode_b64(value));

    // break value in 2 parts and try to decode that
    for (int i = 0; i < value_b64.size(); ++i) {
        Base64 decoder;
        std::string value_decoded;
        value_decoded.append(decoder.process(value_b64.c_str(), i));
        value_decoded.append(decoder.process(value_b64.c_str()+i, value_b64.size() - i));
        if (value_decoded != value) {
            std::cerr << "failed stream test on line " << line_num << " break i: " << i << std::endl;
            std::cerr << value << std::endl;
            std::cerr << value_b64 << std::endl;
            TEST(value_decoded == value);
        }
    }
}

void test_decode(std::string value, unsigned line_num) {
    test_simple_decode(value, line_num);
    test_stream_decode(value, line_num);
}

// encode -> decode test
// g++ base64.cc ../src/frpcwriter.cc  ../src/frpcbase64.cc ../src/frpcb64writer.cc && ./a.out
int main(int argc, const char *argv[])
{
    for (int i=0; i<500; ++i) {
        test_decode(std::string(i, 'a'), __LINE__);
    }

    test_decode("", __LINE__);
    test_decode("\n\n\t\r\vsdfsadlkjfsadj  klfds", __LINE__);
    test_decode("\ndlfkajsdflkj23lkj2342342l3kj4l2k3j4l2knhs", __LINE__);
    test_decode("/", __LINE__);
    test_decode(std::string(100, 's'), __LINE__);
    test_decode(std::string(100, '\n'), __LINE__);
    test_decode(std::string(101, '2'), __LINE__);
    test_decode(std::string(401, 'a'), __LINE__);

    return 0;
}
