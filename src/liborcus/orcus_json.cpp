/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_json.hpp"
#include "orcus/json_document_tree.hpp"
#include "orcus/config.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/global.hpp"
#include "orcus/json_parser.hpp"
#include "orcus/stream.hpp"
#include "json_map_tree.hpp"

#include <iostream>
#include <sstream>

using namespace std;

namespace orcus {

namespace {

struct json_value
{
    enum class value_type { string, numeric, boolean, null };

    value_type type;

    union
    {
        struct { const char* p; size_t n; } str;
        double numeric;
        bool boolean;

    } value;

    json_value(double v) : type(value_type::numeric)
    {
        value.numeric = v;
    }

    json_value(const char* p, size_t n) : type(value_type::string)
    {
        value.str.p = p;
        value.str.n = n;
    }

    json_value(bool v) : type(value_type::boolean)
    {
        value.boolean = v;
    }

    json_value(value_type vt) : type(vt) {}

    void commit(spreadsheet::iface::import_factory& im_factory, const cell_position_t& pos) const
    {
        spreadsheet::iface::import_sheet* sheet =
            im_factory.get_sheet(pos.sheet.data(), pos.sheet.size());

        if (!sheet)
            return;

        switch (type)
        {
            case value_type::string:
            {
                spreadsheet::iface::import_shared_strings* ss = im_factory.get_shared_strings();
                if (!ss)
                    break;

                size_t sid = ss->add(value.str.p, value.str.n);
                sheet->set_string(pos.row, pos.col, sid);
                break;
            }
            case value_type::numeric:
                sheet->set_value(pos.row, pos.col, value.numeric);
                break;
            case value_type::boolean:
                sheet->set_bool(pos.row, pos.col, value.boolean);
                break;
            case value_type::null:
                break;
        }
    }
};

class json_content_handler
{
    json_map_tree::walker m_walker;
    json_map_tree::node* mp_current_node;
    json_map_tree::range_reference_type* mp_increment_row;

    struct row_group_scope
    {
        json_map_tree::node* node;
        spreadsheet::row_t row_position;

        row_group_scope(json_map_tree::node* _node, spreadsheet::row_t _row_position) :
            node(_node), row_position(_row_position) {}
    };

    /**
     * Stack of row group nodes, used to keep track of whether or not we are
     * currently within a linked range.
     */
    std::vector<row_group_scope> m_row_group_stack;

    spreadsheet::iface::import_factory& m_im_factory;

public:
    json_content_handler(const json_map_tree& map_tree, spreadsheet::iface::import_factory& im_factory) :
        m_walker(map_tree.get_tree_walker()),
        mp_current_node(nullptr),
        mp_increment_row(nullptr),
        m_im_factory(im_factory) {}

    void begin_parse() {}
    void end_parse() {}

    void begin_array()
    {
        push_node(json_map_tree::input_node_type::array);
    }

    void end_array()
    {
        pop_node(json_map_tree::input_node_type::array);
    }

    void begin_object()
    {
        push_node(json_map_tree::input_node_type::object);
    }

    void object_key(const char* p, size_t len, bool transient)
    {
        m_walker.set_object_key(p, len);
    }

    void end_object()
    {
        pop_node(json_map_tree::input_node_type::object);
    }

    void boolean_true()
    {
        push_node(json_map_tree::input_node_type::value);
        commit_value(true);
        pop_node(json_map_tree::input_node_type::value);
    }

    void boolean_false()
    {
        push_node(json_map_tree::input_node_type::value);
        commit_value(false);
        pop_node(json_map_tree::input_node_type::value);
    }

    void null()
    {
        push_node(json_map_tree::input_node_type::value);
        commit_value(json_value::value_type::null);
        pop_node(json_map_tree::input_node_type::value);
    }

    void string(const char* p, size_t len, bool transient)
    {
        push_node(json_map_tree::input_node_type::value);
        commit_value(json_value(p, len));
        pop_node(json_map_tree::input_node_type::value);
    }

    void number(double val)
    {
        push_node(json_map_tree::input_node_type::value);
        commit_value(val);
        pop_node(json_map_tree::input_node_type::value);
    }

private:

