/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/json_document_tree.hpp>
#include <orcus/json_parser.hpp>
#include <orcus/config.hpp>
#include <orcus/stream.hpp>
#include <orcus/string_pool.hpp>

#include "json_util.hpp"

#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <string_view>
#include <limits>
#include <iostream>
#include <deque>

#include <boost/current_function.hpp>
#include <boost/pool/object_pool.hpp>

#ifdef HAVE_FILESYSTEM
#include <filesystem>
namespace fs = std::filesystem;
#else
#ifdef HAVE_EXPERIMENTAL_FILESYSTEM
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;
#endif
#endif

namespace orcus { namespace json {

namespace {

struct json_value_object;
struct json_value_array;

}

struct document_resource
{
    string_pool str_pool;
    boost::object_pool<json_value> obj_pool;
    boost::object_pool<json_value_object> obj_pool_jvo;
    boost::object_pool<json_value_array> obj_pool_jva;
};

namespace detail {

enum class node_t : int
{
    // These must have the same values as their public counterparts.
    unset         = static_cast<int>(json::node_t::unset),
    string        = static_cast<int>(json::node_t::string),
    number        = static_cast<int>(json::node_t::number),
    object        = static_cast<int>(json::node_t::object),
    array         = static_cast<int>(json::node_t::array),
    boolean_true  = static_cast<int>(json::node_t::boolean_true),
    boolean_false = static_cast<int>(json::node_t::boolean_false),
    null          = static_cast<int>(json::node_t::null),

    /**
     * Value that represents a single key-value pair.  This is an internal
     * only type, and only to be used in the initializer.
     */
    key_value = 10,

    /**
     * Implicit array initialized with a {} instead of explicit array().
     */
    array_implicit = 11,
};

std::ostream& operator<< (std::ostream& os, node_t nt)
{
    static const char* unknown = "???";

    static std::vector<const char*> values =
    {
        "unset",          // 0
        "string",         // 1
        "number",         // 2
        "object",         // 3
        "array",          // 4
        "boolean_true",   // 5
        "boolean_false",  // 6
        "null",           // 7
        unknown,          // 8
        unknown,          // 9
        "key_value",      // 10
        "array_implicit", // 11
    };

    size_t nt_value = size_t(nt);

    if (nt_value < values.size())
        os << values[nt_value];
    else
        os << unknown;

    return os;
}

}

document_error::document_error(const std::string& msg) :
    general_error("json::document_error", msg) {}

document_error::~document_error() = default;

key_value_error::key_value_error(const std::string& msg) :
    document_error(msg) {}

key_value_error::~key_value_error() = default;

struct json_value final
{
    detail::node_t type;
    json_value* parent;

    union
    {
        double numeric;

        struct
        {
            const char* p;
            size_t n;

        } str;

        struct
        {
            const char* key;
            size_t n_key;
            json_value* value;

        } kvp; // key-value pair

        json_value_object* object;
        json_value_array* array;

    } value;

    json_value(const json_value&) = delete;
    json_value& operator= (const json_value&) = delete;

    json_value() : type(detail::node_t::unset), parent(nullptr) {}
    json_value(detail::node_t _type) : type(_type), parent(nullptr) {}
    ~json_value() {}
};

namespace {

const char* tab = "    ";
const char quote = '"';

const xmlns_id_t NS_orcus_json_xml = "http://schemas.kohei.us/orcus/2015/json";

struct json_value_array
{
    std::vector<json_value*> value_array;
};

struct json_value_object
{
    using object_type = std::unordered_map<std::string_view, json_value*>;

    std::vector<std::string_view> key_order;
    object_type value_object;

    bool has_ref;

    json_value_object() : has_ref(false) {}

