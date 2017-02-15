/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/json_document_tree.hpp"
#include "orcus/json_parser.hpp"
#include "orcus/pstring.hpp"
#include "orcus/global.hpp"
#include "orcus/config.hpp"
#include "orcus/stream.hpp"
#include "orcus/string_pool.hpp"

#include "json_util.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <limits>
#include <iostream>

#include <boost/current_function.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace orcus { namespace json {

namespace detail {

enum class node_t : int
{
    unset = json::node_t::unset,
    string = json::node_t::string,
    number = json::node_t::number,
    object = json::node_t::object,
    array = json::node_t::array,
    boolean_true = json::node_t::boolean_true,
    boolean_false = json::node_t::boolean_false,
    null = json::node_t::null,

    // internal only.
    key_value = 10,
};

}

document_error::document_error(const std::string& msg) :
    general_error("json::document_error", msg) {}

document_error::~document_error() throw() {}

struct json_value
{
    detail::node_t type;
    json_value* parent;

    json_value() : type(detail::node_t::unset), parent(nullptr) {}
    json_value(detail::node_t _type) : type(_type), parent(nullptr) {}
    virtual ~json_value() {}
};

namespace {

const char* tab = "    ";
const char quote = '"';

const xmlns_id_t NS_orcus_json_xml = "http://schemas.kohei.us/orcus/2015/json";

struct json_value_string : public json_value
{
    pstring value_string;

    json_value_string() : json_value(detail::node_t::string) {}
    json_value_string(const pstring& s) : json_value(detail::node_t::string), value_string(s) {}
    virtual ~json_value_string() {}
};

struct json_value_number : public json_value
{
    double value_number;

    json_value_number() :
        json_value(detail::node_t::number),
        value_number(std::numeric_limits<double>::quiet_NaN()) {}

    json_value_number(double num) : json_value(detail::node_t::number), value_number(num) {}

    virtual ~json_value_number() {}
};

struct json_value_array : public json_value
{
    std::vector<std::unique_ptr<json_value>> value_array;

    json_value_array() : json_value(detail::node_t::array) {}
    virtual ~json_value_array() {}
};

struct json_value_object : public json_value
{
    using object_type = std::unordered_map<pstring, std::unique_ptr<json_value>, pstring::hash>;

    std::vector<pstring> key_order;
    object_type value_object;

    bool has_ref;

    json_value_object() : json_value(detail::node_t::object), has_ref(false) {}
    virtual ~json_value_object() {}

    void swap(json_value_object& src)
    {
        key_order.swap(src.key_order);
        value_object.swap(src.value_object);
    }
};

/**
 * Value that represents a single key-value pair. Only to be used in the
 * initializer.
 */
struct json_value_kvp : public json_value
{
    pstring key;
    std::unique_ptr<json_value> value;

