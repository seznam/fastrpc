/* Tests covering unmarshaller and marshaller parts of the fastrpc library.
 */

#include <cstring>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>

#include "frpcunmarshaller.h"
#include "frpcmarshaller.h"
#include "frpctreebuilder.h"
#include "frpcwriter.h"
#include "frpctreefeeder.h"
#include "frpccompare.h"

/*

Uses external test file that contains both binary representation and resulting FRPC format

 */

const std::string DEFAULT_TEST_FILE = "frpc.tests";

namespace color {

enum Code_t {
    FG_RED      = 31,
    FG_GREEN    = 32,
    FG_BLUE     = 34,
    FG_DEFAULT  = 39,
    BG_RED      = 41,
    BG_GREEN    = 42,
    BG_BLUE     = 44,
    BG_DEFAULT  = 49
};

class Modifier_t {
    Code_t code;
public:
    Modifier_t(Code_t col) : code(col) {}

    friend std::ostream&
    operator<<(std::ostream& os, const Modifier_t& mod) {
        return os << "\033[" << mod.code << "m";
    }
};

}

class StrWriter_t : public FRPC::Writer_t {
public:
    StrWriter_t(std::string &tgt)
        : FRPC::Writer_t()
        , target(tgt)
    {}

    ~StrWriter_t()
    {}

    virtual void write(const char *data, unsigned int size) {
        target.append(data, size);
    }

    virtual void flush() {}

private:
    std::string &target;
};


static const color::Modifier_t cl_red(color::FG_RED);
static const color::Modifier_t cl_green(color::FG_GREEN);
static const color::Modifier_t cl_default(color::FG_DEFAULT);

typedef std::vector<std::string> Args_t;

std::string extract_arg(Args_t::const_iterator &it,
                        Args_t::const_iterator iend)
{
    if (++it == iend)
        throw std::runtime_error("Command expects arguments!");
    return *it;
}

struct TestSettings_t {
    TestSettings_t()
        : diffable(false),
          usestdin(false),
          testfile(DEFAULT_TEST_FILE),
          verbose(false)
    {}

    bool diffable; //!< Diffable mode - outputs a corrected test input
    bool usestdin;
    std::string testfile;
    bool verbose;
};

void processArgs(TestSettings_t &s, const Args_t &args) {
    for (Args_t::const_iterator
             it = args.begin(),
             iend = args.end();
         it != iend;
         ++it)
    {
        if (*it == "diffable") {
            s.diffable = true;
        } else if (*it == "stdin") {
            s.usestdin = true;
        } else if (*it == "testfile") {
            s.testfile = extract_arg(it, iend);
        } else if (*it == "verbose") {
            s.verbose = true;
        } else {
            std::cerr << "Unknown parameter " << *it
                      << " expected [diffable] [stdin] [testfile filename]" << std::endl;
            return;
        }
    }
}

int nextHexNibble(std::string::const_iterator &it,
                  std::string::const_iterator &iend)
{
    while (it != iend) {
        unsigned char c = static_cast<unsigned char>(*it);
        ++it;

        switch (c) {
        case '0' ... '9': return c - '0';
        case 'a' ... 'f': return c - 'a' + 10;
        case 'A' ... 'F': return c - 'A' + 10;
        // Whitespace moves us to the next character
 case '\n':
        case  ' ':
        case '\t': continue;
        default: return -2;
        }
    }

    return -1;
}

void skipWS(std::string::const_iterator &it,
            const std::string::const_iterator &iend)
{
    for (;it != iend; ++it) {
        unsigned char c = static_cast<unsigned char>(*it);
        switch (c) {
        // Whitespace moves us to the next character
        case '\n':
        case  ' ':
        case '\t': continue;
        default: return;
        }
    }
}

void pushString(std::string &buf,
                std::string::const_iterator &it,
                const std::string::const_iterator iend)
{
    bool inEsc = false;

    for (; it != iend; ++it) {
        unsigned char c = static_cast<unsigned char>(*it);

        if (c == '\\') {
            inEsc = true;
            continue;
        }

        if (c == '"' && !inEsc) {
            ++it;
            break;
        }

        inEsc = false;
        buf.push_back(c);
    }

}