    void swap(json_value_object& src)
    {
        key_order.swap(src.key_order);
        value_object.swap(src.value_object);
    }
};

void dump_repeat(std::ostringstream& os, const char* s, int repeat)
{
    for (int i = 0; i < repeat; ++i)
        os << s;
}

void dump_item(
    std::ostringstream& os, const std::string_view* key, const json_value* val,
    int level, bool sep);

void dump_value(std::ostringstream& os, const json_value* v, int level, const std::string_view* key = nullptr)
{
    dump_repeat(os, tab, level);

    if (key)
        os << quote << *key << quote << ": ";

    switch (v->type)
    {
        case detail::node_t::array:
        {
            auto& vals = v->value.array->value_array;
            os << "[" << std::endl;
            size_t n = vals.size();
            size_t pos = 0;
            for (auto it = vals.begin(), ite = vals.end(); it != ite; ++it, ++pos)
                dump_item(os, nullptr, *it, level, pos < (n-1));

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
            os << v->value.numeric;
        break;
        case detail::node_t::object:
        {
            const std::vector<std::string_view>& key_order = v->value.object->key_order;
            auto& vals = v->value.object->value_object;
            os << "{" << std::endl;
            size_t n = vals.size();

            if (key_order.empty())
            {
                // Dump object's children unordered.
                size_t pos = 0;
                for (auto it = vals.begin(), ite = vals.end(); it != ite; ++it, ++pos)
                {
                    std::string_view this_key = it->first;
                    auto& this_val = it->second;

                    dump_item(os, &this_key, this_val, level, pos < (n-1));
                }
            }
            else
            {
                // Dump them based on key's original ordering.
                size_t pos = 0;
                for (auto it = key_order.begin(), ite = key_order.end(); it != ite; ++it, ++pos)
                {
                    std::string_view this_key = *it;
                    auto val_pos = vals.find(this_key);
                    assert(val_pos != vals.end());

                    dump_item(os, &this_key, val_pos->second, level, pos < (n-1));
                }
            }

            dump_repeat(os, tab, level);
            os << "}";
        }
        break;
        case detail::node_t::string:
            json::dump_string(
                os,
                std::string(v->value.str.p, v->value.str.n));
        break;
        case detail::node_t::unset:
        default:
            ;
    }
}

void dump_item(
    std::ostringstream& os, const std::string_view* key, const json_value* val,
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

void dump_string_xml(std::ostringstream& os, std::string_view s)
{
    const char* p = s.data();
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
    std::ostringstream& os, std::string_view key, const json_value* val, int level);

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

            auto& vals = v->value.array->value_array;

            for (auto it = vals.begin(), ite = vals.end(); it != ite; ++it)
            {
                os << "<item>";
                dump_value_xml(os, *it, level+1);
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
            os << v->value.numeric;
            os << "\"/>";
        break;
        case detail::node_t::object:
        {
            os << "<object";
            if (level == 0)
                os << " xmlns=\"" << NS_orcus_json_xml << "\"";
            os << ">";

            const json_value_object& jvo = *v->value.object;
            auto& key_order = jvo.key_order;
            auto& vals = jvo.value_object;

            if (key_order.empty())
            {
                // Dump object's children unordered.
                for (auto it = vals.begin(), ite = vals.end(); it != ite; ++it)
                {
                    auto& key = it->first;
                    auto& val = it->second;
                    dump_object_item_xml(os, key, val, level);
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

                    dump_object_item_xml(os, key, val_pos->second, level);
                }
            }

            os << "</object>";
        }
        break;
        case detail::node_t::string:
            os << "<string value=\"";
            dump_string_xml(os, std::string_view(v->value.str.p, v->value.str.n));
            os << "\"/>";
        break;
        case detail::node_t::unset:
        default:
            ;
    }
}

class yaml_dumper
{
    enum class write_type { unspecified, indent, linebreak, value };

    std::size_t m_indent_length = 0;
    write_type m_last_write = write_type::unspecified;

    void reset()
    {
        m_indent_length = 0;
        m_last_write = write_type::unspecified;
    }

    static bool needs_quote(std::string_view s)
    {
        char c_prev = 0;

        for (char c : s)
        {
            switch (c)
            {
                case '#':
                    return true;
                case ' ':
                    if (c_prev == ':')
                        return true;
            }

            c_prev = c;
        }

        return false;
    }

public:
    std::string dump(const json_value* root)
    {
        if (!root || root->type == detail::node_t::unset)
            return std::string();

        reset();

        std::ostringstream os;
        os << "---" << std::endl;
        write_value(os, root);
        return os.str();
    }

private:
    void write_indent(std::ostringstream& os)
    {
        m_last_write = write_type::indent;

        for (std::size_t i = 0; i < m_indent_length; ++i)
            os << ' ';
    }

    void write_linebreak(std::ostringstream& os)
    {
        if (m_last_write == write_type::linebreak)
            return;

        m_last_write = write_type::linebreak;
        os << std::endl;
    }

    void write_string(std::ostringstream& os, std::string_view s)
    {
        bool quote_value = needs_quote(s);

        if (quote_value)
            os << '"';

        os << s;

        if (quote_value)
            os << '"';
    }

    void write_value(std::ostringstream& os, const json_value* v)
    {
        m_last_write = write_type::value;
        detail::node_t parent_type = v->parent ? v->parent->type : detail::node_t::unset;

        switch (v->type)
        {
            case detail::node_t::array:
            {
                if (parent_type != detail::node_t::unset)
                    write_linebreak(os);

                for (const json_value* cv : v->value.array->value_array)
                {
                    write_indent(os);
                    os << "- ";
                    m_indent_length += 2;
                    write_value(os, cv);
                    m_indent_length -= 2;
                    write_linebreak(os);
                }
                break;
            }
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
            {
                os << v->value.numeric;
                break;
            }
            case detail::node_t::object:
            {
                const json_value_object& jvo = *v->value.object;
                const std::vector<std::string_view>& key_order = jvo.key_order;
                const json_value_object::object_type& vals = jvo.value_object;

                std::deque<std::tuple<std::string_view, const json_value*>> key_values;

                if (key_order.empty())
                {
                    // Dump object's children unordered.
                    for (const auto& val : vals)
                        key_values.emplace_back(val.first, val.second);
                }
                else
                {
                    // Dump them based on key's original ordering.
                    for (std::string_view key : key_order)
                    {
                        auto val_pos = vals.find(key);
                        assert(val_pos != vals.end());
                        key_values.emplace_back(key, val_pos->second);
                    }
                }

                if (key_values.empty())
                    break;

                auto write_key_value = [this, &os](std::string_view key, const json_value* value)
                {
                    std::size_t indent_add = 2;
                    write_string(os, key);
                    os << ": ";
                    m_indent_length += indent_add;
                    write_value(os, value);
                    m_indent_length -= indent_add;
                    write_linebreak(os);
                };

                if (parent_type == detail::node_t::array)
                {
                    // Parent is an array. Continue on the "bullet" line.
                    write_key_value(std::get<0>(key_values[0]), std::get<1>(key_values[0]));
                    key_values.pop_front();
                }
                else if (parent_type != detail::node_t::unset)
                    write_linebreak(os);

                for (auto& [key, value] : key_values)
                {
                    write_indent(os);
                    write_key_value(key, value);
                }

                break;
            }
            case detail::node_t::string:
            {
                write_string(os, {v->value.str.p, v->value.str.n});
                break;
            }
            case detail::node_t::unset:
            default:
                ;
        }
    }
};

void dump_object_item_xml(
    std::ostringstream& os, std::string_view key, const json_value* val, int level)
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
    std::string_view key;
    json_value* node;

    parser_stack(json_value* _node) : node(_node) {}
};

struct external_ref
{
    std::string_view path;
    json_value_object* dest;