    json_value_kvp(const pstring& _key, std::unique_ptr<json_value>&& _value) :
        json_value(detail::node_t::key_value), key(_key), value(std::move(_value)) {}
};

void dump_repeat(std::ostringstream& os, const char* s, int repeat)
{
    for (int i = 0; i < repeat; ++i)
        os << s;
}

void dump_item(
    std::ostringstream& os, const pstring* key, const json_value* val,
    int level, bool sep);

void dump_value(std::ostringstream& os, const json_value* v, int level, const pstring* key = nullptr)
{
    dump_repeat(os, tab, level);

    if (key)
        os << quote << *key << quote << ": ";

    switch (v->type)
    {
        case detail::node_t::array:
        {
            auto& vals = static_cast<const json_value_array*>(v)->value_array;
            os << "[" << std::endl;
            size_t n = vals.size();
            size_t pos = 0;
            for (auto it = vals.begin(), ite = vals.end(); it != ite; ++it, ++pos)
                dump_item(os, nullptr, it->get(), level, pos < (n-1));

            dump_repeat(os, tab, level);
            os << "]";
        }
        break;
        case detail::node_t::boolean_false:
            os << "false";
        break;
        case detail::node_t::boolean_true:
            os << "true";
        break;
        case detail::node_t::null:
            os << "null";
        break;
        case detail::node_t::number:
            os << static_cast<const json_value_number*>(v)->value_number;
        break;
        case detail::node_t::object:
        {
            const std::vector<pstring>& key_order = static_cast<const json_value_object*>(v)->key_order;
            auto& vals = static_cast<const json_value_object*>(v)->value_object;
            os << "{" << std::endl;
            size_t n = vals.size();

            if (key_order.empty())
            {
                // Dump object's children unordered.
                size_t pos = 0;
                for (auto it = vals.begin(), ite = vals.end(); it != ite; ++it, ++pos)
                {
                    const pstring& key = it->first;
                    auto& val = it->second;

                    dump_item(os, &key, val.get(), level, pos < (n-1));
                }
            }
            else
            {
                // Dump them based on key's original ordering.
                size_t pos = 0;
                for (auto it = key_order.begin(), ite = key_order.end(); it != ite; ++it, ++pos)
                {
                    const pstring& key = *it;
                    auto val_pos = vals.find(key);
                    assert(val_pos != vals.end());

                    dump_item(os, &key, val_pos->second.get(), level, pos < (n-1));
                }
            }

            dump_repeat(os, tab, level);
            os << "}";
        }
        break;
        case detail::node_t::string:
            json::dump_string(os, static_cast<const json_value_string*>(v)->value_string.str());
        break;
        case detail::node_t::unset:
        default:
            ;
    }
}

void dump_item(
    std::ostringstream& os, const pstring* key, const json_value* val,
    int level, bool sep)
{
    dump_value(os, val, level+1, key);
    if (sep)
        os << ",";
    os << std::endl;
}

std::string dump_json_tree(const json_value* root)
{
    if (root->type == detail::node_t::unset)
        return std::string();

    std::ostringstream os;
    dump_value(os, root, 0);
    return os.str();
}

void dump_string_xml(std::ostringstream& os, const pstring& s)
{
    const char* p = s.get();
    const char* p_end = p + s.size();
    for (; p != p_end; ++p)
    {
        char c = *p;
        switch (c)
        {
            case '"':
                os << "&quot;";
            break;
            case '<':
                os << "&lt;";
            break;
            case '>':
                os << "&gt;";
            break;
            case '&':
                os << "&amp;";
            break;
            case '\'':
                os << "&apos;";
            break;
            default:
                os << c;
        }
    }
}

void dump_object_item_xml(
    std::ostringstream& os, const pstring& key, const json_value* val, int level);

void dump_value_xml(std::ostringstream& os, const json_value* v, int level)
{
    switch (v->type)
    {
        case detail::node_t::array:
        {
            os << "<array";
            if (level == 0)
                os << " xmlns=\"" << NS_orcus_json_xml << "\"";
            os << ">";

            auto& vals = static_cast<const json_value_array*>(v)->value_array;

            for (auto it = vals.begin(), ite = vals.end(); it != ite; ++it)
            {
                os << "<item>";
                dump_value_xml(os, it->get(), level+1);
                os << "</item>";
            }

            os << "</array>";
        }
        break;
        case detail::node_t::boolean_false:
            os << "<false/>";
        break;
        case detail::node_t::boolean_true:
            os << "<true/>";
        break;
        case detail::node_t::null:
            os << "<null/>";
        break;
        case detail::node_t::number:
            os << "<number value=\"";
            os << static_cast<const json_value_number*>(v)->value_number;
            os << "\"/>";
        break;
        case detail::node_t::object:
        {
            os << "<object";
            if (level == 0)
                os << " xmlns=\"" << NS_orcus_json_xml << "\"";
            os << ">";

            auto& key_order = static_cast<const json_value_object*>(v)->key_order;
            auto& vals = static_cast<const json_value_object*>(v)->value_object;

            if (key_order.empty())
            {
                // Dump object's children unordered.
                for (auto it = vals.begin(), ite = vals.end(); it != ite; ++it)
                {
                    auto& key = it->first;
                    auto& val = it->second;
                    dump_object_item_xml(os, key, val.get(), level);
                }
            }
            else
            {
                // Dump them based on key's original ordering.
                for (auto it = key_order.begin(), ite = key_order.end(); it != ite; ++it)
                {
                    auto& key = *it;
                    auto val_pos = vals.find(key);
                    assert(val_pos != vals.end());

                    dump_object_item_xml(os, key, val_pos->second.get(), level);
                }
            }

            os << "</object>";
        }
        break;
        case detail::node_t::string:
            os << "<string value=\"";
            dump_string_xml(os, static_cast<const json_value_string*>(v)->value_string);
            os << "\"/>";
        break;
        case detail::node_t::unset:
        default:
            ;
    }
}

void dump_object_item_xml(
    std::ostringstream& os, const pstring& key, const json_value* val, int level)
{
    os << "<item name=\"";
    dump_string_xml(os, key);
    os << "\">";
    dump_value_xml(os, val, level+1);
    os << "</item>";
}

std::string dump_xml_tree(const json_value* root)
{
    if (root->type == detail::node_t::unset)
        return std::string();

    std::ostringstream os;
    os << "<?xml version=\"1.0\"?>" << std::endl;
    dump_value_xml(os, root, 0);
    os << std::endl;
    return os.str();
}

struct parser_stack
{
    pstring key;
    json_value* node;

