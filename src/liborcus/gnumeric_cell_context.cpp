/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "gnumeric_token_constants.hpp"
#include "gnumeric_namespace_types.hpp"
#include "gnumeric_cell_context.hpp"
#include "gnumeric_value_format_parser.hpp"

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/measurement.hpp>

#include <fstream>
#include <algorithm>

namespace ss = orcus::spreadsheet;

namespace orcus {

gnumeric_cell_context::gnumeric_cell_context(
    session_context& session_cxt, const tokens& tokens, ss::iface::import_factory* factory) :
    xml_context_base(session_cxt, tokens),
    mp_factory(factory),
    mp_sheet(nullptr)
{
}

gnumeric_cell_context::~gnumeric_cell_context() = default;

void gnumeric_cell_context::start_element(
    xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs)
{
    push_stack(ns, name);

    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Cell:
                start_cell(attrs);
                break;
            default:
                warn_unhandled();
        }
    }
    else
        warn_unhandled();
}

bool gnumeric_cell_context::end_element(xmlns_id_t ns, xml_token_t name)
{
    if (ns == NS_gnumeric_gnm)
    {
        switch (name)
        {
            case XML_Cell:
                end_cell();
                break;
            default:
                ;
        }
    }
    return pop_stack(ns, name);
}

void gnumeric_cell_context::characters(std::string_view str, bool transient)
{
    m_chars = str;
    if (transient)
        m_chars = intern(m_chars);
}

void gnumeric_cell_context::reset(ss::iface::import_sheet* sheet)
{
    m_cell_data.reset();
    m_chars = std::string_view{};
    mp_sheet = sheet;
    m_pool.clear();
}

void gnumeric_cell_context::start_cell(const xml_token_attrs_t& attrs)
{
    m_cell_data = cell_data{};
    m_cell_data->type = cell_type_formula;
    m_format_segments.clear();

    for (const auto& attr : attrs)
    {
        switch (attr.name)
        {
            case XML_Row:
                m_cell_data->row = to_long(attr.value);
                break;
            case XML_Col:
                m_cell_data->col = to_long(attr.value);
                break;
            case XML_ValueType:
            {
                auto v = to_long(attr.value);
                m_cell_data->type = cell_type_value;
                m_cell_data->value_type = static_cast<gnumeric_value_type>(v);
                break;
            }
            case XML_ValueFormat:
            {
                auto v = attr.value;
                if (attr.transient)
                    v = m_pool.intern(v).first;

                gnumeric_value_format_parser parser(v);

                try
                {
                    parser.parse();
                }
                catch (const parse_error& e)
                {
                    std::ostringstream os;
                    os << "failed to parse a value format string: '" << v << "' (reason=" << e.what() << ")";
                    warn(os.str());
                    break;
                }

                m_format_segments = parser.pop_segments();
                break;
            }
            case XML_ExprID:
                m_cell_data->shared_formula_id = to_long(attr.value);
                m_cell_data->type = cell_type_shared_formula;
                break;
            case XML_Rows:
                m_cell_data->type = cell_type_value;
                m_cell_data->value_type = vt_array;
                m_cell_data->array_rows = to_long(attr.value);
                break;
            case XML_Cols:
                m_cell_data->type = cell_type_value;
                m_cell_data->value_type = vt_array;
                m_cell_data->array_cols = to_long(attr.value);
                break;
        }
    }
}