std::string unhex(const std::string &hex) {
    std::string result;
    result.reserve(hex.length() / 2);

    std::string::const_iterator it = hex.begin(),
                              iend = hex.end();
    while (it != iend) {
        // skip spaces, etc
        skipWS(it, iend);

        // try to read string literals if present
        if (*it == '"') {
            ++it;
            pushString(result, it, iend);
            continue;
        }

        // read it out as a hexadecimal byte
        int a = nextHexNibble(it, iend);
        int b = nextHexNibble(it, iend);

        // negative values denounce error
        if ((a < 0) || (b < 0))
            throw std::runtime_error(
                    "encountered invalid character in hex stream");

        result.push_back((a << 4) | b);
    }

    return result;
}

std::string hex(const std::string &binary) {
    static const char *ht = "0123456789ABCDEF";
    std::string result;

    result.reserve(binary.size() * 2);

    for (std::string::const_iterator it = binary.begin(), iend = binary.end();
         it != iend;
         ++it)
    {
        result.push_back(ht[(*it >> 4) & 0x0f]);
        result.push_back(ht[*it        & 0x0f]);
    }

    return result;
}

enum ErrorType_t {
    ERROR_UNKNOWN = 0,
    ERROR_BAD_PROTOCOL_VERSION,
    ERROR_UNEXPECTED_END,
    ERROR_FAULT_TEST,
    ERROR_INVALID_TYPE,
    ERROR_INVALID_INT_SIZE,
    ERROR_INVALID_STR_SIZE,
    ERROR_INVALID_BIN_SIZE,
    ERROR_INVALID_STRUCT_KEY_SIZE,
    ERROR_INVALID_FAULT,
    ERROR_INVALID_BOOL_VALUE,
    ERROR_INVALID_ARRAY_SIZE,
    ERROR_INVALID_MESSAGE_TYPE,
    ERROR_DATA_AFTER_END,
    ERROR_ENTITY_TOO_LARGE,
    ERROR_BAD_CALL_NAME
};

const char *errorTypeStr(ErrorType_t et) {
    switch (et) {
    case ERROR_BAD_PROTOCOL_VERSION: return "bad protocol version";
    case ERROR_UNEXPECTED_END:       return "unexpected data end";
    case ERROR_FAULT_TEST:           return "fault test";
    case ERROR_INVALID_TYPE:         return "unknown type";
    case ERROR_INVALID_INT_SIZE:     return "bad size";
    case ERROR_INVALID_STR_SIZE:     return "bad size";
    case ERROR_INVALID_BIN_SIZE:     return "bad size";
    case ERROR_INVALID_STRUCT_KEY_SIZE: return "bad key length";
    case ERROR_INVALID_FAULT:        return "invalid fault";
    case ERROR_INVALID_BOOL_VALUE:   return "invalid bool value";
    case ERROR_INVALID_ARRAY_SIZE:   return "invalid array length";
    case ERROR_INVALID_MESSAGE_TYPE: return "invalid message type";
    case ERROR_DATA_AFTER_END:       return "data after end";
    case ERROR_ENTITY_TOO_LARGE:     return "entity too large";
    case ERROR_BAD_CALL_NAME:        return "bad call name";
    case ERROR_UNKNOWN:
    default:
        return "unknown";
    }
}

template<typename ExceptionT>
ErrorType_t parseErrorType(const ExceptionT &ex) {
    return ERROR_UNKNOWN;
}

bool doColor = false;

std::ostream &success() {
    if (!doColor)
        return std::cerr <<  "[ OK ] ";
    return std::cerr << cl_green << "[ OK ]" << cl_default << " ";
}

std::ostream &error() {
    if (!doColor)
        return std::cerr <<  "[ !! ] Error: ";
    return std::cerr << cl_red << "[ !! ]" << cl_default << " Error: ";
}