    parser_stack(json_value* _node) : node(_node) {}
};

struct external_ref
{
    pstring path;
    json_value_object* dest;

    external_ref(const pstring& _path, json_value_object* _dest) :
        path(_path), dest(_dest) {}
};

class parser_handler
{
    const json_config& m_config;

    std::unique_ptr<json_value> m_root;
    std::vector<parser_stack> m_stack;
    std::vector<external_ref> m_external_refs;

    string_pool& m_pool;

    json_value* push_value(std::unique_ptr<json_value>&& value)
    {
        assert(!m_stack.empty());
        parser_stack& cur = m_stack.back();

        switch (cur.node->type)
        {
            case detail::node_t::array:
            {
                json_value_array* jva = static_cast<json_value_array*>(cur.node);
                value->parent = jva;
                jva->value_array.push_back(std::move(value));
                return jva->value_array.back().get();
            }
            break;
            case detail::node_t::object:
            {
                const pstring& key = cur.key;
                json_value_object* jvo = static_cast<json_value_object*>(cur.node);
                value->parent = jvo;

                if (m_config.resolve_references &&
                    key == "$ref" && value->type == detail::node_t::string)
                {
                    json_value_string* jvs = static_cast<json_value_string*>(value.get());
                    if (!jvo->has_ref && !jvs->value_string.empty() && jvs->value_string[0] != '#')
                    {
                        // Store the external reference path and the destination
                        // object for later processing.
                        m_external_refs.emplace_back(jvs->value_string, jvo);
                        jvo->has_ref = true;
                    }
                }

                if (m_config.preserve_object_order)
                    jvo->key_order.push_back(key);

                auto r = jvo->value_object.insert(
                    std::make_pair(key, std::move(value)));

                if (!r.second)
                    throw document_error("adding the same key twice");

                return r.first->second.get();
            }
            break;
            default:
            {
                std::ostringstream os;
                os << BOOST_CURRENT_FUNCTION << ": unstackable JSON value type.";
                throw document_error(os.str());
            }
        }

        return nullptr;
    }

public:
    parser_handler(const json_config& config, string_pool& pool) :
        m_config(config), m_pool(pool) {}

    void begin_parse()
    {
        m_root.reset();
    }

    void end_parse()
    {
    }