    void push_node(json_map_tree::input_node_type nt)
    {
        if (!m_row_group_stack.empty() && mp_current_node)
        {
            if (mp_current_node->row_group && mp_increment_row == mp_current_node->row_group)
            {
                // The last closing node was a row group boundary.  Increment the row position.
                ++mp_current_node->row_group->row_position;
                mp_increment_row = nullptr;
            }
        }

        mp_current_node = m_walker.push_node(nt);

        if (mp_current_node && mp_current_node->row_group)
        {
            m_row_group_stack.emplace_back(
                mp_current_node, mp_current_node->row_group->row_position);
        }
    }

    void pop_node(json_map_tree::input_node_type nt)
    {
        spreadsheet::row_t row_start = -1;
        spreadsheet::row_t row_end = -1;
        json_map_tree::range_reference_type* fill_down_ref = nullptr;

        if (mp_current_node && mp_current_node->row_group)
        {
            // We are exiting a row group.
            assert(!m_row_group_stack.empty());
            assert(m_row_group_stack.back().node == mp_current_node);

            // Record the current row range for this level.
            row_start = m_row_group_stack.back().row_position;
            row_end = mp_current_node->row_group->row_position;

            if (row_end > row_start && m_row_group_stack.size() > 1)
            {
                // The current range is longer than 1. We need to perform fill-downs for the parent level.
                fill_down_ref = mp_current_node->row_group;

                if (fill_down_ref->row_header)
                {
                    // Account for the row header.
                    ++row_start;
                    ++row_end;
                }
            }

            m_row_group_stack.pop_back();
        }

        mp_current_node = m_walker.pop_node(nt);

        if (!m_row_group_stack.empty())
        {
            if (mp_current_node && mp_current_node->row_group)
            {
                assert(m_row_group_stack.back().node == mp_current_node);
                mp_increment_row = mp_current_node->row_group;
            }

            if (fill_down_ref)
            {
                // Perform fill-downs for all anchored fields.
                const cell_position_t& pos = fill_down_ref->pos;
                spreadsheet::iface::import_sheet* sheet =
                    m_im_factory.get_sheet(pos.sheet.data(), pos.sheet.size());

                if (sheet)
                {
                    json_map_tree::node* node = m_row_group_stack.back().node;
                    for (const json_map_tree::node* anchored_field : node->anchored_fields)
                    {
                        spreadsheet::col_t col_offset =
                            anchored_field->value.range_field_ref->column_pos;
                        sheet->fill_down_cells(
                            pos.row + row_start, pos.col + col_offset, row_end - row_start);
                    }
                }
            }
        }
    }

    void commit_value(const json_value& v)
    {
        if (!mp_current_node)
            return;

        switch (mp_current_node->type)
        {
            case json_map_tree::map_node_type::cell_ref:
            {
                // Single cell reference
                v.commit(m_im_factory, mp_current_node->value.cell_ref->pos);
                break;
            }
            case json_map_tree::map_node_type::range_field_ref:
            {
                // Range field reference.  Offset from the origin before
                // pushing the value.
                spreadsheet::col_t col_offset = mp_current_node->value.range_field_ref->column_pos;
                json_map_tree::range_reference_type* ref = mp_current_node->value.range_field_ref->ref;

                cell_position_t pos = ref->pos; // copy
                pos.col += col_offset;
                pos.row += ref->row_position;
                if (ref->row_header)
                    ++pos.row; // Account for the row header.

                v.commit(m_im_factory, pos);
                break;
            }
            default:
                ;
        }
    }
};

} // anonymous namespace

struct orcus_json::impl
{
    spreadsheet::iface::import_factory* im_factory;
    spreadsheet::sheet_t sheet_count;
    json_map_tree map_tree;