template<>
ErrorType_t parseErrorType(const FRPC::StreamError_t &err) {
    if (err.what() == std::string("Unsupported protocol version !!!"))
        return ERROR_BAD_PROTOCOL_VERSION;

    if (err.what() == std::string("Stream not complete"))
        return ERROR_UNEXPECTED_END;

    if (err.what() == std::string("Don't known this type"))
        return ERROR_INVALID_TYPE;

    if (err.what() == std::string("Unknown value type"))
        return ERROR_INVALID_TYPE;

    if (err.what() == std::string("Illegal element length"))
        return ERROR_INVALID_INT_SIZE;

    if (err.what() == std::string("Size of int is 0 or > 4 !!!"))
        return ERROR_INVALID_INT_SIZE;

    if (err.what() == std::string("Size of string length is 0 !!!"))
        return ERROR_INVALID_STR_SIZE;

    if (err.what() == std::string("Size of binary length is 0 !!!"))
        return ERROR_INVALID_BIN_SIZE;

    if (err.what() == std::string("Lenght of member name is 0 not in interval (1-255)"))
        return ERROR_INVALID_STRUCT_KEY_SIZE;

    if (err.what() == std::string("Length of member name is 0 not in interval (1-255)"))
        return ERROR_INVALID_STRUCT_KEY_SIZE;

    if (err.what() == std::string("Struct member name length is zero"))
        return ERROR_INVALID_STRUCT_KEY_SIZE;

    if (err.what() == std::string("Only second value of fault can be string"))
        return ERROR_INVALID_FAULT;

    if (err.what() == std::string("Only first value of fault can be int"))
        return ERROR_INVALID_FAULT;

    if (err.what() == std::string("Only second value of fault can be string"))
        return ERROR_INVALID_FAULT;

    if (err.what() == std::string("Invalid bool value"))
        return ERROR_INVALID_BOOL_VALUE;

    if (err.what() == std::string("Array too long !!!"))
        return ERROR_INVALID_ARRAY_SIZE;

    if (err.what() == std::string("Invalid stream message type"))
        return ERROR_INVALID_MESSAGE_TYPE;

    if (err.what() == std::string("Unexpected value after end"))
        return ERROR_DATA_AFTER_END;

    if (err.what() == std::string("Binary entity too large"))
        return ERROR_ENTITY_TOO_LARGE;

    if (err.what() == std::string("String entity too large"))
        return ERROR_ENTITY_TOO_LARGE;

    if (err.what() == std::string("Array entity too large"))
        return ERROR_ENTITY_TOO_LARGE;

    if (err.what() == std::string("Struct entity too large"))
        return ERROR_ENTITY_TOO_LARGE;

    if (err.what() == std::string("Bad call name"))
        return ERROR_BAD_CALL_NAME;

    error() << "Unhandled FRPC::StreamError_t " << err.what() << std::endl;

    return ERROR_UNKNOWN;
}

template<>
ErrorType_t parseErrorType(const FRPC::Fault_t &err) {
    if (err.what() ==  std::string("No data unmarshalled"))
        return ERROR_UNEXPECTED_END;

    if (err.what() == std::string("FAULT_TEST"))
        return ERROR_FAULT_TEST;

    error() << "Unhandled FRPC::Fault_t " << err.what() << std::endl;
    return ERROR_UNKNOWN;
}

// A single test instance containing source and destination data
struct TestInstance_t {
    std::string binary;
    std::string text;

    void reset() {
        binary.clear();
        text.clear();
    }
};

enum TestResultType_t {
    TEST_PASSED = 0,
    TEST_FAILED = 1,
    TEST_FAILED_ERROR = 2
};

struct TestResult_t {
    TestResult_t(TestResultType_t tr, const std::string &com = "")
        : result(tr), comment(com)
    {}

    void set(TestResultType_t tr, const std::string &com = "") {
        result = tr;
        comment = com;
    }

    TestResultType_t operator()() const {
        return result;
    }

    TestResultType_t result;
    std::string comment;
    std::string corrected;
};

std::string toStr(int i) {
    std::ostringstream s;
    s << i;
    return s.str();
}

std::unique_ptr<FRPC::UnMarshaller_t> unmarshall(FRPC::TreeBuilder_t &builder,
                                               const char *data,
                                               size_t size,
                                               size_t offset = 0,
                                               size_t step = 0)
{
    std::unique_ptr<FRPC::UnMarshaller_t>
        unmarshaller = std::unique_ptr<FRPC::UnMarshaller_t>(
                FRPC::UnMarshaller_t::create(
                        FRPC::UnMarshaller_t::BINARY_RPC,
                            builder));

    if (!step) {
        step = size - offset;
    }

    const char *p = data;
    const char *end = data + size;
    while (p < end) {
        size_t rest = end - p;
        size_t ds = rest > step ? step : rest;

        if (offset != 0) {
            ds = offset > rest ? rest : offset;
            offset = 0;
        }

        unmarshaller->unMarshall(p,
                                 ds,
                                 FRPC::UnMarshaller_t::TYPE_ANY);
        p += ds;
    }

    unmarshaller->finish();
    return unmarshaller;
}


void formatTextDump(FRPC::TreeBuilder_t &builder, std::string &tgt) {
    std::string txtree;
    tgt = builder.getUnMarshaledMethodName();
    FRPC::dumpFastrpcTree(builder.getUnMarshaledData(), txtree, -1);

    tgt += txtree;
}