    void begin_array()
    {
        if (m_root)
        {
            json_value* jv = push_value(orcus::make_unique<json_value_array>());
            assert(jv && jv->type == detail::node_t::array);
            m_stack.push_back(parser_stack(jv));
        }
        else
        {
            m_root = orcus::make_unique<json_value_array>();
            m_stack.push_back(parser_stack(m_root.get()));
        }
    }

    void end_array()
    {
        assert(!m_stack.empty());
        m_stack.pop_back();
    }

    void begin_object()
    {
        if (m_root)
        {
            json_value* jv = push_value(orcus::make_unique<json_value_object>());
            assert(jv && jv->type == detail::node_t::object);
            m_stack.push_back(parser_stack(jv));
        }
        else
        {
            m_root = orcus::make_unique<json_value_object>();
            m_stack.push_back(parser_stack(m_root.get()));
        }
    }

    void object_key(const char* p, size_t len, bool transient)
    {
        parser_stack& cur = m_stack.back();
        cur.key = pstring(p, len);
        if (m_config.persistent_string_values || transient)
            // The tree manages the life cycle of this string value.
            cur.key = m_pool.intern(cur.key).first;
    }

    void end_object()
    {
        assert(!m_stack.empty());
        m_stack.pop_back();
    }

    void boolean_true()
    {
        push_value(orcus::make_unique<json_value>(detail::node_t::boolean_true));
    }

    void boolean_false()
    {
        push_value(orcus::make_unique<json_value>(detail::node_t::boolean_false));
    }

    void null()
    {
        push_value(orcus::make_unique<json_value>(detail::node_t::null));
    }

    void string(const char* p, size_t len, bool transient)
    {
        pstring s(p, len);
        if (m_config.persistent_string_values || transient)
            // The tree manages the life cycle of this string value.
            s = m_pool.intern(s).first;

        push_value(orcus::make_unique<json_value_string>(s));
    }

    void number(double val)
    {
        push_value(orcus::make_unique<json_value_number>(val));
    }

    void swap(std::unique_ptr<json_value>& other_root)
    {
        other_root.swap(m_root);
    }

    const std::vector<external_ref>& get_external_refs() const
    {
        return m_external_refs;
    }
};

} // anonymous namespace

struct node::impl
{
    const json_value* m_node;

    impl(const json_value* jv) : m_node(jv) {}
};

node::node(const json_value* jv) : mp_impl(orcus::make_unique<impl>(jv)) {}
node::node(const node& other) : mp_impl(orcus::make_unique<impl>(other.mp_impl->m_node)) {}
node::node(node&& rhs) : mp_impl(std::move(rhs.mp_impl)) {}
node::~node() {}

node& node::operator=(const node& other)
{
    if (this == &other)
        return *this;

    node tmp(other);
    mp_impl.swap(tmp.mp_impl);
    return *this;
}

uintptr_t node::identity() const
{
    return reinterpret_cast<uintptr_t>(mp_impl->m_node);
}

node_t node::type() const
{
    // Convert it back to the public enum type.
    return static_cast<node_t>(mp_impl->m_node->type);
}

size_t node::child_count() const
{
    switch (mp_impl->m_node->type)
    {
        case detail::node_t::object:
            return static_cast<const json_value_object*>(mp_impl->m_node)->value_object.size();
        case detail::node_t::array:
            return static_cast<const json_value_array*>(mp_impl->m_node)->value_array.size();
        case detail::node_t::string:
        case detail::node_t::number:
        case detail::node_t::boolean_true:
        case detail::node_t::boolean_false:
        case detail::node_t::null:
        case detail::node_t::unset:
        default:
            ;
    }
    return 0;
}

std::vector<pstring> node::keys() const
{
    if (mp_impl->m_node->type != detail::node_t::object)
        throw document_error("node::keys: this node is not of object type.");

    const json_value_object* jvo = static_cast<const json_value_object*>(mp_impl->m_node);
    if (!jvo->key_order.empty())
        // Prefer to use key_order when it's populated.
        return jvo->key_order;

    std::vector<pstring> keys;
    std::for_each(jvo->value_object.begin(), jvo->value_object.end(),
        [&](const json_value_object::object_type::value_type& node)
        {
            keys.push_back(node.first);
        }
    );

    return keys;
}

