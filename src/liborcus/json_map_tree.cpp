/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "json_map_tree.hpp"
#include "orcus/measurement.hpp"

#include <iostream>
#include <sstream>

namespace orcus {

constexpr json_map_tree::child_position_type json_map_tree::node_child_default_position;

namespace {

void throw_path_error(const char* file, int line, std::string_view path)
{
    std::ostringstream os;
    os << file << "#" << line << ": failed to link this path '" << path << "'";
    throw json_map_tree::path_error(os.str());
}

enum class json_path_token_t { unknown, array_pos, object_key, end };

struct json_path_token_value_t
{
    json_path_token_t type = json_path_token_t::unknown;

    union
    {
        json_map_tree::child_position_type array_pos = json_map_tree::node_child_default_position;

        struct
        {
            const char* p;
            size_t n;

        } str;

    } value;

    json_path_token_value_t(json_path_token_t _type) : type(_type) {}

    json_path_token_value_t(json_map_tree::child_position_type array_pos): type(json_path_token_t::array_pos)
    {
        value.array_pos = array_pos;
    }

    json_path_token_value_t(const char* p, size_t n) : type(json_path_token_t::object_key)
    {
        value.str.p = p;
        value.str.n = n;
    }
};

std::string_view get_last_object_key(const std::vector<json_path_token_value_t>& stack)
{
    if (stack.size() < 2)
        return std::string_view{};

    auto it = stack.rbegin();
    ++it;
    const json_path_token_value_t& t2 = *it;
    if (t2.type != json_path_token_t::object_key)
        return std::string_view{};

    return std::string_view{t2.value.str.p, t2.value.str.n};
}

class json_path_parser
{
    const char* mp_cur;
    const char* mp_end;

public:

    json_path_parser(std::string_view path) :
        mp_cur(path.data()),
        mp_end(mp_cur + path.size())
    {
        assert(!path.empty());
        assert(path[0] == '$');
        ++mp_cur; // skip the first '$'.
    }

    json_path_token_value_t next()
    {
        if (mp_cur == mp_end)
            return json_path_token_t::end;

        if (*mp_cur == '[')
            return next_pos();

        return json_path_token_t::unknown;
    }

    json_path_token_value_t next_object_key()
    {
        assert(*mp_cur == '\'');
        ++mp_cur;
        const char* p_head = mp_cur;

        for (; mp_cur != mp_end && *mp_cur != '\''; ++mp_cur)
        {
            // Skip until we reach the closing quote.
        }

        if (*mp_cur != '\'')
            return json_path_token_t::unknown;

        size_t n = std::distance(p_head, mp_cur);

        ++mp_cur; // Skip the quote.
        if (*mp_cur != ']')
            return json_path_token_t::unknown;

        ++mp_cur; // Skip the ']'.

        return json_path_token_value_t(p_head, n);
    }