    external_ref(std::string_view _path, json_value_object* _dest) :
        path(_path), dest(_dest) {}
};

class parser_handler
{
    json_value* m_root;
    const json_config& m_config;

    std::vector<parser_stack> m_stack;
    std::vector<external_ref> m_external_refs;

    document_resource& m_res;

    json_value* push_value(json_value* value)
    {
        assert(!m_stack.empty());
        parser_stack& cur = m_stack.back();

        switch (cur.node->type)
        {
            case detail::node_t::array:
            {
                json_value_array* jva = cur.node->value.array;
                value->parent = cur.node;
                jva->value_array.push_back(value);
                return jva->value_array.back();
            }
            case detail::node_t::object:
            {
                std::string_view key = cur.key;
                json_value_object* jvo = cur.node->value.object;
                value->parent = cur.node;

                if (m_config.resolve_references &&
                    key == "$ref" && value->type == detail::node_t::string)
                {
                    std::string_view sv(value->value.str.p, value->value.str.n);
                    if (!jvo->has_ref && !sv.empty() && sv[0] != '#')
                    {
                        // Store the external reference path and the destination
                        // object for later processing.
                        m_external_refs.emplace_back(sv, jvo);
                        jvo->has_ref = true;
                    }
                }

                if (m_config.preserve_object_order)
                    jvo->key_order.push_back(key);

                auto r = jvo->value_object.insert(std::make_pair(key, value));

                if (!r.second)
                    throw document_error("adding the same key twice");

                return r.first->second;
            }
            default:
                break;
        }

        std::ostringstream os;
        os << BOOST_CURRENT_FUNCTION << ": unstackable JSON value type.";
        throw document_error(os.str());
    }

public:
    parser_handler(const json_config& config, document_resource& res) :
        m_root(nullptr), m_config(config), m_res(res) {}

    void begin_parse()
    {
        m_root = nullptr;
    }

    void end_parse()
    {
    }

    void begin_array()
    {
        if (m_root)
        {
            json_value* jv = m_res.obj_pool.construct(detail::node_t::array);
            jv->value.array = m_res.obj_pool_jva.construct();
            jv = push_value(jv);
            assert(jv && jv->type == detail::node_t::array);
            m_stack.push_back(parser_stack(jv));
        }
        else
        {
            m_root = m_res.obj_pool.construct(detail::node_t::array);
            m_root->value.array = m_res.obj_pool_jva.construct();
            m_stack.push_back(parser_stack(m_root));
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
            json_value* jv = m_res.obj_pool.construct(detail::node_t::object);
            jv->value.object = m_res.obj_pool_jvo.construct();
            jv = push_value(jv);
            assert(jv && jv->type == detail::node_t::object);
            m_stack.push_back(parser_stack(jv));
        }
        else
        {
            m_root = m_res.obj_pool.construct(detail::node_t::object);
            m_root->value.object = m_res.obj_pool_jvo.construct();
            m_stack.push_back(parser_stack(m_root));
        }
    }

    void object_key(std::string_view key, bool transient)
    {
        parser_stack& cur = m_stack.back();
        cur.key = key;
        if (m_config.persistent_string_values || transient)
            // The tree manages the life cycle of this string value.
            cur.key = m_res.str_pool.intern(cur.key).first;
    }

    void end_object()
    {
        assert(!m_stack.empty());
        m_stack.pop_back();
    }

    void boolean_true()
    {
        push_value(m_res.obj_pool.construct(detail::node_t::boolean_true));
    }

    void boolean_false()
    {
        push_value(m_res.obj_pool.construct(detail::node_t::boolean_false));
    }

    void null()
    {
        push_value(m_res.obj_pool.construct(detail::node_t::null));
    }

    void string(std::string_view s, bool transient)
    {
        if (m_config.persistent_string_values || transient)
            // The tree manages the life cycle of this string value.
            s = m_res.str_pool.intern(s).first;

        json_value* jv = m_res.obj_pool.construct(detail::node_t::string);
        jv->value.str.p = s.data();
        jv->value.str.n = s.size();
        push_value(jv);
    }

    void number(double val)
    {
        json_value* jv = m_res.obj_pool.construct(detail::node_t::number);
        jv->value.numeric = val;
        push_value(jv);
    }

    json_value* get_root()
    {
        return m_root;
    }

    const std::vector<external_ref>& get_external_refs() const
    {
        return m_external_refs;
    }
};

} // anonymous namespace

struct const_node::impl
{
    const document_tree* m_doc;
    json_value* m_node;

