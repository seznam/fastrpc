/* Tests covering unmarshaller and marshaller parts of the fastrpc library.
 */

#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <memory>

#include "frpcunmarshaller.h"
#include "frpctreebuilder.h"

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
          testfile(DEFAULT_TEST_FILE)
    {}

    bool diffable; //!< Diffable mode - outputs a corrected test input
    bool usestdin;
    std::string testfile;
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

enum ErrorType_t {
    ERROR_UNKNOWN = 0,
    ERROR_BAD_PROTOCOL_VERSION,
    ERROR_UNEXPECTED_END,
    ERROR_FAULT_TEST
};

const char *errorTypeStr(ErrorType_t et) {
    switch (et) {
    case ERROR_BAD_PROTOCOL_VERSION: return "bad protocol version";
    case ERROR_UNEXPECTED_END:       return "unexpected data end";
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
    if (err.what() ==  std::string("Unsupported protocol version !!!"))
        return ERROR_BAD_PROTOCOL_VERSION;

    if (err.what() ==  std::string("Stream not complete"))
        return ERROR_UNEXPECTED_END;

    error() << "Unhandled FRPC::StreamError_t " << err.what() << std::endl;

    return ERROR_UNKNOWN;
}

template<>
ErrorType_t parseErrorType(const FRPC::Fault_t &err) {
    if (err.what() ==  std::string("No data unmarshalled"))
        return ERROR_UNEXPECTED_END;

    if (err.what() ==  std::string("FAULT_TEST"))
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
};

/** Runs the core test, returns corrected result */
std::pair<TestInstance_t, TestResult_t>
runTest(const TestSettings_t &ts, const TestInstance_t &ti,
        size_t testNum, size_t lineNum)
{
    TestInstance_t corrected;
    TestResult_t result(TEST_PASSED);

    try {
        FRPC::Pool_t pool;
        FRPC::TreeBuilder_t builder(pool);

        std::auto_ptr<FRPC::UnMarshaller_t>
            unmarshaller = std::auto_ptr<FRPC::UnMarshaller_t>(
                    FRPC::UnMarshaller_t::create(
                            FRPC::UnMarshaller_t::BINARY_RPC,
                            builder));

        unmarshaller->unMarshall(ti.binary.data(),
                                 ti.binary.size(),
                                 FRPC::UnMarshaller_t::TYPE_ANY);

        unmarshaller->finish();

        // TODO: Fix - introduce a normalized dump method
        // Text format of the deserialized data
        std::string txtree;
        corrected.text = builder.getUnMarshaledMethodName();
        FRPC::dumpFastrpcTree(builder.getUnMarshaledData(), txtree, -1);
        corrected.text += txtree;

        // TODO: Binary should be generated by marshaller
        corrected.binary = ti.binary;

        // TODO: convert the intermediate format back to binary
    } catch (const FRPC::StreamError_t &ex) {
        ErrorType_t ert = parseErrorType(ex);
        corrected.text = std::string("error(")+errorTypeStr(ert)+")";
    } catch (const FRPC::Fault_t &ex) {
        ErrorType_t ert = parseErrorType(ex);
        corrected.text = std::string("error(")+errorTypeStr(ert)+")";
    } catch (const std::exception &ex) {
        corrected.text = std::string("error(")+ex.what()+")";
    }

    // compare the unmarshalled data
    if (corrected.text != ti.text) {
        result.set(TEST_FAILED, corrected.text + " <> " + ti.text);
    }

    return std::make_pair(corrected, result);
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
            ti.binary = unhex(line); ps = PS_TEXT;
            if (ts.diffable)
                std::cout << line << std::endl;
            break;
        case PS_TEXT:
            // empty line denotes expected result end.
            if (line.empty()) {
                ps = PS_BINARY;

                std::pair<TestInstance_t, TestResult_t> result =
                    runTest(ts, ti, testNum, lineNum);

                if (ts.diffable) {
                    std::cout << result.first.text << std::endl << std::endl;
                }

                if (result.second() != TEST_PASSED) {
                    ++errCount;
                    error() << "Failed test no. " << testNum
                            << " '" << testName << "'"
                            << " in " << ts.testfile << ":" << lineNum
                            << " with '" << result.second.comment << "'"
                            << std::endl;
                } else {
                    success() << "Passed test no. "
                              << testNum
                              << " '" << testName << "'"
                              << std::endl;
                }

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
        std::ifstream infile(ts.testfile);
        runTests(ts, infile);
    }
}

int main(int argc, const char *argv[]) {
    doColor = ::isatty(::fileno(::stderr));

    Args_t args(argv + 1, argv + argc);
    TestSettings_t ts;
    processArgs(ts, args);

    runTests(ts);

    if (errCount) {
        std::cerr << "----" << std::endl;
        error() << "Test contained " << errCount << " error(s)." << std::endl;
        return 1;
    }

    return 0;
}