pstring node::key(size_t index) const
{
    if (mp_impl->m_node->type != detail::node_t::object)
        throw document_error("node::key: this node is not of object type.");

    const json_value_object* jvo = static_cast<const json_value_object*>(mp_impl->m_node);
    if (index >= jvo->key_order.size())
        throw std::out_of_range("node::key: index is out-of-range.");

    return jvo->key_order[index];
}

node node::child(size_t index) const
{
    switch (mp_impl->m_node->type)
    {
        case detail::node_t::object:
        {
            // This works only when the key order is preserved.
            const json_value_object* jvo = static_cast<const json_value_object*>(mp_impl->m_node);
            if (index >= jvo->key_order.size())
                throw std::out_of_range("node::child: index is out-of-range");

            const pstring& key = jvo->key_order[index];
            auto it = jvo->value_object.find(key);
            assert(it != jvo->value_object.end());
            return node(it->second.get());
        }
        break;
        case detail::node_t::array:
        {
            const json_value_array* jva = static_cast<const json_value_array*>(mp_impl->m_node);
            if (index >= jva->value_array.size())
                throw std::out_of_range("node::child: index is out-of-range");

            return node(jva->value_array[index].get());
        }
        break;
        case detail::node_t::string:
        case detail::node_t::number:
        case detail::node_t::boolean_true:
        case detail::node_t::boolean_false:
        case detail::node_t::null:
        case detail::node_t::unset:
        default:
            throw document_error("node::child: this node cannot have child nodes.");
    }
}

node node::child(const pstring& key) const
{
    if (mp_impl->m_node->type != detail::node_t::object)
        throw document_error("node::child: this node is not of object type.");

    const json_value_object* jvo = static_cast<const json_value_object*>(mp_impl->m_node);
    auto it = jvo->value_object.find(key);
    if (it == jvo->value_object.end())
    {
        std::ostringstream os;
        os << "node::child: this object does not have a key labeled '" << key << "'";
        throw document_error(os.str());
    }

    return node(it->second.get());
}

node node::parent() const
{
    if (!mp_impl->m_node->parent)
        throw document_error("node::parent: this node has no parent.");

    return node(mp_impl->m_node->parent);
}

pstring node::string_value() const
{
    if (mp_impl->m_node->type != detail::node_t::string)
        throw document_error("node::key: current node is not of string type.");

    const json_value_string* jvs = static_cast<const json_value_string*>(mp_impl->m_node);
    return jvs->value_string;
}

double node::numeric_value() const
{
    if (mp_impl->m_node->type != detail::node_t::number)
        throw document_error("node::key: current node is not of numeric type.");

    const json_value_number* jvn = static_cast<const json_value_number*>(mp_impl->m_node);
    return jvn->value_number;
}

namespace init {

struct node::impl
{
    detail::node_t m_type;

    union
    {
        double m_value_number;
        const char* m_value_string;
    };

    std::initializer_list<init::node> m_value_array;

    impl(double v) : m_type(detail::node_t::number), m_value_number(v) {}
    impl(bool b) : m_type(b ? detail::node_t::boolean_true : detail::node_t::boolean_false) {}
    impl(decltype(nullptr)) : m_type(detail::node_t::null) {}
    impl(const char* p) : m_type(detail::node_t::string), m_value_string(p) {}

