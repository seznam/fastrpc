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
 * FILE          $Id: frpctreebuilder.cc,v 1.6 2010-04-21 08:48:03 edois Exp $
 *
 * DESCRIPTION
 *
 * AUTHOR
 *              Miroslav Talasek <miroslav.talasek@firma.seznam.cz>
 *
 * HISTORY
 *
 */

#include <stdexcept>
#include <utility>
#include <variant>

#include "frpcpool.h"
#include "frpcnull.h"
#include "frpcsecret.h"
#include "frpctreebuilder.h"

namespace FRPC {
namespace {

struct MethodCall_t {
    std::string name;   //!< name of the method
    Array_t *args = {}; //!< arguments of the method call
};

/** Node of the secret tree.
 */
using Node_t = std::variant<
    Fault_t,
    MethodCall_t,
    std::string,
    Array_t *,
    Struct_t *
>;

/** Node of the secret tree that contains edges to other nodes.
 */
struct SecretNode_t;

/** A key in the secret tree.
 */
struct SecretKey_t {
    /** C'tor.
     */
    SecretKey_t(std::string string_value)
        : int_value(to_int(string_value)), string_value(std::move(string_value))
    {}

    /** Returns the integer value of the key.
     * The hand-written code is used because std::stoll and other are too
     * benevolent.
     */
    static int64_t to_int(const std::string_view &value) {
        int64_t result = 0;
        for (auto i = 0lu; i < value.size(); ++i) {
            switch (auto ch = value[i]) {
            case '1' ... '9':
                result = (result * 10) + ch - '0';
                break;
            case '0':
                if (i == 0)
                    break;
                [[fallthrough]];
            default:
                // not valid array index
                return -1;
            }
        }
        return result;
    }

    int64_t int_value;        //!< unique identifier of the secret
    std::string string_value; //!< name of the secret
};

/** Compare two SecretKey_t objects for equality.
 */
bool operator==(const SecretKey_t &lhs, const SecretKey_t &rhs) {
    return lhs.string_value == rhs.string_value;
}

/** Compare SecretKey_t with int64_t for equality.
 */
bool operator==(const SecretKey_t &lhs, int64_t rhs) {
    return lhs.int_value == rhs;
}

/** Edge in the secret tree.
 */
using SecretEdge_t = std::pair<SecretKey_t, std::unique_ptr<SecretNode_t>>;

/** Node of the secret tree that contains edges to other nodes.
 */
struct SecretNode_t {
    /** Add a new edge to the node if it does not exist or returns the existing
     * edge.
     */
    SecretEdge_t &add(const SecretKey_t &key) {
        if (auto *edge = find(key))
            return *edge;
        edges.emplace_back(key, nullptr);
        return edges.back();
    }

    /** Returns the edge with the given key or nullptr if it does not exist.
     */
    template <typename KeyT>
    SecretEdge_t *find(const KeyT &key) {
        for (auto &edge: edges)
            if (edge.first == key)
                return &edge;
        return nullptr;
    }

    /** Returns the string representation of the node.
     */
    std::string str() {
        std::string result;
        for (const auto &edge: edges) {
            if (!result.empty())
                result += ", ";
            result += edge.first.string_value + "->";
            if (auto *node = edge.second.get()) {
                result += node->str();
            } else {
                result += "<null>";
            }
        }
        return result;
    }