void gnumeric_cell_context::end_cell()
{
    if (!m_cell_data)
        return;

    if (!mp_sheet)
        return;

    ss::col_t col = m_cell_data->col;
    ss::row_t row = m_cell_data->row;

    switch (m_cell_data->type)
    {
        case cell_type_value:
        {
            if (!m_cell_data->value_type)
                break;

            switch (*m_cell_data->value_type)
            {
                case vt_boolean:
                {
                    bool val = to_bool(m_chars);
                    mp_sheet->set_bool(row, col, val);
                    break;
                }
                case vt_float:
                {
                    double val = to_double(m_chars);
                    mp_sheet->set_value(row, col, val);
                    break;
                }
                case vt_string:
                {
                    push_string(row, col);
                    break;
                }
                case vt_array:
                {
                    ss::range_t range;
                    range.first.column = col;
                    range.first.row = row;
                    range.last.column = col + m_cell_data->array_cols - 1;
                    range.last.row = row + m_cell_data->array_rows - 1;

                    ss::iface::import_array_formula* af = mp_sheet->get_array_formula();
                    if (!af)
                        break;

                    if (m_chars.empty() || m_chars[0] != '=')
                        // formula string should start with a '='
                        break;

                    af->set_range(range);
                    af->set_formula(ss::formula_grammar_t::gnumeric, m_chars.substr(1));
                    af->commit();
                    break;
                }
                default:
                {
                    std::ostringstream os;
                    os << "unhandled cell value type (" << *m_cell_data->value_type << ")";
                    warn(os.str());
                }
            }
            break;
        }
        case cell_type_formula:
        {
            ss::iface::import_formula* xformula = mp_sheet->get_formula();
            if (!xformula)
                break;

            if (m_chars.empty() || m_chars[0] != '=')
                // formula string should start with a '='
                break;

            xformula->set_position(row, col);
            xformula->set_formula(ss::formula_grammar_t::gnumeric, m_chars.substr(1));
            xformula->commit();
            break;
        }
        case cell_type_shared_formula:
        {
            ss::iface::import_formula* xformula = mp_sheet->get_formula();
            if (!xformula)
                break;

            xformula->set_position(row, col);

            if (!m_chars.empty() && m_chars[0] == '=')
                xformula->set_formula(ss::formula_grammar_t::gnumeric, m_chars.substr(1));

            xformula->set_shared_formula_index(m_cell_data->shared_formula_id);
            xformula->commit();
            break;
        }
        case cell_type_unknown:
        {
            std::ostringstream os;
            os << "cell type is unknown (row=" << row << "; col=" << col << ")";
            warn(os.str());
            break;
        }
    }

    m_cell_data.reset();
}

void gnumeric_cell_context::push_string(ss::row_t row, ss::col_t col)
{
    ss::iface::import_shared_strings* shared_strings = mp_factory->get_shared_strings();
    if (!shared_strings)
        return;

    if (m_format_segments.empty())
    {
        // unformatted text
        std::size_t sid = shared_strings->add(m_chars);
        mp_sheet->set_string(row, col, sid);
        return;
    }

    for (const auto& [start, end] : build_format_segment_ranges())
    {
        assert(start < end);

        auto t = m_chars.substr(start, end - start);

        // TODO: use segment tree to eliminate this inner loop
        for (const gnumeric_value_format_segment& vfs : m_format_segments)
        {
            if (vfs.value.empty())
                continue;

            if (start < vfs.start || vfs.end < end)
                continue;

            // we have format information
            switch (vfs.type)
            {
                case gnumeric_value_format_type::bold:
                {
                    bool v = to_bool(vfs.value);
                    shared_strings->set_segment_bold(v);
                    break;
                }
                case gnumeric_value_format_type::italic:
                {
                    bool v = to_bool(vfs.value);
                    shared_strings->set_segment_italic(v);
                    break;
                }
                case gnumeric_value_format_type::color:
                {
                    // [red]x[green]x[blue]
                    auto color = parse_gnumeric_rgb_8x(vfs.value);
                    if (color)
                        shared_strings->set_segment_font_color(255, color->red, color->green, color->blue);
                    break;
                }
                case gnumeric_value_format_type::family:
                {
                    if (!vfs.value.empty())
                        shared_strings->set_segment_font_name(vfs.value);
                    break;
                }
                case gnumeric_value_format_type::size:
                {
                    const char* p_end = nullptr;
                    double v = to_double(vfs.value, &p_end);
                    if (p_end > vfs.value.data())
                    {
                        // font size here is stored in 1024ths of a point, likely coming from Pango
                        v /= 1024.0;
                        shared_strings->set_segment_font_size(v);
                    }
                    break;
                }
                default:
                {
                    std::ostringstream os;
                    os << "unsupported format segment type (" << int(vfs.type) << ")";
                    warn(os.str());
                }
            }
        }

        shared_strings->append_segment(t);
    }

    std::size_t sid = shared_strings->commit_segments();
    mp_sheet->set_string(row, col, sid);
}

std::vector<std::pair<std::size_t, std::size_t>> gnumeric_cell_context::build_format_segment_ranges() const
{
    if (m_format_segments.empty())
        return {};

    std::vector<std::size_t> pts;
    pts.push_back(0);
    pts.push_back(m_chars.size());
    for (const auto& seg : m_format_segments)
    {
        pts.push_back(seg.start);
        pts.push_back(seg.end);
    }

    std::sort(pts.begin(), pts.end());
    auto last = std::unique(pts.begin(), pts.end());
    pts.erase(last, pts.end());
    assert(pts.size() > 2u);

    std::vector<std::pair<std::size_t, std::size_t>> ranges;

    auto it = pts.begin();
    auto start = *it;
    for (++it; it != pts.end(); ++it)
    {
        auto end = *it;
        ranges.emplace_back(start, end);
        start = end;
    }

    return ranges;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