    impl(std::initializer_list<init::node> vs) :
        m_type(detail::node_t::array),
        m_value_array(std::move(vs))
    {
        // If the list has two elements, and the first element is of type string,
        // we treat this as object's key-value pair.

        if (vs.size() != 2)
            return;

        const init::node& v0 = *vs.begin();
        if (v0.mp_impl->m_type == detail::node_t::string)
            m_type = detail::node_t::object;
    }
};

node::node(double v) : mp_impl(orcus::make_unique<impl>(v)) {}
node::node(bool b) : mp_impl(orcus::make_unique<impl>(b)) {}
node::node(decltype(nullptr)) : mp_impl(orcus::make_unique<impl>(nullptr)) {}
node::node(const char* p) : mp_impl(orcus::make_unique<impl>(p)) {}
node::node(std::initializer_list<init::node> vs) : mp_impl(orcus::make_unique<impl>(std::move(vs))) {}
node::node(node&& other) : mp_impl(std::move(other.mp_impl)) {}
node::~node() {}

}

namespace {

std::unique_ptr<json_value> aggregate_nodes(std::vector<std::unique_ptr<json_value>> nodes, bool object)
{
    bool preserve_object_order = true;

    if (object)
    {
        std::unique_ptr<json_value> jv = orcus::make_unique<json_value_object>();
        json_value_object* jvo = static_cast<json_value_object*>(jv.get());

        for (std::unique_ptr<json_value>& node : nodes)
        {
            if (node->type != detail::node_t::key_value)
                throw document_error("key-value pair was expected.");

            json_value_kvp& kv = static_cast<json_value_kvp&>(*node);

            if (preserve_object_order)
                jvo->key_order.push_back(kv.key);

            kv.value->parent = jvo;
            auto r = jvo->value_object.insert(
                std::make_pair(kv.key, std::move(kv.value)));

            if (!r.second)
                throw document_error("adding the same key twice");
        }

        return jv;
    }

    std::unique_ptr<json_value> jv = orcus::make_unique<json_value_array>();
    json_value_array* jva = static_cast<json_value_array*>(jv.get());

    for (std::unique_ptr<json_value>& node : nodes)
    {
        if (node->type == detail::node_t::key_value)
            throw document_error("key-value pair was not expected.");

        node->parent = jva;
        jva->value_array.push_back(std::move(node));
    }

    return jv;
};

} // anonymous namespace

struct document_tree::impl
{
    std::unique_ptr<json::json_value> m_root;
    std::unique_ptr<string_pool> m_own_pool;
    string_pool& m_pool;

    impl() : m_own_pool(orcus::make_unique<string_pool>()), m_pool(*m_own_pool) {}
    impl(string_pool& pool) : m_pool(pool) {}
};

document_tree::document_tree() : mp_impl(orcus::make_unique<impl>()) {}
document_tree::document_tree(string_pool& pool) : mp_impl(orcus::make_unique<impl>(pool)) {}

document_tree::document_tree(std::initializer_list<init::node> vs) :
    mp_impl(orcus::make_unique<impl>())
{
    using inserter_func_type = std::function<std::unique_ptr<json_value>(string_pool&,const init::node&)>;

    inserter_func_type inserter_func = [&inserter_func](string_pool& pool, const init::node& v)
    {
        std::unique_ptr<json_value> jv;

        switch (v.mp_impl->m_type)
        {
            case detail::node_t::object:
            {
                assert(v.mp_impl->m_value_array.size() == 2);
                auto it = v.mp_impl->m_value_array.begin();
                const init::node& key_node = *it;
                assert(key_node.mp_impl->m_type == detail::node_t::string);
                pstring key = pool.intern(key_node.mp_impl->m_value_string).first;
                ++it;
                std::unique_ptr<json_value> value = inserter_func(pool, *it);
                assert(++it == v.mp_impl->m_value_array.end());

                jv = orcus::make_unique<json_value_kvp>(key, std::move(value));
                break;
            }
            case detail::node_t::array:
            {
                std::vector<std::unique_ptr<json_value>> nodes;
                bool object = true;
                for (const init::node& v2 : v.mp_impl->m_value_array)
                {
                    std::unique_ptr<json_value> r = inserter_func(pool, v2);
                    if (r->type != detail::node_t::key_value)
                        object = false;
                    nodes.push_back(std::move(r));
                }

                jv = aggregate_nodes(std::move(nodes), object);
                break;
            }
            case detail::node_t::string:
            {
                pstring s = pool.intern(v.mp_impl->m_value_string).first;
                jv = orcus::make_unique<json_value_string>(s);
                break;
            }
            case detail::node_t::number:
                jv = orcus::make_unique<json_value_number>(v.mp_impl->m_value_number);
                break;
            case detail::node_t::boolean_true:
            case detail::node_t::boolean_false:
            case detail::node_t::null:
                jv = orcus::make_unique<json_value>(v.mp_impl->m_type);
                break;
            case detail::node_t::unset:
            default:
                throw document_error("unknown node type.");
        }

        return jv;
    };

    std::vector<std::unique_ptr<json_value>> nodes;
    bool object = true;
    for (const init::node& v : vs)
    {
        std::unique_ptr<json_value> r = inserter_func(mp_impl->m_pool, v);
        if (r->type != detail::node_t::key_value)
            object = false;
        nodes.push_back(std::move(r));
    }

    mp_impl->m_root = aggregate_nodes(std::move(nodes), object);
}