FRPC::Value_t& serDeser(FRPC::Pool_t &pool,
                        FRPC::TreeBuilder_t &orig,
                        FRPC::ProtocolVersion_t pv,
                        size_t offset = 0,
                        size_t step = 0)
{
    std::string bintarget;

    // // marshall into string again and see how we differ
    StrWriter_t sw(bintarget);
    std::string mname = orig.getUnMarshaledMethodName();

    std::unique_ptr<FRPC::Marshaller_t> marshaller
        (FRPC::Marshaller_t::create
         (FRPC::Marshaller_t::BINARY_RPC, sw,
          pv));

    FRPC::TreeFeeder_t feeder(*marshaller);

    if (!mname.empty()) {
        marshaller->packMethodCall(mname.c_str());
        FRPC::Array_t &params = FRPC::Array(orig.getUnMarshaledData());
        for (FRPC::Array_t::const_iterator
                 iparams = params.begin(),
                 eparams = params.end();
             iparams != eparams; ++iparams) {
            feeder.feedValue(**iparams);
        }
    } else {
        marshaller->packMethodResponse();
        feeder.feedValue(orig.getUnMarshaledData());
    }

    marshaller->flush();

    // after this we deserialize again
    FRPC::TreeBuilder_t builder(pool);
    std::unique_ptr<FRPC::UnMarshaller_t> unm
        = unmarshall(builder, bintarget.data(), bintarget.size(), offset, step);

    if (FRPC::compare(orig.getUnMarshaledData(),
                      builder.getUnMarshaledData()) != 0)
    {
        throw std::runtime_error("remarshalled data yield different resuilt");
    }

    return builder.getUnMarshaledData();
}

// just a counter of individual compares done
size_t passedChecks = 0;
size_t passedTests  = 0;

/** Runs the core test, returns corrected result */
TestResult_t runTest(FRPC::Pool_t &pool, const TestSettings_t &/*ts*/,
                     const TestInstance_t &ti,
                     size_t /*testNum*/, size_t /*lineNum*/, FRPC::Value_t &value,
                     const std::string &origTxt,
                     size_t offset, size_t step)
{
    TestResult_t result(TEST_PASSED);
    std::string secondTxtForm;

    FRPC::TreeBuilder_t builder(pool);

    std::unique_ptr<FRPC::UnMarshaller_t> unmarshaller
        = unmarshall(builder, ti.binary.data(), ti.binary.size(),
                     offset, step);

    if (FRPC::compare(builder.getUnMarshaledData(), value) != 0) {
        std::string myTxt;
        formatTextDump(builder, myTxt);
        result.set(TEST_FAILED,
                   "Unmarshalled data yield different result : \n\t"
                   + origTxt + "\n\t" + myTxt);

        return result;
    }

    ++passedChecks;

    FRPC::Value_t &second =
        serDeser(pool,
                 builder,
                 unmarshaller->getProtocolVersion(),
                 offset, step);

    if (FRPC::compare(builder.getUnMarshaledData(), second) != 0) {
        std::string myTxt;
        formatTextDump(builder, myTxt);
        result.set(TEST_FAILED,
                   "Remarshalled data yield different result : \n\t"
                   + origTxt + "\n\t" + myTxt);

        return result;
    }

    ++passedChecks;

    return result;
}

TestResult_t
runTest(const TestSettings_t &ts, const TestInstance_t &ti,
        size_t testNum, size_t lineNum)
{
    TestResult_t result(TEST_PASSED);

    try {
        FRPC::Pool_t pool;
        FRPC::TreeBuilder_t builder(pool);

        std::unique_ptr<FRPC::UnMarshaller_t> unmarshaller
            = unmarshall(builder, ti.binary.data(), ti.binary.size(),
                         0, ti.binary.size());

        // TODO: Fix - introduce a normalized dump method
        // Text format of the deserialized data
        formatTextDump(builder, result.corrected);

        // compare the unmarshalled data
        if (result.corrected != ti.text) {
            result.set(TEST_FAILED, result.corrected + " <> " + ti.text);
            return result;
        }
        bool wasError = false;

        // run the test with combo of offsets+sizes
        for (size_t offset = 0; offset < ti.binary.size() && !wasError;
             ++offset)
        {
            for (size_t step = 1;
                 (step < ti.binary.size() - offset) && !wasError;
                 ++step)
            {
                if (step == offset)
                    continue;

                result = runTest(pool, ts, ti, testNum, lineNum,
                                 builder.getUnMarshaledData(),
                                 result.corrected,
                                 offset, step);

                if (result() != TEST_PASSED)
                    return result;
            }
        }

        return result;

    } catch (const FRPC::StreamError_t &ex) {
        ErrorType_t ert = parseErrorType(ex);
        result.corrected = std::string("error(")+errorTypeStr(ert)+")";
    } catch (const FRPC::Fault_t &ex) {
        if (ex.errorNum() > 0) {
            // fault contains integral error number, let's contain it in the output
            std::string errNumStr = toStr(ex.errorNum());
            result.corrected = std::string("fault(") + errNumStr + ", "
                               + ex.what() + ")";
        } else {
            ErrorType_t ert = parseErrorType(ex);
            result.corrected = std::string("error(") + errorTypeStr(ert) + ")";
        }
    } catch (const std::exception &ex) {
        result.corrected = std::string("error(")+ex.what()+")";
    }

    if (result.corrected != ti.text) {
        result.set(TEST_FAILED, result.corrected + " <> " + ti.text);
    }

    return result;
}