    std::vector<SecretEdge_t> edges; //!< edges to other nodes
};

/** Unescapes a JSON Pointer segment according to RFC 6901. It also handles
 * the extension for commas, which are escaped as '~2'.
 */
std::string unescape_segment(std::string_view value) {
    std::string result;
    for (auto i = 0lu; i < value.size(); ++i) {
        if (value[i] == '~') {
            if (i + 1 < value.size()) {
                switch (value[++i]) {
                case '0':
                    result += '~';
                    break;
                case '1':
                    result += '/';
                    break;
                case '2':
                    result += ',';
                    break;
                default:
                    throw std::runtime_error("Invalid escape sequence in pointer");
                }
            } else {
                throw std::runtime_error("Invalid escape sequence in pointer");
            }
        } else {
            result += value[i];
        }
    }
    return result;
}

} // namespace

TreeBuilder_t::~TreeBuilder_t() = default;

void TreeBuilder_t::buildBinary(const char* data, unsigned int size)
{
    Value_t &binary = pool.Binary(const_cast<char*>(data),size);
    if(!isMember(binary))
        if (!isFirst(binary))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildBinary(const std::string& data)
{
    Value_t &binary = pool.Binary(data);
    if(!isMember(binary))
        if (!isFirst(binary))
            throw StreamError_t("Unexpected value after end");

}

void TreeBuilder_t::buildBool(bool value)
{
    Value_t &boolean = pool.Bool(value);
    if(!isMember(boolean))
        if (!isFirst(boolean))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildNull()
{
    Value_t &nullValue = pool.Null();
    if (!isMember(nullValue))
        if (!isFirst(nullValue))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildDateTime(short year, char month, char day, char hour,
                                  char minute, char sec, char weekDay,
                                  time_t unixTime, int timeZone)
{
    Value_t &dateTime = pool.DateTime(year, month, day, hour, minute, sec,
                                      weekDay, unixTime, timeZone);
    if(!isMember(dateTime))
        if (!isFirst(dateTime))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildDouble(double value)
{
    Value_t &doubleVal = pool.Double(value);

    if(!isMember(doubleVal))
        if (!isFirst(doubleVal))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildFault(int errNumber, const char* errMsg,
                               unsigned int size)
{
    this->errNum = errNumber;
    this->errMsg.erase();
    this->errMsg.append(errMsg, size);
    retValue = nullptr;
    first = true;

}

void TreeBuilder_t::buildFault(int errNumber, const std::string& errMsg)
{
    this->errNum = errNumber;
    this->errMsg = errMsg;
    retValue = nullptr;
    first = true;

}

void TreeBuilder_t::buildInt(Int_t::value_type value)
{
    Value_t &integer = pool.Int(value);

    if(!isMember(integer))
        if (!isFirst(integer))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildMethodCall(const char* methodName, unsigned int size)
{
    this->methodName.erase();
    this->methodName.append(methodName, size);

    Value_t &array = pool.Array();

    retValue = &array;
    first = true;
    entityStorage.push_back(ValueTypeStorage_t(&array,ARRAY));

}

void TreeBuilder_t::buildMethodCall(const std::string& methodName)
{

    this->methodName = methodName;

    Value_t &array = pool.Array();

    retValue = &array;
    first = true;
    entityStorage.push_back(ValueTypeStorage_t(&array,ARRAY));

}

void TreeBuilder_t::buildMethodResponse()
{}

void TreeBuilder_t::buildString(const char* data, unsigned int size)
{
    Value_t &stringVal = pool.String(const_cast<char*>(data), size);

    if(!isMember(stringVal))
        if (!isFirst(stringVal))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildString(const std::string& data)
{

    Value_t &stringVal = pool.String(data);

    if(!isMember(stringVal))
        if (!isFirst(stringVal))
            throw StreamError_t("Unexpected value after end");
}

void TreeBuilder_t::buildStructMember(const char* memberName, unsigned int size)
{
    this->memberName.erase();
    this->memberName.append(memberName,size);
}

void TreeBuilder_t::buildStructMember(const std::string& memberName)
{
    this->memberName = memberName;

}

void TreeBuilder_t::closeArray()
{
    entityStorage.pop_back();
}

void TreeBuilder_t::closeStruct()
{
    entityStorage.pop_back();
}

void TreeBuilder_t::openArray(unsigned int numOfItems)
{
    Array_t &array = pool.Array();
    array.reserve(numOfItems);

    if(!isMember(array))
        if (!isFirst(array))
            throw StreamError_t("Unexpected value after end");

    entityStorage.push_back(ValueTypeStorage_t(&array,ARRAY));
}

void TreeBuilder_t::openStruct(unsigned int /*numOfMembers*/)
{
    Value_t &structVal = pool.Struct();

    if(!isMember(structVal))
        if (!isFirst(structVal))
            throw StreamError_t("Unexpected value after end");

    entityStorage.push_back(ValueTypeStorage_t(&structVal,STRUCT));
}

struct ExtTreeBuilder_t::Pimpl_t {
    /** C'tor.
     */
    Pimpl_t(Pool_t &pool, const std::vector<std::string> &secrets)
        : pool(pool), secret_tree(build_secret_tree(secrets))
    {}

    /** Returns a top node of the path.
     */
    template <typename TypeT>
    auto top() {
        return !path.empty()
            ? cast<TypeT>(path.back().node)
            : nullptr;
    }

    /** Returns a bottom node of the path.
     */
    template <typename TypeT>
    auto bottom() {
        return !path.empty()
            ? cast<TypeT>(path.front().node)
            : nullptr;
    }

    /** Cast the node to the specified type and returns a pointer to it.
     */
    template <typename TypeT>
    static auto *cast(Node_t &node) {
        if constexpr (std::is_pointer_v<TypeT>) {
            auto *ptr = std::get_if<TypeT>(&node);
            return ptr? *ptr: nullptr;
        } else {
            return std::get_if<TypeT>(&node);
        }
    }

    /** Returns true if the top node is secret and also returns the
     * node itself.
     */
    std::pair<bool, SecretNode_t *> match_secret() const {
        auto impl = [&] (auto &node) -> std::pair<bool, SecretNode_t *> {
            using NodeT = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<NodeT, Fault_t>) {
                return {false, nullptr};

            } else if constexpr (std::is_same_v<NodeT, MethodCall_t>) {
                auto &method_call = static_cast<const MethodCall_t &>(node);
                auto i = method_call.args->size();
                if (auto *edge = path.back().secret_node->find(i))
                    return {!edge->second, edge->second.get()};
                return {false, nullptr};

            } else if constexpr (std::is_same_v<NodeT, Array_t *>) {
                auto *array = static_cast<Array_t *>(node);
                auto i = array->size();
                if (auto *edge = path.back().secret_node->find(i))
                    return {!edge->second, edge->second.get()};
                return {false, nullptr};

            } else if constexpr (std::is_same_v<NodeT, Struct_t *>) {
                throw std::runtime_error(__PRETTY_FUNCTION__);

            } else if constexpr (std::is_same_v<NodeT, std::string>) {
                auto &name = static_cast<const std::string &>(node);
                if (auto *edge = path.back().secret_node->find(name))
                    return {!edge->second, edge->second.get()};
                return {false, nullptr};

            } else {
                static_assert(
                    std::is_same_v<NodeT, void>,
                    "Unexpected node type in ExtTreeBuilder_t"
                );
            }
        };
        if (path.empty() || !path.back().secret_node)
            return {false, nullptr};
        return std::visit(impl, path.back().node);
    }

    /** Builds an object of type TypeT with the given arguments
     * and pushes it to the path. If the top node is a secret, it will wrap the
     * created object in a SecretValue_t and store it in the list of secrets.
     */
    template <typename TypeT, typename... ArgsT>
    void build_node(ArgsT &&...args) {
        auto [is_secret, next_secret_node] = match_secret();

        auto *real_value = pool.create<TypeT>(std::forward<ArgsT>(args)...);
        Value_t *value = !is_secret
            ? static_cast<Value_t *>(real_value)
            : &pool.Secret(*real_value);

        auto build = [&] (auto &node) {
            using NodeT = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<NodeT, Fault_t>) {
                throw std::runtime_error(__PRETTY_FUNCTION__);

            } else if constexpr (std::is_same_v<NodeT, MethodCall_t>) {
                auto &method_call = static_cast<MethodCall_t &>(node);
                method_call.args->push_back(*value);

            } else if constexpr (std::is_same_v<NodeT, Array_t *>) {
                auto *array = static_cast<Array_t *>(node);
                array->push_back(*value);

            } else if constexpr (std::is_same_v<NodeT, Struct_t *>) {

            } else if constexpr (std::is_same_v<NodeT, std::string>) {
                auto name = static_cast<std::string &>(node);
                path.pop_back();
                if (auto *struct_ptr = top<Struct_t *>()) {
                    struct_ptr->append(name, *value);
                } else {
                    throw std::runtime_error(__PRETTY_FUNCTION__);
                }

            } else {
                static_assert(
                    std::is_same_v<NodeT, void>,
                    "Unexpected node type in ExtTreeBuilder_t"
                );
            }
        };
        std::visit(build, path.back().node);

        if constexpr (std::is_same_v<TypeT, Array_t>) {
            path.push_back({real_value, next_secret_node});
        } else if constexpr (std::is_same_v<TypeT, Struct_t>) {
            path.push_back({real_value, next_secret_node});
        }
    }

    /** Push a new string value to the path.
     */
    void push(std::string value) {
        path.push_back({std::move(value), path.back().secret_node});
    }

    /** Pop the top node from the path.
     */
    template <typename TypeT>
    void pop() {
        if (!top<TypeT>())
            throw std::runtime_error(__PRETTY_FUNCTION__);
        path.pop_back();
    }

    /** Clear the path.
     */
    void clear() {path.clear();}

    /** Start a new path with the given TypeT and arguments.
     */
    template <typename TypeT, typename... ArgsT>
    void start(ArgsT &&...args) {
        if (!path.empty())
            throw std::runtime_error(__PRETTY_FUNCTION__);

        if constexpr (std::is_same_v<TypeT, MethodCall_t>) {
            path.push_back({
                MethodCall_t{{std::forward<ArgsT>(args)...}, &pool.Array()},
                &secret_tree
            });

        } else if constexpr (std::is_same_v<TypeT, Fault_t>) {
            path.push_back({
                Fault_t(std::forward<ArgsT>(args)...),
                nullptr
            });

        } else {
            static_assert(
                std::is_same_v<TypeT, void>,
                "Unsupported type for start in ExtTreeBuilder_t"
            );
        }
    }

    /** Returns the current pointer as a string.
     */
    std::string current_pointer() const {
        std::string pointer;
        for (const auto &[node, _]: path) {
            if (std::get_if<Fault_t>(&node)) {
                pointer += "<fault>";

            } else if (const auto *call = std::get_if<MethodCall_t>(&node)) {
                pointer += "/" + std::to_string(call->args->size());

            } else if (const auto *array = std::get_if<Array_t *>(&node)) {
                pointer += "/" + std::to_string((*array)->size());

            } else if (std::get_if<Struct_t *>(&node)) {

            } else if (const auto *str = std::get_if<std::string>(&node)) {
                pointer += '/';
                for (auto ch: *str) {
                    switch (ch) {
                    case ',':
                        pointer += "~2";
                        break;
                    case '/':
                        pointer += "~1";
                        break;
                    case '~':
                        pointer += "~0";
                        break;
                    default:
                        pointer += ch;
                        break;
                    }
                }
            }
        }
        return pointer;
    }

protected:
    /** Returns the next key from the pointer string.
     */
    static SecretKey_t next_key(const std::string &pointer, std::size_t &pos) {
        auto next_pos = pointer.find('/', pos);
        if (next_pos == std::string::npos) {
            SecretKey_t key(unescape_segment(pointer.substr(pos)));
            pos = pointer.size();
            return key;
        }
        SecretKey_t key(unescape_segment(pointer.substr(pos, next_pos - pos)));
        pos = next_pos + 1;
        return key;
    }

    /** Builds a tree of secrets from the given vector of secret pointers.
     */
    static SecretNode_t
    build_secret_tree(const std::vector<std::string> &secrets) {
        SecretNode_t secret_tree;
        for (const auto &secret: secrets) {
            // skip invalid secrets (empty string starts with '\0')
            if (secret[0] != '/')
                continue;

            std::size_t pos = 1;
            SecretNode_t *current = &secret_tree;
            if (pos < secret.size()) {
                while (true) {
                    auto key = next_key(secret, pos);
                    auto &edge = current->add(key);
                    if (pos >= secret.size())
                        break;
                    if (!edge.second)
                        edge.second = std::make_unique<SecretNode_t>();
                    current = edge.second.get();
                }
            }
        }
        return secret_tree;
    }

    struct PNode_t {
        Node_t node;               //!< current node in the path
        SecretNode_t *secret_node; //!< secrets for sensitive data
    };

    Pool_t &pool;              //!< pool for storing values
    std::vector<PNode_t> path; //!< current path of nodes
    SecretNode_t secret_tree;  //!< tree of sensitive nodes
};

ExtTreeBuilder_t::ExtTreeBuilder_t(
    Pool_t &pool,
    const std::vector<std::string> &secrets
)
    : pimpl(std::make_unique<Pimpl_t>(pool, secrets))
{}

ExtTreeBuilder_t::~ExtTreeBuilder_t() = default;

void ExtTreeBuilder_t::buildNull() {
    pimpl->build_node<Null_t>();
}

void ExtTreeBuilder_t::buildInt(Int_t::value_type value) {
    pimpl->build_node<Int_t>(value);
}

void ExtTreeBuilder_t::buildBool(bool value) {
    pimpl->build_node<Bool_t>(value);
}

void ExtTreeBuilder_t::buildDouble(double value) {
    pimpl->build_node<Double_t>(value);
}

void ExtTreeBuilder_t::buildString(const char *data, unsigned int size) {
    pimpl->build_node<String_t>(data, size);
}

void ExtTreeBuilder_t::buildBinary(const char *data, unsigned int size) {
    pimpl->build_node<Binary_t>(data, size);
}

void ExtTreeBuilder_t::buildDateTime(
    short year, char month, char day,
    char hour, char min, char sec,
    char weekDay, time_t unixTime, int timeZone
) {
    pimpl->build_node<DateTime_t>(
        year, month, day, hour, min, sec, weekDay, unixTime, timeZone
    );
}

void ExtTreeBuilder_t::openArray(unsigned int numOfItems) {
    pimpl->build_node<Array_t>(numOfItems);
}

void ExtTreeBuilder_t::closeArray() {
    pimpl->pop<Array_t *>();
}

void ExtTreeBuilder_t::openStruct(unsigned int /*numOfMembers*/) {
    pimpl->build_node<Struct_t>();
}

void ExtTreeBuilder_t::closeStruct() {
    pimpl->pop<Struct_t *>();
}

void ExtTreeBuilder_t::buildStructMember(const char *name, unsigned int size) {
    pimpl->push(std::string(name, size));
}

void ExtTreeBuilder_t::buildFault(int code, const char *msg, unsigned int size) {
    pimpl->clear();
    pimpl->start<Fault_t>(code, std::string(msg, size));
}

void ExtTreeBuilder_t::buildMethodCall(const char *name, unsigned int size) {
    pimpl->start<MethodCall_t>(name, size);
}

void ExtTreeBuilder_t::buildMethodResponse() {
}

std::pair<std::string, Array_t *> ExtTreeBuilder_t::getMethodCall() const {
    if (auto *call = pimpl->bottom<MethodCall_t>())
        return {call->name, call->args};
    throw Error_t("unmarshalled method call not found");
}

Fault_t *ExtTreeBuilder_t::getFault() const {
    return pimpl->bottom<Fault_t>();
}

} // namespace FRPC