document_tree::~document_tree() {}

document_tree& document_tree::operator= (std::initializer_list<init::node> vs)
{
    document_tree tmp(std::move(vs));
    swap(tmp);
    return *this;
}

void document_tree::load(const std::string& strm, const json_config& config)
{
    load(strm.data(), strm.size(), config);
}

void document_tree::load(const char* p, size_t n, const json_config& config)
{
    json::parser_handler hdl(config, mp_impl->m_pool);
    json_parser<json::parser_handler> parser(p, n, hdl);
    parser.parse();
    hdl.swap(mp_impl->m_root);

    auto& external_refs = hdl.get_external_refs();

    json_config ext_config = config;
    // The stream will get destroyed after each parsing of an external json file.
    ext_config.persistent_string_values = true;

    fs::path parent_dir = config.input_path;
    parent_dir = parent_dir.parent_path();
    for (auto it = external_refs.begin(), ite = external_refs.end(); it != ite; ++it)
    {
        fs::path extfile = it->path.str();
        fs::path extpath = parent_dir;
        extpath /= extfile;

        // Get the stream content from the path.
        std::string ext_strm = load_file_content(extpath.string().c_str());

        ext_config.input_path = extpath.string();
        document_tree doc(mp_impl->m_pool);
        try
        {
            doc.load(ext_strm, ext_config);
        }
        catch (const json::parse_error& e)
        {
            std::ostringstream os;
            os << "Error while parsing " << extpath.string() << std::endl;
            os << create_parse_error_output(ext_strm, e.offset()) << std::endl;
            os << e.what();

            // Re-throw as general_error to avoid getting caught as parse
            // error by the parent caller.
            throw general_error(os.str());
        }

        json::json_value* root = doc.mp_impl->m_root.get();
        if (root->type == detail::node_t::object)
        {
            json::json_value_object* jvo_src = static_cast<json::json_value_object*>(root);
            json::json_value_object* jvo_dest = it->dest;
            if (jvo_dest->value_object.size() == 1)
            {
                // Swap with the referenced object only when the destination
                // has one child value i.e. it only has '$ref'.
                jvo_dest->swap(*jvo_src);
                jvo_dest->has_ref = false;
            }
        }
    }
}

json::node document_tree::get_document_root() const
{
    const json::json_value* p = mp_impl->m_root.get();
    if (!p)
        throw document_error("document tree is empty");

    return json::node(p);
}

std::string document_tree::dump() const
{
    if (!mp_impl->m_root)
        return std::string();

    return json::dump_json_tree(mp_impl->m_root.get());
}

std::string document_tree::dump_xml() const
{
    if (!mp_impl->m_root)
        return std::string();

    return json::dump_xml_tree(mp_impl->m_root.get());
}

void document_tree::swap(document_tree& other)
{
    std::swap(mp_impl, other.mp_impl);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
