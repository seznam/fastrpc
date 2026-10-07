/* Runner for the FastRPC conformance vectors (email/fastrpc-conformance).
 *
 * Reads JSON requests from stdin and answers one JSON line per request, see
 * FORMAT.md (Runner protocol) in fastrpc-conformance:
 *
 *   pytest harness --impl libfastrpc --runner "<build>/conformance-runner"
 *
 * It calls the binary marshaller and unmarshaller directly, as
 * test_marshallers does. The options datetime-validation and
 * string-validation switch the LibConfig_t policies for each request.
 */

#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <nlohmann/json.hpp>

#include "frpc.h"
#include "frpcconfig.h"
#include "frpcencodingerror.h"
#include "frpcmarshaller.h"
#include "frpcstreamerror.h"
#include "frpctreebuilder.h"
#include "frpctreefeeder.h"
#include "frpcunmarshaller.h"
#include "frpcwriter.h"

using Json_t = nlohmann::json;

namespace {

const int QUARTER = 15 * 60;
const char *REVISIONS[] = {"1.0", "2.0", "2.1", "3.0"};

/** The value is valid, but libfastrpc cannot hold or write it. */
struct Unrepresentable_t: std::runtime_error {
    using std::runtime_error::runtime_error;
};

/** The request does not follow the runner protocol. */
struct BadRequest_t: std::runtime_error {
    using std::runtime_error::runtime_error;
};

/** The only member of a one-key object: the canonical value tag. */
Json_t::const_iterator tagged(const Json_t &json) {
    if (!json.is_object() || json.size() != 1)
        throw BadRequest_t("expected a one-key object");
    return json.begin();
}

//=============================================================================
// Hex and numbers
//----------------

std::string toHex(const std::string &data) {
    static const char *digits = "0123456789ABCDEF";
    std::string out;
    out.reserve(data.size() * 2);
    for (unsigned char c: data) {
        out.push_back(digits[c >> 4]);
        out.push_back(digits[c & 0x0f]);
    }
    return out;
}

std::string fromHex(const std::string &hex) {
    if (hex.size() % 2) throw BadRequest_t("odd number of hex digits");
    std::string out;
    for (size_t i = 0; i < hex.size(); i += 2)
        out.push_back(char(std::stoi(hex.substr(i, 2), nullptr, 16)));
    return out;
}

int64_t toInt64(const std::string &text) {
    try {
        size_t used = 0;
        long long v = std::stoll(text, &used, 10);
        if (used != text.size()) throw BadRequest_t("invalid number " + text);
        return v;
    } catch (const std::out_of_range &) {
        throw Unrepresentable_t(text + " does not fit int64");
    }
}

FRPC::ProtocolVersion_t protocol(const std::string &revision) {
    for (const char *known: REVISIONS) {
        if (revision == known) {
            return FRPC::ProtocolVersion_t(char(revision[0] - '0'),
                                           char(revision[2] - '0'));
        }
    }
    throw FRPC::StreamError_t("Unsupported protocol version !!!");
}

//=============================================================================
// Decoding: libfastrpc values -> canonical JSON
//-----------------------------------------------

class Builder_t: public FRPC::TreeBuilder_t {
public:
    using FRPC::TreeBuilder_t::TreeBuilder_t;
    using FRPC::TreeBuilder_t::buildMethodCall;
    using FRPC::TreeBuilder_t::buildFault;

    void buildMethodCall(const char *name, unsigned int size) override {
        isCall = true;
        FRPC::TreeBuilder_t::buildMethodCall(name, size);
    }

    void buildFault(int code, const char *msg, unsigned int size) override {
        isFault = true;
        FRPC::TreeBuilder_t::buildFault(code, msg, size);
    }