    json_path_token_value_t next_pos()
    {
        assert(*mp_cur == '[');
        ++mp_cur; // Skip the '['.

        if (mp_cur == mp_end)
            return json_path_token_t::unknown;

        if (*mp_cur == '\'')
            return next_object_key();

        const char* p_head = mp_cur;

        for (; mp_cur != mp_end; ++mp_cur)
        {
            if (*mp_cur != ']')
                continue;

            if (p_head == mp_cur)
            {
                // empty brackets.
                ++mp_cur;
                return json_map_tree::node_child_default_position;
            }

            const char* p_parse_ended = nullptr;
            std::size_t n = mp_cur - p_head;
            long pos = to_long({p_head, n}, &p_parse_ended);

            if (p_parse_ended != mp_cur)
                // Parsing failed.
                break;

            if (pos < 0)
                // array position cannot be negative.
                break;

            ++mp_cur; // skip the ']'.
            return pos;
        }

        return json_path_token_t::unknown;
    }
};

} // anonymous namespace

json_map_tree::path_error::path_error(const std::string& msg) :
    general_error(msg) {}

json_map_tree::cell_reference_type::cell_reference_type(const cell_position_t& _pos) :
    pos(_pos) {}

json_map_tree::range_reference_type::range_reference_type(const cell_position_t& _pos) :
    pos(_pos), row_position(0), row_header(false) {}

json_map_tree::node::node() {}
json_map_tree::node::node(node&& other) :
    type(other.type)
{
    value.children = nullptr;

    switch (type)
    {
        case map_node_type::array:
            value.children = other.value.children;
            other.value.children = nullptr;
            break;
        case map_node_type::cell_ref:
            value.cell_ref = other.value.cell_ref;
            other.value.cell_ref = nullptr;
            break;
        case map_node_type::range_field_ref:
            value.range_field_ref = other.value.range_field_ref;
            other.value.range_field_ref = nullptr;
        default:
            ;
    }

    other.type = map_node_type::unknown;

    row_group = other.row_group;
    other.row_group = nullptr;

    anchored_fields = std::move(other.anchored_fields);
}

json_map_tree::node& json_map_tree::node::get_or_create_child_node(child_position_type pos)
{
    node_children_type& children = *value.children;

    auto it = children.lower_bound(pos); // get the first position where pos <= k is true.

    if (it == children.end() || children.key_comp()(pos, it->first))
    {
        // Insert a new array child node of unspecified type at the specified position.
        it = children.insert(
            it, node_children_type::value_type(pos, node()));
    }

    assert(it->first == pos);
    return it->second;
}

json_map_tree::walker::scope::scope(node* _p) : p(_p), array_position(0) {}

json_map_tree::walker::walker(const json_map_tree& parent) : m_parent(parent) {}

json_map_tree::node* json_map_tree::walker::push_node(input_node_type nt)
{
    if (!m_unlinked_stack.empty())
    {
        // We're still in the unlinked region.
        m_unlinked_stack.push_back(nt);
        return nullptr;
    }

    if (m_stack.empty())
    {
        if (!m_parent.m_root)
        {
            // Tree is empty.
            m_unlinked_stack.push_back(nt);
            return nullptr;
        }

        node* p = m_parent.m_root.get();

        if (!is_equivalent(nt, p->type))
        {
            // Different node type.
            m_unlinked_stack.push_back(nt);
            return nullptr;
        }

        m_stack.push_back(p);
        return m_stack.back().p;
    }

    scope& cur_scope = m_stack.back();

    switch (cur_scope.p->type)
    {
        case json_map_tree::map_node_type::array:
        {
            node_children_type& node_children = *cur_scope.p->value.children;

            auto it = node_children.find(cur_scope.array_position++);
            if (it == node_children.end())
                it = node_children.find(json_map_tree::node_child_default_position);

            if (it == node_children.end())
            {
                // This array node has no children.
                m_unlinked_stack.push_back(nt);
                return nullptr;
            }

            node* p = &it->second;

            if (!is_equivalent(nt, p->type))
            {
                // Different node type.
                m_unlinked_stack.push_back(nt);
                return nullptr;
            }

            m_stack.push_back(p);
            return m_stack.back().p;
        }
        case json_map_tree::map_node_type::object:
        {
            node_children_type& node_children = *cur_scope.p->value.children;
            auto it = node_children.find(cur_scope.array_position);
            if (it == node_children.end())
            {
                // The currently specified key does not exist in this object.
                m_unlinked_stack.push_back(nt);
                return nullptr;
            }

            node* p = &it->second;

            if (!is_equivalent(nt, p->type))
            {
                // Different node type.
                m_unlinked_stack.push_back(nt);
                return nullptr;
            }

            m_stack.push_back(p);
            return m_stack.back().p;
        }
        default:
            ;
    }

    m_unlinked_stack.push_back(nt);
    return nullptr;
}

json_map_tree::node* json_map_tree::walker::pop_node(input_node_type nt)
{
    if (!m_unlinked_stack.empty())
    {
        // We're in the unlinked region.  Pop a node from the unlinked stack.
        if (m_unlinked_stack.back() != nt)
            throw general_error("Closing node is of different type than the opening node in the unlinked node stack.");

        m_unlinked_stack.pop_back();

        if (!m_unlinked_stack.empty())
            // We are still in the unlinked region.
            return nullptr;

        return m_stack.empty() ? nullptr : m_stack.back().p;
    }

    if (m_stack.empty())
        throw general_error("A node was popped while the stack was empty.");

    if (!is_equivalent(nt, m_stack.back().p->type))
        throw general_error("Closing node is of different type than the opening node in the linked node stack.");

    m_stack.pop_back();
    return m_stack.empty() ? nullptr : m_stack.back().p;
}

void json_map_tree::walker::set_object_key(const char* p, size_t n)
{
    if (!m_unlinked_stack.empty())
        return;

    if (m_stack.empty())
        return;

    scope& cur_scope = m_stack.back();
    if (cur_scope.p->type != map_node_type::object)
        return;

    std::string_view pooled = m_parent.m_str_pool.intern({p, n}).first;
    cur_scope.array_position = reinterpret_cast<child_position_type>(pooled.data());
}

json_map_tree::json_map_tree() {}
json_map_tree::~json_map_tree() {}

json_map_tree::walker json_map_tree::get_tree_walker() const
{
    return walker(*this);
}

void json_map_tree::set_cell_link(std::string_view path, const cell_position_t& pos)
{
    path_stack_type stack = get_or_create_destination_node(path);
    if (stack.node_stack.empty())
        return;

    node* p = stack.node_stack.back();
    if (p->type != map_node_type::unknown)
    {
        std::ostringstream os;
        os << "this path is not linkable: '" << path << '\'';
        throw path_error(os.str());
    }

    p->type = map_node_type::cell_ref;
    p->value.cell_ref = m_cell_ref_pool.construct(pos);

    // Ensure that this tree owns the instance of the string.
    p->value.cell_ref->pos.sheet = m_str_pool.intern(p->value.cell_ref->pos.sheet).first;
}

const json_map_tree::node* json_map_tree::get_link(std::string_view path) const
{
    return get_destination_node(path);
}

void json_map_tree::start_range(const cell_position_t& pos, bool row_header)
{
    m_current_range.pos = pos;
    m_current_range.fields.clear();
    m_current_range.row_groups.clear();
    m_current_range.row_header = row_header;
}

void json_map_tree::append_field_link(std::string_view path, std::string_view label)
{
    m_current_range.fields.emplace_back(path, label);
}

void json_map_tree::set_range_row_group(std::string_view path)
{
    m_current_range.row_groups.push_back(path);
}

void json_map_tree::commit_range()
{
    range_reference_type* ref = &get_range_reference(m_current_range.pos);
    ref->row_header = m_current_range.row_header;
    spreadsheet::col_t column_pos = 0;

    for (std::string_view path : m_current_range.row_groups)
    {
        path_stack_type stack = get_or_create_destination_node(path);
        if (stack.node_stack.empty())
            throw_path_error(__FILE__, __LINE__, path);

        stack.node_stack.back()->row_group = ref;
    }

    long unlabeled_field_count = 0;

    for (const auto& field : m_current_range.fields)
    {
        std::string_view path = field.first;
        std::string_view label = field.second;

        path_stack_type stack = get_or_create_destination_node(path);
        if (stack.node_stack.empty() || stack.node_stack.back()->type != map_node_type::unknown)
            throw_path_error(__FILE__, __LINE__, path);

        node* p = stack.node_stack.back();
        p->type = map_node_type::range_field_ref;
        p->value.range_field_ref = m_range_field_ref_pool.construct();
        p->value.range_field_ref->column_pos = column_pos++;
        p->value.range_field_ref->ref = ref;

        if (!label.empty())
        {
            // A custom label is specified.  This one takes precedence.
            p->value.range_field_ref->label = m_str_pool.intern(label).first;
        }
        else if (stack.dest_key.empty())
        {
            // This field is probably associated with an array.
            std::ostringstream os;
            os << "field " << unlabeled_field_count++;
            p->value.range_field_ref->label = m_str_pool.intern(os.str()).first;
        }
        else
            // This field is associated with an object key.  Use its key as the label.
            p->value.range_field_ref->label = m_str_pool.intern(stack.dest_key).first;

        ref->fields.push_back(p->value.range_field_ref);

        // Find the first row group node ancountered going up from the field
        // node, and anchor itself to it.
        for (auto it = stack.node_stack.rbegin(); it != stack.node_stack.rend(); ++it)
        {
            node* anchor_node = *it;
            if (anchor_node->row_group)
            {
                anchor_node->anchored_fields.push_back(p);
                break;
            }
        }
    }
}

json_map_tree::range_ref_store_type& json_map_tree::get_range_references()
{
    return m_range_refs;
}

json_map_tree::range_reference_type& json_map_tree::get_range_reference(const cell_position_t& pos)
{
    auto it = m_range_refs.lower_bound(pos);
    if (it == m_range_refs.end() || m_range_refs.key_comp()(m_current_range.pos, it->first))
    {
        // Ensure that we own the sheet name instance before storing it.
        m_current_range.pos.sheet = m_str_pool.intern(m_current_range.pos.sheet).first;

        it = m_range_refs.insert(
            it, range_ref_store_type::value_type(
                m_current_range.pos, range_reference_type(m_current_range.pos)));
    }

    return it->second;
}

const json_map_tree::node* json_map_tree::get_destination_node(std::string_view path) const
{
    if (!m_root)
        // The tree is empty.
        return nullptr;

    if (path.empty() || path[0] != '$')
        // A valid path must begin with a '$'.
        return nullptr;

    json_path_parser parser(path);
    const node* cur_node = m_root.get();

    for (json_path_token_value_t t = parser.next(); t.type != json_path_token_t::unknown; t = parser.next())
    {
        switch (t.type)
        {
            case json_path_token_t::array_pos:
            {
                if (cur_node->type != map_node_type::array)
                    return nullptr;

                auto it = cur_node->value.children->find(t.value.array_pos);
                if (it == cur_node->value.children->end())
                    return nullptr;

                cur_node = &it->second;
                break;
            }
            case json_path_token_t::object_key:
            {
                if (cur_node->type != map_node_type::object)
                    return nullptr;

                child_position_type pos = to_key_position(t.value.str.p, t.value.str.n);

                auto it = cur_node->value.children->find(pos);
                if (it == cur_node->value.children->end())
                    return nullptr;

                cur_node = &it->second;
                break;
            }
            case json_path_token_t::end:
                return cur_node;
            case json_path_token_t::unknown:
            default:
                // Something has gone wrong. Bail out.
                break;
        }
    }

    // If this code path reaches here, something has gone wrong.
    return nullptr;
}

json_map_tree::path_stack_type json_map_tree::get_or_create_destination_node(std::string_view path)
{
    path_stack_type stack;

    if (path.empty() || path[0] != '$')
        // A valid path must begin with a '$'.
        return stack;

    json_path_parser parser(path);
    json_path_token_value_t t = parser.next();

    std::vector<json_path_token_value_t> token_stack;
    token_stack.push_back(t);

    switch (t.type)
    {
        case json_path_token_t::array_pos:
        {
            // Insert or re-use an array node and its child at specified position.

            if (m_root)
            {
                if (m_root->type == map_node_type::unknown)
                {
                    m_root->type = map_node_type::array;
                    m_root->value.children = m_node_children_pool.construct();
                }

                if (m_root->type != map_node_type::array)
                    throw path_error("root node was expected to be of type array, but is not.");
            }
            else
            {
                m_root = std::make_unique<node>();
                m_root->type = map_node_type::array;
                m_root->value.children = m_node_children_pool.construct();
            }

            stack.node_stack.push_back(m_root.get());
            node* p = &stack.node_stack.back()->get_or_create_child_node(t.value.array_pos);
            stack.node_stack.push_back(p);
            break;
        }
        case json_path_token_t::object_key:
        {
            if (m_root)
            {
                if (m_root->type == map_node_type::unknown)
                {
                    m_root->type = map_node_type::object;
                    m_root->value.children = m_node_children_pool.construct();
                }

                if (m_root->type != map_node_type::object)
                    throw path_error("root node was expected to be of type array, but is not.");
            }
            else
            {
                m_root = std::make_unique<node>();
                m_root->type = map_node_type::object;
                m_root->value.children = m_node_children_pool.construct();
            }

            stack.node_stack.push_back(m_root.get());
            child_position_type pos = to_key_position(t.value.str.p, t.value.str.n);
            node* p = &stack.node_stack.back()->get_or_create_child_node(pos);
            stack.node_stack.push_back(p);
            break;
        }
        case json_path_token_t::end:
        {
            if (!m_root)
            {
                m_root = std::make_unique<node>();
                m_root->type = map_node_type::unknown;
            }

            stack.node_stack.push_back(m_root.get());
            return stack;
        }
        default:
            // Something has gone wrong. Bail out.
            stack.node_stack.clear();
            return stack;
    }

    for (t = parser.next(); t.type != json_path_token_t::unknown; t = parser.next())
    {
        token_stack.push_back(t);
        node* cur_node = stack.node_stack.back();

        switch (t.type)
        {
            case json_path_token_t::array_pos:
            {
                switch (cur_node->type)
                {
                    case map_node_type::array:
                        // Do nothing.
                        break;
                    case map_node_type::unknown:
                        // Turn this node into an array node.
                        cur_node->type = map_node_type::array;
                        cur_node->value.children = m_node_children_pool.construct();
                        break;
                    default:
                        throw_path_error(__FILE__, __LINE__, path);
                }
                node* p = &stack.node_stack.back()->get_or_create_child_node(t.value.array_pos);
                stack.node_stack.push_back(p);
                break;
            }
            case json_path_token_t::object_key:
            {
                switch (cur_node->type)
                {
                    case map_node_type::object:
                        // Do nothing.
                        break;
                    case map_node_type::unknown:
                        // Turn this node into an object node.
                        cur_node->type = map_node_type::object;
                        cur_node->value.children = m_node_children_pool.construct();
                        break;
                    default:
                        throw_path_error(__FILE__, __LINE__, path);
                }

                // For an object children, we use the memory address of a
                // pooled key string as its position.
                child_position_type pos = to_key_position(t.value.str.p, t.value.str.n);
                node* p = &stack.node_stack.back()->get_or_create_child_node(pos);
                stack.node_stack.push_back(p);
                break;
            }
            case json_path_token_t::end:
            {
                assert(token_stack.size() >= 2);
                stack.dest_key = get_last_object_key(token_stack);

                return stack;
            }
            case json_path_token_t::unknown:
            default:
                // Something has gone wrong. Bail out.
                break;
        }
    }

    // If this code path reaches here, something has gone wrong.
    stack.node_stack.clear();
    return stack;
}

json_map_tree::child_position_type json_map_tree::to_key_position(const char* p, size_t n) const
{
    std::string_view pooled_key = m_str_pool.intern({p, n}).first;
    child_position_type pos = reinterpret_cast<child_position_type>(pooled_key.data());
    return pos;
}

bool json_map_tree::is_equivalent(input_node_type input_node, map_node_type map_node)
{
    uint8_t left = (0x0F & uint8_t(input_node));
    uint8_t right = (0x0F & uint8_t(map_node));
    return left == right;
}

std::ostream& operator<< (std::ostream& os, json_map_tree::input_node_type nt)
{
    os << "(input-node-type: ";

    switch (nt)
    {
        case json_map_tree::input_node_type::array:
            os << "array";
            break;
        case json_map_tree::input_node_type::object:
            os << "object";
            break;
        case json_map_tree::input_node_type::value:
            os << "value";
            break;
        case json_map_tree::input_node_type::unknown:
            os << "unknown";
            break;
    }

    os << ')';

    return os;
}

std::ostream& operator<< (std::ostream& os, json_map_tree::map_node_type nt)
{
    os << "(map-node-type: ";

    switch (nt)
    {
        case json_map_tree::map_node_type::array:
            os << "array";
            break;
        case json_map_tree::map_node_type::cell_ref:
            os << "cell-ref";
            break;
        case json_map_tree::map_node_type::object:
            os << "object";
            break;
        case json_map_tree::map_node_type::range_field_ref:
            os << "range-field-ref";
            break;
        case json_map_tree::map_node_type::unknown:
            os << "unknown";
            break;
    }

    os << ')';

    return os;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