enum ParseState_t {
    PS_BINARY,
    PS_TEXT
};

bool isCommentLine(const std::string &str) {
    if(str.empty())
        return false;

    if (str[0] == '#')
        return true;

    return false;
}

bool parseTestName(const std::string &str, std::string &name) {
    if(str.empty())
        return false;

    if (str[0] == '@') {
        name = str.substr(1);
        return true;
    }

    return false;
}

size_t errCount = 0;

void runTests(const TestSettings_t &ts, std::istream &input) {
    // parse the testfile. Anything starting with # is a skipped line
    std::string line;
    std::string testName;

    ParseState_t ps = PS_BINARY;

    TestInstance_t ti;
    size_t testNum = 1;

    for (size_t lineNum = 0; std::getline(input, line); ++lineNum) {
        if (isCommentLine(line)) {
            if (ts.diffable)
                std::cout << line << std::endl;
            continue;
        }

        if (parseTestName(line, testName)) {
            if (ts.diffable)
                std::cout << line << std::endl;
            continue;
        }

        switch (ps) {
        case PS_BINARY:
            if (line.empty()) {
                if (ts.diffable)
                    std::cout << std::endl;
                continue;
            }

            ti.binary = unhex(line); ps = PS_TEXT;
            if (ts.diffable)
                std::cout << line << std::endl;
            break;
        case PS_TEXT:
            // empty line denotes expected result end.
            if (line.empty()) {
                ps = PS_BINARY;

                int outputs = -1;
                if (ts.diffable) outputs = 1;

                TestResult_t result = runTest(ts, ti, testNum, lineNum);

                if (ts.diffable && outputs) {
                    std::cout << result.corrected << std::endl << std::endl;
                }

                if (result() != TEST_PASSED) {
                    ++errCount;
                    error() << "Failed test no. " << testNum
                            << " '" << testName << "'"
                            << " in " << ts.testfile << ":" << lineNum
                            << " with '" << result.comment << "'\n";
                } else {
                    ++passedTests;

                    if (ts.verbose) {
                        success() << "Passed test no. "
                                  << testNum
                                  << " '" << testName << "'\n";
                    }
                }

                if (outputs != 0) --outputs;

                ti.reset();
                ++testNum;
                break;
            }

            if (!ti.text.empty())
                ti.text.push_back('\n');

            ti.text.append(line);
        }
    }

    if (ps != PS_BINARY) {
        ++errCount;
        error() << "The last test is incompletely defined."
                  << std::endl;
    }
}

void runTests(const TestSettings_t &ts) {
    if (ts.usestdin) {
        runTests(ts, std::cin);
    } else {
        std::ifstream infile(ts.testfile.c_str());
        if (!infile.is_open()) {
            throw std::runtime_error(
                    "Could not open the test file " + ts.testfile);
        }
        runTests(ts, infile);
    }
}

int main(int argc, const char *argv[]) {
    doColor = ::isatty(::fileno(::stderr));

    Args_t args(argv + 1, argv + argc);
    TestSettings_t ts;
    processArgs(ts, args);

    runTests(ts);

    std::cerr << "----" << std::endl;

    if (errCount) {
        error() << "Test contained " << errCount << " error(s)." << std::endl;
        return 1;
    } else {
        success() << "Passed " << passedTests << " tests with "
                  << passedChecks << " individual checks" << std::endl;
    }

    return 0;
}