    bool isCall = false;
    bool isFault = false;
};

Json_t canonical(const FRPC::Value_t &v) {
    switch (v.getType()) {
    case FRPC::Int_t::TYPE:
        return {{"int", std::to_string(FRPC::Int(v).getValue())}};
    case FRPC::Bool_t::TYPE:
        return {{"bool", FRPC::Bool(v).getValue()}};
    case FRPC::Double_t::TYPE: {
        double d = FRPC::Double(v).getValue();
        uint64_t bits;
        std::memcpy(&bits, &d, sizeof(bits));
        std::string octets;
        for (int i = 7; i >= 0; --i) octets.push_back(char(bits >> (8 * i)));
        return {{"double_bits", toHex(octets)}};
    }
    case FRPC::String_t::TYPE:
        return {{"string", toHex(FRPC::String(v).getValue())}};
    case FRPC::Binary_t::TYPE:
        return {{"binary", toHex(FRPC::Binary(v).getValue())}};
    case FRPC::Null_t::TYPE:
        return {{"null", nullptr}};
    case FRPC::DateTime_t::TYPE: {
        const FRPC::DateTime_t &dt = FRPC::DateTime(v);
        Json_t fields = {dt.getYear(), dt.getMonth(), dt.getDay(),
                         dt.getHour(), dt.getMin(), dt.getSec(),
                         dt.getDayOfWeek()};
        return {{"datetime",
                 {{"ts", std::to_string(dt.getUnixTime())},
                  {"zone", dt.getTimeZone() / QUARTER},
                  {"fields", fields}}}};
    }
    case FRPC::Array_t::TYPE: {
        Json_t items = Json_t::array();
        for (const FRPC::Value_t *item: FRPC::Array(v))
            items.push_back(canonical(*item));
        return {{"array", items}};
    }
    case FRPC::Struct_t::TYPE: {
        Json_t members = Json_t::array();
        for (const auto &member: FRPC::Struct(v))
            members.push_back({toHex(member.first),
                               canonical(*member.second)});
        return {{"struct", members}};
    }
    default:
        throw std::runtime_error(std::string("unexpected type ")
                                 + v.getTypeName());
    }
}

Json_t decode(const std::string &data) {
    FRPC::Pool_t pool;
    Builder_t builder(pool);
    std::unique_ptr<FRPC::UnMarshaller_t> unmarshaller(
            FRPC::UnMarshaller_t::create(FRPC::UnMarshaller_t::BINARY_RPC,
                                         builder));
    unmarshaller->unMarshall(data.data(), unsigned(data.size()),
                             FRPC::UnMarshaller_t::TYPE_ANY);
    unmarshaller->finish();

    if (builder.isFault) {
        return {{"fault",
                 {{"code",
                   std::to_string(builder.getUnMarshaledErrorNumber())},
                  {"message",
                   toHex(builder.getUnMarshaledErrorMessage())}}}};
    }
    FRPC::Value_t &value = builder.getUnMarshaledData();
    if (builder.isCall) {
        Json_t params = Json_t::array();
        for (const FRPC::Value_t *param: FRPC::Array(value))
            params.push_back(canonical(*param));
        return {{"call",
                 {{"name", toHex(builder.getUnMarshaledMethodName())},
                  {"params", params}}}};
    }
    return {{"response", canonical(value)}};
}

//=============================================================================
// Encoding: canonical JSON -> libfastrpc values -> octets
//--------------------------------------------------------

FRPC::Value_t &build(const Json_t &json, FRPC::Pool_t &pool);

FRPC::DateTime_t &buildDateTime(const Json_t &json, FRPC::Pool_t &pool) {
    int64_t ts = toInt64(json.at("ts").get<std::string>());
    int zone = json.at("zone").get<int>() * QUARTER;
    const Json_t &fields = json.at("fields");
    if (fields.is_array() && fields.size() == 7) {
        return pool.DateTime(
                short(fields[0].get<int>()), char(fields[1].get<int>()),
                char(fields[2].get<int>()), char(fields[3].get<int>()),
                char(fields[4].get<int>()), char(fields[5].get<int>()),
                char(fields[6].get<int>()), time_t(ts), zone);
    }
    // libfastrpc computes the fields itself
    return pool.DateTime(time_t(ts), zone);
}

FRPC::Value_t &build(const Json_t &json, FRPC::Pool_t &pool) {
    auto it = tagged(json);
    const std::string &tag = it.key();
    const Json_t &item = it.value();
    if (tag == "null") return pool.Null();
    if (tag == "bool") return pool.Bool(item.get<bool>());
    if (tag == "int") return pool.Int(toInt64(item.get<std::string>()));
    if (tag == "double")
        return pool.Double(std::stod(item.get<std::string>()));
    if (tag == "double_bits") {
        uint64_t bits = std::stoull(item.get<std::string>(), nullptr, 16);
        double d;
        std::memcpy(&d, &bits, sizeof(d));
        return pool.Double(d);
    }
    if (tag == "string") return pool.String(fromHex(item.get<std::string>()));
    if (tag == "binary") return pool.Binary(fromHex(item.get<std::string>()));
    if (tag == "datetime") return buildDateTime(item, pool);
    if (tag == "array") {
        FRPC::Array_t &array = pool.Array();
        for (const Json_t &member: item) array.append(build(member, pool));
        return array;
    }
    if (tag == "struct") {
        FRPC::Struct_t &st = pool.Struct();
        for (const Json_t &member: item) {
            st.append(fromHex(member.at(0).get<std::string>()),
                      build(member.at(1), pool));
        }
        return st;
    }
    throw BadRequest_t("unknown value " + tag);
}

struct StringWriter_t: FRPC::Writer_t {
    void write(const char *data, unsigned int size) override {
        target.append(data, size);
    }
    void flush() override {}
    std::string target;
};

std::string encode(const Json_t &message, const std::string &revision) {
    FRPC::Pool_t pool;
    StringWriter_t writer;
    std::unique_ptr<FRPC::Marshaller_t> marshaller(
            FRPC::Marshaller_t::create(FRPC::Marshaller_t::BINARY_RPC,
                                       writer, protocol(revision)));
    FRPC::TreeFeeder_t feeder(*marshaller);
    auto it = tagged(message);
    const Json_t &item = it.value();
    if (it.key() == "call") {
        std::string name = fromHex(item.at("name").get<std::string>());
        FRPC::Array_t &params = pool.Array();
        for (const Json_t &p: item.at("params")) params.append(build(p, pool));
        marshaller->packMethodCall(name.data(), unsigned(name.size()));
        for (const FRPC::Value_t *param: params) feeder.feedValue(*param);
    } else if (it.key() == "response") {
        FRPC::Value_t &value = build(item, pool);
        marshaller->packMethodResponse();
        feeder.feedValue(value);
    } else if (it.key() == "fault") {
        int64_t code = toInt64(item.at("code").get<std::string>());
        std::string msg = fromHex(item.at("message").get<std::string>());
        marshaller->packFault(int(code), msg.data(), unsigned(msg.size()));
    } else {
        throw BadRequest_t("unknown message " + it.key());
    }
    marshaller->flush();
    return toHex(writer.target);
}

//=============================================================================
// Requests
//---------

std::string errorClass(const std::string &what) {
    if (what == "Unsupported protocol version !!!")
        return "unsupported-revision";
    if (what.find("entity too large") != std::string::npos) return "limit";
    return "malformed";
}

/** Applies the options of a request; absent options keep the defaults. */
void configure(const Json_t &options) {
    FRPC::LibConfig_t *config = FRPC::LibConfig_t::getInstance();
    config->setDatetimeValidationPolicy(
            options.value("datetime-validation", "on") == "on");
    config->setStringValidationPolicy(
            options.value("string-validation", "off") == "on");
}

Json_t info() {
    Json_t onOff = {"on", "off"};
    return {{"name", "libfastrpc"},
            {"version", LIBFASTRPC_VERSION},
            {"decode", REVISIONS},
            {"encode", REVISIONS},
            {"options", {{"datetime-validation", onOff},
                         {"string-validation", onOff}}}};
}

Json_t handle(const std::string &line) {
    Json_t answer = Json_t::object();
    try {
        Json_t request = Json_t::parse(line);
        const std::string op = request.at("op").get<std::string>();
        if (op == "info") return info();
        answer["id"] = request.at("id");
        configure(request.value("options", Json_t::object()));

        if (op == "decode") {
            answer["value"] =
                decode(fromHex(request.at("hex").get<std::string>()));
        } else if (op == "encode") {
            answer["hex"] = encode(request.at("value"),
                                   request.at("revision").get<std::string>());
        } else if (op == "relay") {
            Json_t message =
                decode(fromHex(request.at("hex").get<std::string>()));
            answer["hex"] = encode(message,
                                   request.at("to").get<std::string>());
        } else {
            throw BadRequest_t("unknown op " + op);
        }
    } catch (const FRPC::EncodingError_t &e) {
        answer["error"] = "invalid-utf8";
        answer["detail"] = e.what();
    } catch (const FRPC::StreamError_t &e) {
        answer["error"] = errorClass(e.what());
        answer["detail"] = e.what();
    } catch (const Unrepresentable_t &e) {
        answer["error"] = "unrepresentable";
        answer["detail"] = e.what();
    } catch (const std::exception &e) {
        answer["error"] = errorClass(e.what());
        answer["detail"] = e.what();
    }
    return answer;
}

} // namespace

int main() {
    std::string line;
    while (std::getline(std::cin, line)) {
        // error texts of the library may contain any octets
        std::cout << handle(line).dump(-1, ' ', false,
                                       Json_t::error_handler_t::replace)
                  << std::endl;
    }
    return 0;
}