    impl(spreadsheet::iface::import_factory* _im_factory) :
        im_factory(_im_factory), sheet_count(0) {}
};

orcus_json::orcus_json(spreadsheet::iface::import_factory* im_fact) :
    mp_impl(orcus::make_unique<impl>(im_fact)) {}

orcus_json::~orcus_json() {}

void orcus_json::set_cell_link(const pstring& path, const pstring& sheet, spreadsheet::row_t row, spreadsheet::col_t col)
{
    mp_impl->map_tree.set_cell_link(path, cell_position_t(sheet, row, col));
}

void orcus_json::start_range(const pstring& sheet, spreadsheet::row_t row, spreadsheet::col_t col, bool row_header)
{
    mp_impl->map_tree.start_range(cell_position_t(sheet, row, col), row_header);
}

void orcus_json::append_field_link(const pstring& path)
{
    mp_impl->map_tree.append_field_link(path);
}

void orcus_json::set_range_row_group(const pstring& path)
{
    mp_impl->map_tree.set_range_row_group(path);
}

void orcus_json::commit_range()
{
    mp_impl->map_tree.commit_range();
}

void orcus_json::append_sheet(const pstring& name)
{
    if (name.empty())
        return;

    mp_impl->im_factory->append_sheet(mp_impl->sheet_count++, name.data(), name.size());
}

void orcus_json::read_stream(const char* p, size_t n)
{
    if (!mp_impl->im_factory)
        return;

    spreadsheet::iface::import_shared_strings* ss = mp_impl->im_factory->get_shared_strings();
    if (!ss)
        return;

    // Insert range headers first (if applicable).
    for (const auto& entry : mp_impl->map_tree.get_range_references())
    {
        const json_map_tree::range_reference_type& ref = entry.second;
        if (!ref.row_header)
            // This range does not use row header.
            continue;

        const cell_position_t& origin = ref.pos;

        spreadsheet::iface::import_sheet* sheet =
            mp_impl->im_factory->get_sheet(origin.sheet.data(), origin.sheet.size());

        if (!sheet)
            continue;

        for (const json_map_tree::range_field_reference_type* field : ref.fields)
        {
            cell_position_t pos = origin;
            pos.col += field->column_pos;
            size_t sid = ss->add(field->label.data(), field->label.size());
            sheet->set_string(pos.row, pos.col, sid);
        }
    }

    json_content_handler hdl(mp_impl->map_tree, *mp_impl->im_factory);
    json_parser<json_content_handler> parser(p, n, hdl);
    parser.parse();
}

void orcus_json::read_map_definition(const char* p, size_t n)
{
    try
    {
        // Since a typical map file will likely be very small, let's be lazy and
        // load the whole thing into a in-memory tree.
        json::document_tree map_doc;
        json_config jc;
        jc.preserve_object_order = false;
        jc.persistent_string_values = false;
        jc.resolve_references = false;

        map_doc.load(p, n, jc);
        json::const_node root = map_doc.get_document_root();

        // Create sheets first.

        if (!root.has_key("sheets"))
            throw json_structure_error("The map definition must contains 'sheets' section.");

        for (const json::const_node& node_name : root.child("sheets"))
            append_sheet(node_name.string_value());

        if (root.has_key("cells"))
        {
            // Set cell links.
            for (const json::const_node& link_node : root.child("cells"))
            {
                pstring path = link_node.child("path").string_value();
                pstring sheet = link_node.child("sheet").string_value();
                spreadsheet::row_t row = link_node.child("row").numeric_value();
                spreadsheet::col_t col = link_node.child("column").numeric_value();

                set_cell_link(path, sheet, row, col);
            }
        }

        if (root.has_key("ranges"))
        {
            // Set range links.
            for (const json::const_node& link_node : root.child("ranges"))
            {
                pstring sheet = link_node.child("sheet").string_value();
                spreadsheet::row_t row = link_node.child("row").numeric_value();
                spreadsheet::col_t col = link_node.child("column").numeric_value();

                bool row_header = link_node.has_key("row-header") && link_node.child("row-header").type() == json::node_t::boolean_true;

                start_range(sheet, row, col, row_header);

                for (const json::const_node& field_node : link_node.child("fields"))
                {
                    pstring path = field_node.child("path").string_value();
                    append_field_link(path);
                }

                for (const json::const_node& rg_node : link_node.child("row-groups"))
                {
                    pstring path = rg_node.child("path").string_value();
                    set_range_row_group(path);
                }

                commit_range();
            }
        }
    }
    catch (const json::parse_error& e)
    {
        std::ostringstream os;
        os << "Error parsing the map definition file:" << std::endl
            << std::endl
            << create_parse_error_output(pstring(p, n), e.offset()) << std::endl
            << e.what();

        throw invalid_map_error(os.str());
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