    impl(const document_tree* doc, json_value* jv) : m_doc(doc), m_node(jv) {}
    impl(const impl& other) : m_doc(other.m_doc), m_node(other.m_node) {}
};

const_node::const_node(const document_tree* doc, json_value* jv) : mp_impl(std::make_unique<impl>(doc, jv)) {}
const_node::const_node(std::unique_ptr<impl>&& p) : mp_impl(std::move(p)) {}
const_node::const_node(const const_node& other) : mp_impl(std::make_unique<impl>(*other.mp_impl)) {}
const_node::const_node(const_node&& rhs) : mp_impl(std::move(rhs.mp_impl)) {}
const_node::~const_node() {}

const_node& const_node::operator=(const const_node& other)
{
    if (this == &other)
        return *this;

    const_node tmp(other);
    mp_impl.swap(tmp.mp_impl);
    return *this;
}

const_node& const_node::operator=(const_node&& other)
{
    if (this == &other)
        return *this;

    mp_impl = std::move(other.mp_impl);
    return *this;
}

uintptr_t const_node::identity() const
{
    return reinterpret_cast<uintptr_t>(mp_impl->m_node);
}

const_node_iterator const_node::begin() const
{
    if (mp_impl->m_node->type != detail::node_t::array)
        throw document_error("const_node::begin: this method only supports array nodes.");

    return const_node_iterator(mp_impl->m_doc, *this, true);
}

const_node_iterator const_node::end() const
{
    if (mp_impl->m_node->type != detail::node_t::array)
        throw document_error("const_node::end: this method only supports array nodes.");

    return const_node_iterator(mp_impl->m_doc, *this, false);
}

node_t const_node::type() const
{
    // Convert it back to the public enum type.
    return static_cast<node_t>(mp_impl->m_node->type);
}

size_t const_node::child_count() const
{
    switch (mp_impl->m_node->type)
    {
        case detail::node_t::object:
            return mp_impl->m_node->value.object->value_object.size();
        case detail::node_t::array:
            return mp_impl->m_node->value.array->value_array.size();
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

std::vector<std::string_view> const_node::keys() const
{
    if (mp_impl->m_node->type != detail::node_t::object)
        throw document_error("node::keys: this node is not of object type.");

    const json_value_object* jvo = mp_impl->m_node->value.object;
    if (!jvo->key_order.empty())
        // Prefer to use key_order when it's populated.
        return jvo->key_order;

    std::vector<std::string_view> keys;

    for (const auto& n : jvo->value_object)
        keys.push_back(n.first);

    return keys;
}

std::string_view const_node::key(size_t index) const
{
    if (mp_impl->m_node->type != detail::node_t::object)
        throw document_error("node::key: this node is not of object type.");

    const json_value_object* jvo = mp_impl->m_node->value.object;
    if (index >= jvo->key_order.size())
        throw std::out_of_range("node::key: index is out-of-range.");

    return jvo->key_order[index];
}

bool const_node::has_key(std::string_view key) const
{
    if (mp_impl->m_node->type != detail::node_t::object)
        return false;

    const json_value_object* jvo = mp_impl->m_node->value.object;
    const json_value_object::object_type& children = jvo->value_object;

    return children.count(key) != 0;
}

const_node const_node::child(size_t index) const
{
    switch (mp_impl->m_node->type)
    {
        case detail::node_t::object:
        {
            // This works only when the key order is preserved.
            const json_value_object* jvo = mp_impl->m_node->value.object;
            if (index >= jvo->key_order.size())
                throw std::out_of_range("node::child: index is out-of-range");

            std::string_view key = jvo->key_order[index];
            auto it = jvo->value_object.find(key);
            assert(it != jvo->value_object.end());
            return const_node(mp_impl->m_doc, it->second);
        }
        break;
        case detail::node_t::array:
        {
            const json_value_array* jva = mp_impl->m_node->value.array;
            if (index >= jva->value_array.size())
                throw std::out_of_range("node::child: index is out-of-range");

            return const_node(mp_impl->m_doc, jva->value_array[index]);
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

const_node const_node::child(std::string_view key) const
{
    if (mp_impl->m_node->type != detail::node_t::object)
        throw document_error("node::child: this node is not of object type.");

    const json_value_object* jvo = mp_impl->m_node->value.object;
    auto it = jvo->value_object.find(key);
    if (it == jvo->value_object.end())
    {
        std::ostringstream os;
        os << "node::child: this object does not have a key labeled '" << key << "'";
        throw document_error(os.str());
    }

    return const_node(mp_impl->m_doc, it->second);
}

const_node const_node::parent() const
{
    if (!mp_impl->m_node->parent)
        throw document_error("node::parent: this node has no parent.");

    return const_node(mp_impl->m_doc, mp_impl->m_node->parent);
}

const_node const_node::back() const
{
    if (mp_impl->m_node->type != detail::node_t::array)
        throw document_error("const_node::child: this node is not of array type.");

    const json_value_array* jva = mp_impl->m_node->value.array;
    if (jva->value_array.empty())
        throw document_error("const_node::child: this node has no children.");

    return const_node(mp_impl->m_doc, jva->value_array.back());
}

std::string_view const_node::string_value() const
{
    if (mp_impl->m_node->type != detail::node_t::string)
        throw document_error("node::key: current node is not of string type.");

    return std::string_view(mp_impl->m_node->value.str.p, mp_impl->m_node->value.str.n);
}

double const_node::numeric_value() const
{
    if (mp_impl->m_node->type != detail::node_t::number)
        throw document_error("node::key: current node is not of numeric type.");

    return mp_impl->m_node->value.numeric;
}

node::node(const document_tree* doc, json_value* jv) : const_node(doc, jv) {}
node::node(const_node&& rhs) : const_node(std::move(rhs)) {}
node::node(const node& other) : const_node(other) {}
node::node(node&& rhs) : const_node(rhs) {}
node::~node() {}

node& node::operator=(const node& other)
{
    if (this == &other)
        return *this;

    node tmp(other);
    mp_impl.swap(tmp.mp_impl);
    return *this;
}

node& node::operator=(const detail::init::node& v)
{
    document_resource& res =
        const_cast<document_resource&>(mp_impl->m_doc->get_resource());

    v.store_to_node(res, mp_impl->m_node);

    return *this;
}

node node::operator[](std::string_view key)
{
    if (mp_impl->m_node->type != detail::node_t::object)
        throw document_error("node::operator[]: the node must be of object type.");

    json_value_object* jvo = const_cast<json_value_object*>(mp_impl->m_node->value.object);
    auto it = jvo->value_object.find(key);
    if (it == jvo->value_object.end())
    {
        // This object doesn't have the specified key. Create a new empty node
        // on the fly.
        document_resource& res =
            const_cast<document_resource&>(mp_impl->m_doc->get_resource());
        json_value* jv = res.obj_pool.construct(detail::node_t::unset);
        jv->parent = mp_impl->m_node;
        auto r = jvo->value_object.insert(std::make_pair(key, jv));
        it = r.first;
    }

    return node(mp_impl->m_doc, it->second);
}

node node::child(size_t index)
{
    const_node cn = const_node::child(index);
    return node(std::move(cn));
}

node node::child(std::string_view key)
{
    const_node cn = const_node::child(key);
    return node(std::move(cn));
}

node node::parent()
{
    const_node cn = const_node::parent();
    return node(std::move(cn));
}

node node::back()
{
    const_node cn = const_node::back();
    return node(std::move(cn));
}

void node::push_back(const detail::init::node& v)
{
    if (mp_impl->m_node->type != detail::node_t::array)
    {
        std::ostringstream os;
        os << "node::push_back: the node must be of array type, but the value of this node type is '" << mp_impl->m_node->type << "'.";
        throw document_error(os.str());
    }

    json_value_array* jva = mp_impl->m_node->value.array;
    const document_resource& res = mp_impl->m_doc->get_resource();
    jva->value_array.push_back(v.to_json_value(const_cast<document_resource&>(res)));
}

struct const_node_iterator::impl
{
    const document_tree* m_doc;
    std::vector<json_value*>::const_iterator m_pos;
    std::vector<json_value*>::const_iterator m_end;
    const_node m_current_node;

    impl() : m_doc(nullptr), m_current_node(nullptr, nullptr) {}

    impl(const impl& other) :
        m_doc(other.m_doc),
        m_pos(other.m_pos),
        m_end(other.m_end),
        m_current_node(other.m_current_node) {}

    impl(const document_tree* doc, const const_node& v, bool begin) :
        m_doc(doc), m_current_node(nullptr, nullptr)
    {
        const json_value_array* jva = v.mp_impl->m_node->value.array;
        m_pos = begin ? jva->value_array.cbegin() : jva->value_array.cend();
        m_end = jva->value_array.cend();

        if (m_pos != m_end)
            m_current_node = const_node(m_doc, *m_pos);
    }

    void update_current()
    {
        m_current_node = const_node(m_doc, m_pos == m_end ? nullptr : *m_pos);
    }
};

const_node_iterator::const_node_iterator() :
    mp_impl(std::make_unique<impl>()) {}

const_node_iterator::const_node_iterator(const const_node_iterator& other) :
    mp_impl(std::make_unique<impl>(*other.mp_impl)) {}

const_node_iterator::const_node_iterator(const document_tree* doc, const const_node& v, bool begin) :
    mp_impl(std::make_unique<impl>(doc, v, begin)) {}

const_node_iterator::~const_node_iterator() {}

const const_node& const_node_iterator::operator*() const
{
    return mp_impl->m_current_node;
}

const const_node* const_node_iterator::operator->() const
{
    return &mp_impl->m_current_node;
}

const_node_iterator& const_node_iterator::operator++()
{
    ++mp_impl->m_pos;
    mp_impl->update_current();
    return *this;
}

const_node_iterator const_node_iterator::operator++(int)
{
    const_node_iterator tmp(*this);
    ++mp_impl->m_pos;
    mp_impl->update_current();
    return tmp;
}

const_node_iterator& const_node_iterator::operator--()
{
    --mp_impl->m_pos;
    mp_impl->update_current();
    return *this;
}

const_node_iterator const_node_iterator::operator--(int)
{
    const_node_iterator tmp(*this);
    --mp_impl->m_pos;
    mp_impl->update_current();
    return tmp;
}

bool const_node_iterator::operator== (const const_node_iterator& other) const
{
    return mp_impl->m_pos == other.mp_impl->m_pos && mp_impl->m_end == other.mp_impl->m_end;
}

bool const_node_iterator::operator!= (const const_node_iterator& other) const
{
    return !operator==(other);
}

const_node_iterator& const_node_iterator::operator= (const const_node_iterator& other)
{
    mp_impl->m_doc = other.mp_impl->m_doc;
    mp_impl->m_pos = other.mp_impl->m_pos;
    mp_impl->m_end = other.mp_impl->m_end;
    mp_impl->update_current();

    return *this;
}

array::array() {}
array::array(array&& other) : m_vs(std::move(other.m_vs)) {}
array::array(std::initializer_list<detail::init::node> vs)
{
    for (const detail::init::node& v : vs)
        m_vs.push_back(std::move(const_cast<detail::init::node&>(v)));
}

array::~array() {}

object::object() {}
object::object(object&& /*other*/) {}
object::~object() {}

namespace {

json_value* aggregate_nodes(document_resource& res, std::vector<json_value*> nodes, bool object)
{
    bool preserve_object_order = true;

    if (object)
    {
        json_value* jv = res.obj_pool.construct(detail::node_t::object);
        jv->value.object = res.obj_pool_jvo.construct();
        json_value_object* jvo = jv->value.object;

        for (json_value* const_node : nodes)
        {
            if (const_node->type != detail::node_t::key_value)
                throw document_error("key-value pair was expected.");

            auto& kvp = const_node->value.kvp;

            if (preserve_object_order)
                jvo->key_order.emplace_back(kvp.key, kvp.n_key);

            kvp.value->parent = jv;
            auto r = jvo->value_object.insert(
                std::make_pair(std::string_view(kvp.key, kvp.n_key), kvp.value));

            if (!r.second)
                throw document_error("adding the same key twice");
        }

        return jv;
    }

    json_value* jv = res.obj_pool.construct(detail::node_t::array);
    jv->value.array = res.obj_pool_jva.construct();
    json_value_array* jva = jv->value.array;

    for (json_value* const_node : nodes)
    {
        if (const_node->type == detail::node_t::key_value)
            throw document_error("key-value pair was not expected.");

        const_node->parent = jv;
        jva->value_array.push_back(const_node);
    }

    return jv;
}

void aggregate_nodes_to_object(
    document_resource& res, std::vector<json_value*> nodes, json_value* parent)
{
    bool preserve_object_order = true;

    json_value_object* jvo = res.obj_pool_jvo.construct();
    parent->value.object = jvo;

    for (json_value* const_node : nodes)
    {
        if (const_node->type != detail::node_t::key_value)
            throw document_error("key-value pair was expected.");

        auto& kvp = const_node->value.kvp;

        if (preserve_object_order)
            jvo->key_order.emplace_back(kvp.key, kvp.n_key);

        kvp.value->parent = parent;
        auto r = jvo->value_object.insert(
            std::make_pair(std::string_view(kvp.key, kvp.n_key), kvp.value));

        if (!r.second)
            throw document_error("adding the same key twice");
    }
}

void aggregate_nodes_to_array(
    document_resource& res, std::vector<json_value*> nodes, json_value* parent)
{
    json_value_array* jva = res.obj_pool_jva.construct();
    parent->value.array = jva;

    for (json_value* const_node : nodes)
    {
        if (const_node->type == detail::node_t::key_value)
            throw document_error("key-value pair was not expected.");

        const_node->parent = parent;
        jva->value_array.push_back(const_node);
    }
}

#ifndef NDEBUG

/**
 * Verify that the parent pointers stored in the child objects point to
 * their real parent object.
 */
void verify_parent_pointers(const json_value* jv, bool object)
{
    if (object)
    {
        json_value_object* jvo = jv->value.object;
        for (const auto& child : jvo->value_object)
        {
            const json_value& cv = *child.second;
            assert(cv.parent == jv);
        }
    }
    else
    {
        json_value_array* jva = jv->value.array;
        for (const auto& child : jva->value_array)
        {
            const json_value& cv = *child;
            assert(cv.parent == jv);
        }
    }
}

#endif

} // anonymous namespace

namespace detail { namespace init {

struct node::impl
{
    detail::node_t m_type;

    union
    {
        double m_value_number;
        const char* m_value_string;
    };

    std::vector<detail::init::node> m_value_array;

    impl() : m_type(detail::node_t::unset) {}
    impl(double v) : m_type(detail::node_t::number), m_value_number(v) {}
    impl(int v) : m_type(detail::node_t::number), m_value_number(v) {}
    impl(bool b) : m_type(b ? detail::node_t::boolean_true : detail::node_t::boolean_false) {}
    impl(decltype(nullptr)) : m_type(detail::node_t::null) {}
    impl(const char* p) : m_type(detail::node_t::string), m_value_string(p) {}
    impl(const std::string& s) : m_type(detail::node_t::string), m_value_string(s.data()) {}

    impl(std::initializer_list<detail::init::node> vs) :
        m_type(detail::node_t::array_implicit)
    {
        for (const detail::init::node& v : vs)
            m_value_array.push_back(std::move(const_cast<detail::init::node&>(v)));

        // If the list has two elements, and the first element is of type string,
        // we treat this as object's key-value pair.

        if (m_value_array.size() != 2)
            return;

        const detail::init::node& v0 = *m_value_array.begin();
        if (v0.mp_impl->m_type == detail::node_t::string)
            m_type = detail::node_t::key_value;
    }

    impl(json::array array) :
        m_type(detail::node_t::array),
        m_value_array(std::move(array.m_vs))
    {}

    impl(json::object /*obj*/) :
        m_type(detail::node_t::object) {}
};

node::node(double v) : mp_impl(std::make_unique<impl>(v)) {}
node::node(int v) : mp_impl(std::make_unique<impl>(v)) {}
node::node(bool b) : mp_impl(std::make_unique<impl>(b)) {}
node::node(std::nullptr_t) : mp_impl(std::make_unique<impl>(nullptr)) {}
node::node(const char* p) : mp_impl(std::make_unique<impl>(p)) {}
node::node(const std::string& s) : mp_impl(std::make_unique<impl>(s)) {}
node::node(std::initializer_list<detail::init::node> vs) : mp_impl(std::make_unique<impl>(std::move(vs))) {}
node::node(json::array array) : mp_impl(std::make_unique<impl>(std::move(array))) {}
node::node(json::object obj) : mp_impl(std::make_unique<impl>(std::move(obj))) {}

node::node(node&& other) : mp_impl(std::move(other.mp_impl)) {}
node::~node() {}

json::node_t node::type() const
{
    return static_cast<json::node_t>(mp_impl->m_type);
}

json_value* node::to_json_value(document_resource& res) const
{
    json_value* jv = nullptr;

    switch (mp_impl->m_type)
    {
        case detail::node_t::key_value:
        {
            assert(mp_impl->m_value_array.size() == 2);
            auto it = mp_impl->m_value_array.begin();
            const detail::init::node& key_node = *it;
            assert(key_node.mp_impl->m_type == detail::node_t::string);
            std::string_view key = res.str_pool.intern(key_node.mp_impl->m_value_string).first;
            ++it;
            json_value* value = it->to_json_value(res);
            if (value->type == detail::node_t::key_value)
                throw key_value_error("nested key-value pairs are not allowed.");

            ++it;
            assert(it == mp_impl->m_value_array.end());

            jv = res.obj_pool.construct(mp_impl->m_type);
            jv->value.kvp.key = key.data();
            jv->value.kvp.n_key = key.size();
            jv->value.kvp.value = value;
            break;
        }
        case detail::node_t::array:
        {
            std::vector<json_value*> nodes;
            for (const detail::init::node& v2 : mp_impl->m_value_array)
            {
                json_value* r = v2.to_json_value(res);
                nodes.push_back(r);
            }

            jv = aggregate_nodes(res, std::move(nodes), false);
#ifndef NDEBUG
            verify_parent_pointers(jv, false);
#endif
            break;
        }
        case detail::node_t::array_implicit:
        {
            std::vector<json_value*> nodes;
            bool object = !mp_impl->m_value_array.empty();
            for (const detail::init::node& v2 : mp_impl->m_value_array)
            {
                json_value* r = v2.to_json_value(res);
                if (r->type != detail::node_t::key_value)
                    object = false;
                nodes.push_back(r);
            }

            jv = aggregate_nodes(res, std::move(nodes), object);
#ifndef NDEBUG
            verify_parent_pointers(jv, object);
#endif
            break;
        }
        case detail::node_t::object:
        {
            // Currently only empty object instance is allowed.
            assert(mp_impl->m_value_array.size() == 0);
            jv = res.obj_pool.construct(mp_impl->m_type);
            jv->value.object = res.obj_pool_jvo.construct();
            break;
        }
        case detail::node_t::string:
        {
            std::string_view s = res.str_pool.intern(mp_impl->m_value_string).first;
            jv = res.obj_pool.construct(mp_impl->m_type);
            jv->value.str.p = s.data();
            jv->value.str.n = s.size();
            break;
        }
        case detail::node_t::number:
            jv = res.obj_pool.construct(mp_impl->m_type);
            jv->value.numeric = mp_impl->m_value_number;
            break;
        case detail::node_t::boolean_true:
        case detail::node_t::boolean_false:
        case detail::node_t::null:
            jv = res.obj_pool.construct(mp_impl->m_type);
            break;
        case detail::node_t::unset:
        default:
        {
            std::ostringstream os;
            os << "unknown node type (type=" << int(mp_impl->m_type) << ")";
            throw document_error(os.str());
        }
    }

    return jv;
}

void node::store_to_node(document_resource& res, json_value* parent) const
{
    parent->type = mp_impl->m_type;

    switch (mp_impl->m_type)
    {
        case detail::node_t::unset:
            throw document_error("node type is unset.");
        case detail::node_t::string:
        {
            std::string_view s = res.str_pool.intern(mp_impl->m_value_string).first;
            parent->value.str.p = s.data();
            parent->value.str.n = s.size();
            break;
        }
        case detail::node_t::number:
            parent->value.numeric = mp_impl->m_value_number;
            break;
        case detail::node_t::object:
        {
            // Currently only empty object instance is allowed.
            assert(mp_impl->m_value_array.size() == 0);
            parent->value.object = res.obj_pool_jvo.construct();
            break;
        }
        case detail::node_t::array_implicit:
        {
            std::vector<json_value*> nodes;
            bool object = true;
            for (const detail::init::node& v2 : mp_impl->m_value_array)
            {
                json_value* r = v2.to_json_value(res);
                if (r->type != detail::node_t::key_value)
                    object = false;
                nodes.push_back(r);
            }

            if (object)
            {
                parent->type = detail::node_t::object;
                aggregate_nodes_to_object(res, std::move(nodes), parent);
            }
            else
            {
                parent->type = detail::node_t::array;
                aggregate_nodes_to_array(res, std::move(nodes), parent);
            }

            break;
        }
        case detail::node_t::array:
        {
            std::vector<json_value*> nodes;
            for (const detail::init::node& v2 : mp_impl->m_value_array)
            {
                json_value* r = v2.to_json_value(res);
                nodes.push_back(r);
            }

            aggregate_nodes_to_array(res, std::move(nodes), parent);
            break;
        }
        case detail::node_t::boolean_true:
        case detail::node_t::boolean_false:
        case detail::node_t::null:
            break;
        case detail::node_t::key_value:
            // fall-through
        default:
        {
            std::ostringstream os;
            os << "unknown node type (" << (int)mp_impl->m_type << ")";
            throw document_error(os.str());
        }
    }
}

}} // namespace detail::init

struct document_tree::impl
{
    json::json_value* m_root;
    std::unique_ptr<document_resource> m_own_res;
    document_resource& m_res;

    impl() : m_root(nullptr), m_own_res(std::make_unique<document_resource>()), m_res(*m_own_res) {}
    impl(document_resource& res) : m_root(nullptr), m_res(res) {}
};

const document_resource& document_tree::get_resource() const
{
    return mp_impl->m_res;
}

document_tree::document_tree() : mp_impl(std::make_unique<impl>()) {}
document_tree::document_tree(document_tree&& other) : mp_impl(std::move(other.mp_impl)) {}
document_tree::document_tree(document_resource& res) : mp_impl(std::make_unique<impl>(res)) {}

document_tree::document_tree(std::initializer_list<detail::init::node> vs) :
    mp_impl(std::make_unique<impl>())
{
    std::vector<json_value*> nodes;
    bool object = true;
    for (const detail::init::node& v : vs)
    {
        json_value* r = v.to_json_value(mp_impl->m_res);
        if (r->type != detail::node_t::key_value)
            object = false;
        nodes.push_back(r);
    }

    mp_impl->m_root = aggregate_nodes(mp_impl->m_res, std::move(nodes), object);
}

document_tree::document_tree(array vs) : mp_impl(std::make_unique<impl>())
{
    json_value_array* jva = mp_impl->m_res.obj_pool_jva.construct();
    mp_impl->m_root = mp_impl->m_res.obj_pool.construct(detail::node_t::array);
    mp_impl->m_root->value.array = jva;

    for (const detail::init::node& v : vs.m_vs)
    {
        json_value* r = v.to_json_value(mp_impl->m_res);
        jva->value_array.push_back(r);
    }
}

document_tree::document_tree(object /*obj*/) : mp_impl(std::make_unique<impl>())
{
    mp_impl->m_root = mp_impl->m_res.obj_pool.construct(detail::node_t::object);
    mp_impl->m_root->value.object = mp_impl->m_res.obj_pool_jvo.construct();
}

document_tree::~document_tree() {}

document_tree& document_tree::operator= (std::initializer_list<detail::init::node> vs)
{
    document_tree tmp(std::move(vs));
    swap(tmp);
    return *this;
}

document_tree& document_tree::operator= (array vs)
{
    document_tree tmp(std::move(vs));
    swap(tmp);
    return *this;
}

document_tree& document_tree::operator= (object obj)
{
    document_tree tmp(std::move(obj));
    swap(tmp);
    return *this;
}

void document_tree::load(std::string_view stream, const json_config& config)
{
    json::parser_handler hdl(config, mp_impl->m_res);
    json_parser<json::parser_handler> parser(stream, hdl);
    parser.parse();
    mp_impl->m_root = hdl.get_root();

    auto& external_refs = hdl.get_external_refs();

    json_config ext_config = config;
    // The stream will get destroyed after each parsing of an external json file.
    ext_config.persistent_string_values = true;

    fs::path parent_dir = config.input_path;
    parent_dir = parent_dir.parent_path();
    for (auto it = external_refs.begin(), ite = external_refs.end(); it != ite; ++it)
    {
        fs::path extfile = std::string{it->path};
        fs::path extpath = parent_dir;
        extpath /= extfile;

        // Get the stream content from the path.
        file_content ext_content(extpath.string().data());

        ext_config.input_path = extpath.string();
        document_tree doc(mp_impl->m_res);
        try
        {
            doc.load(ext_content.str(), ext_config);
        }
        catch (const parse_error& e)
        {
            std::ostringstream os;
            os << "Error while parsing " << extpath.string() << std::endl;
            os << create_parse_error_output(ext_content.str(), e.offset()) << std::endl;
            os << e.what();

            // Re-throw as general_error to avoid getting caught as parse
            // error by the parent caller.
            throw general_error(os.str());
        }

        json::json_value* root = doc.mp_impl->m_root;
        if (root->type == detail::node_t::object)
        {
            json::json_value_object* jvo_src = root->value.object;
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

json::const_node document_tree::get_document_root() const
{
    json::json_value* p = mp_impl->m_root;
    if (!p)
        throw document_error("document tree is empty");

    return json::const_node(this, p);
}

json::node document_tree::get_document_root()
{
    json::json_value* p = mp_impl->m_root;
    if (!p)
        throw document_error("document tree is empty");

    return json::node(this, p);
}

std::string document_tree::dump() const
{
    if (!mp_impl->m_root)
        return std::string();

    return json::dump_json_tree(mp_impl->m_root);
}

std::string document_tree::dump_xml() const
{
    if (!mp_impl->m_root)
        return std::string();

    return json::dump_xml_tree(mp_impl->m_root);
}

std::string document_tree::dump_yaml() const
{
    json::yaml_dumper dumper;
    return dumper.dump(mp_impl->m_root);
}

void document_tree::swap(document_tree& other)
{
    std::swap(mp_impl, other.mp_impl);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
