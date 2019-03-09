/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/sheet.hpp"

#include "orcus/spreadsheet/styles.hpp"
#include "orcus/spreadsheet/shared_strings.hpp"
#include "orcus/spreadsheet/sheet_range.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/auto_filter.hpp"

#include "orcus/global.hpp"
#include "orcus/exception.hpp"
#include "orcus/measurement.hpp"
#include "orcus/string_pool.hpp"

#include "formula_global.hpp"
#include "json_dumper.hpp"
#include "csv_dumper.hpp"
#include "flat_dumper.hpp"
#include "html_dumper.hpp"
#include "impl_types.hpp"
#include "number_format.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <vector>
#include <cassert>
#include <cstdlib>

#include <mdds/flat_segment_tree.hpp>

#include <ixion/cell.hpp>
#include <ixion/formula.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>
#include <ixion/formula_tokens.hpp>
#include <ixion/model_context.hpp>
#include <ixion/address.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>

#define ORCUS_DEBUG_SHEET 0

using namespace std;
using namespace boost;

namespace orcus { namespace spreadsheet {

namespace {

typedef mdds::flat_segment_tree<row_t, size_t>  segment_row_index_type;
typedef std::unordered_map<col_t, std::unique_ptr<segment_row_index_type>> cell_format_type;

// Widths and heights are stored in twips.
typedef mdds::flat_segment_tree<col_t, col_width_t> col_widths_store_type;
typedef mdds::flat_segment_tree<row_t, row_height_t> row_heights_store_type;

// hidden information
typedef mdds::flat_segment_tree<col_t, bool> col_hidden_store_type;
typedef mdds::flat_segment_tree<row_t, bool> row_hidden_store_type;

}

struct sheet_impl
{
    document& m_doc;

    mutable col_widths_store_type m_col_widths;
    mutable row_heights_store_type m_row_heights;
    col_widths_store_type::const_iterator m_col_width_pos;
    row_heights_store_type::const_iterator m_row_height_pos;

    mutable col_hidden_store_type m_col_hidden;
    mutable row_hidden_store_type m_row_hidden;
    col_hidden_store_type::const_iterator m_col_hidden_pos;
    row_hidden_store_type::const_iterator m_row_hidden_pos;

    detail::col_merge_size_type m_merge_ranges; /// 2-dimensional merged cell ranges.

    std::unique_ptr<auto_filter_t> mp_auto_filter_data;

    cell_format_type m_cell_formats;
    row_t m_row_size;
    col_t m_col_size;
    const sheet_t m_sheet; /// sheet ID

    sheet_impl() = delete;
    sheet_impl(const sheet_impl&) = delete;
    sheet_impl& operator=(const sheet_impl&) = delete;

    sheet_impl(document& doc, sheet& sh, sheet_t sheet_index, row_t row_size, col_t col_size) :
        m_doc(doc),
        m_col_widths(0, col_size, get_default_column_width()),
        m_row_heights(0, row_size, get_default_row_height()),
        m_col_width_pos(m_col_widths.begin()),
        m_row_height_pos(m_row_heights.begin()),
        m_col_hidden(0, col_size, false),
        m_row_hidden(0, row_size, false),
        m_col_hidden_pos(m_col_hidden.begin()),
        m_row_hidden_pos(m_row_hidden.begin()),
        m_row_size(row_size), m_col_size(col_size), m_sheet(sheet_index) {}

    ~sheet_impl() {}

    const detail::merge_size* get_merge_size(row_t row, col_t col) const
    {
        detail::col_merge_size_type::const_iterator it_col = m_merge_ranges.find(col);
        if (it_col == m_merge_ranges.end())
            return nullptr;

        detail::merge_size_type& col_merge_sizes = *it_col->second;
        detail::merge_size_type::const_iterator it_row = col_merge_sizes.find(row);
        if (it_row == col_merge_sizes.end())
            return nullptr;

        return &it_row->second;
    }

    ixion::abs_range_t get_data_range() const
    {
        const ixion::model_context& cxt = m_doc.get_model_context();
        return cxt.get_data_range(m_sheet);
    }
};

const row_t sheet::max_row_limit = 1048575;
const col_t sheet::max_col_limit = 1023;

sheet::sheet(document& doc, sheet_t sheet_index, row_t row_size, col_t col_size) :
    mp_impl(new sheet_impl(doc, *this, sheet_index, row_size, col_size)) {}

sheet::~sheet()
{
    delete mp_impl;
}

void sheet::set_auto(row_t row, col_t col, const char* p, size_t n)
{
    if (!p || !n)
        return;

    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();

    // First, see if this can be parsed as a number.
    char* endptr = nullptr;
    double val = strtod(p, &endptr);
    const char* endptr_check = p + n;
    if (endptr == endptr_check)
        // Treat this as a numeric value.
        cxt.set_numeric_cell(ixion::abs_address_t(mp_impl->m_sheet,row,col), val);
    else
        // Treat this as a string value.
        cxt.set_string_cell(ixion::abs_address_t(mp_impl->m_sheet,row,col), p, n);
}

void sheet::set_string(row_t row, col_t col, size_t sindex)
{
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    cxt.set_string_cell(ixion::abs_address_t(mp_impl->m_sheet,row,col), sindex);

#if ORCUS_DEBUG_SHEET
    cout << "sheet::set_string: sheet=" << mp_impl->m_sheet << ", row=" << row << ", col=" << col << ", si=" << sindex << endl;
#endif
}

void sheet::set_value(row_t row, col_t col, double value)
{
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    cxt.set_numeric_cell(ixion::abs_address_t(mp_impl->m_sheet,row,col), value);
}

void sheet::set_bool(row_t row, col_t col, bool value)
{
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    cxt.set_boolean_cell(ixion::abs_address_t(mp_impl->m_sheet,row,col), value);
}

void sheet::set_date_time(row_t row, col_t col, int year, int month, int day, int hour, int minute, double second)
{
    // Convert this to a double value representing days since epoch.

    date_time_t dt_origin = mp_impl->m_doc.get_origin_date();

    gregorian::date origin(dt_origin.year, dt_origin.month, dt_origin.day);
    gregorian::date d(year, month, day);

    double days_since_epoch = (d - origin).days();

    long ms = second * 1000000.0;

    posix_time::time_duration t(
        posix_time::hours(hour) +
        posix_time::minutes(minute) +
        posix_time::microseconds(ms)
    );

    double time_as_day = t.total_microseconds();
    time_as_day /= 1000000.0; // microseconds to seconds
    time_as_day /= 60.0 * 60.0 * 24.0; // seconds to day

    set_value(row, col, days_since_epoch + time_as_day);
}

void sheet::set_format(row_t row, col_t col, size_t index)
{
    set_format(row, col, row, col, index);
}

void sheet::set_format(row_t row_start, col_t col_start, row_t row_end, col_t col_end, size_t index)
{
    for (col_t col = col_start; col <= col_end; ++col)
    {
        cell_format_type::iterator itr = mp_impl->m_cell_formats.find(col);
        if (itr == mp_impl->m_cell_formats.end())
        {
            auto p = orcus::make_unique<segment_row_index_type>(0, mp_impl->m_row_size+1, 0);

            pair<cell_format_type::iterator, bool> r =
                mp_impl->m_cell_formats.insert(cell_format_type::value_type(col, std::move(p)));

            if (!r.second)
            {
                cerr << "insertion of new cell format container failed!" << endl;
                return;
            }

            itr = r.first;
        }

        segment_row_index_type& con = *itr->second;
        con.insert_back(row_start, row_end+1, index);
    }
}

void sheet::set_formula(row_t row, col_t col, const ixion::formula_tokens_store_ptr_t& tokens)
{
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    ixion::abs_address_t pos(mp_impl->m_sheet, row, col);

    cxt.set_formula_cell(pos, tokens);
    ixion::register_formula_cell(cxt, pos);
    mp_impl->m_doc.insert_dirty_cell(pos);
}

void sheet::set_grouped_formula(const range_t range, ixion::formula_tokens_t tokens)
{
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();

    ixion::abs_range_t pos;
    pos.first.sheet  = mp_impl->m_sheet;
    pos.first.row    = range.first.row;
    pos.first.column = range.first.column;
    pos.last.sheet   = mp_impl->m_sheet;
    pos.last.row     = range.last.row;
    pos.last.column  = range.last.column;

    cxt.set_grouped_formula_cells(pos, std::move(tokens));
    ixion::register_formula_cell(cxt, pos.first);
    mp_impl->m_doc.insert_dirty_cell(pos.first);
}

void sheet::set_formula_result(row_t row, col_t col, double value)
{
}

void sheet::set_formula_result(row_t row, col_t col, const char* p, size_t n)
{
}

void sheet::set_col_width(col_t col, col_width_t width)
{
    mp_impl->m_col_width_pos =
        mp_impl->m_col_widths.insert(mp_impl->m_col_width_pos, col, col+1, width).first;
}

col_width_t sheet::get_col_width(col_t col, col_t* col_start, col_t* col_end) const
{
    col_widths_store_type& col_widths = mp_impl->m_col_widths;
    if (!col_widths.is_tree_valid())
        col_widths.build_tree();

    col_width_t ret = 0;
    if (!col_widths.search_tree(col, ret, col_start, col_end).second)
        throw orcus::general_error("sheet::get_col_width: failed to search tree.");

    return ret;
}

void sheet::set_col_hidden(col_t col, bool hidden)
{
    mp_impl->m_col_hidden_pos =
        mp_impl->m_col_hidden.insert(mp_impl->m_col_hidden_pos, col, col+1, hidden).first;
}

bool sheet::is_col_hidden(col_t col, col_t* col_start, col_t* col_end) const
{
    col_hidden_store_type& col_hidden = mp_impl->m_col_hidden;
    if (!col_hidden.is_tree_valid())
        col_hidden.build_tree();

    bool hidden = false;
    if (!col_hidden.search_tree(col, hidden, col_start, col_end).second)
        throw orcus::general_error("sheet::is_col_hidden: failed to search tree.");

    return hidden;
}

void sheet::set_row_height(row_t row, row_height_t height)
{
    mp_impl->m_row_height_pos =
        mp_impl->m_row_heights.insert(mp_impl->m_row_height_pos, row, row+1, height).first;
}

row_height_t sheet::get_row_height(row_t row, row_t* row_start, row_t* row_end) const
{
    row_heights_store_type& row_heights = mp_impl->m_row_heights;
    if (!row_heights.is_tree_valid())
        row_heights.build_tree();

    row_height_t ret = 0;
    if (!row_heights.search_tree(row, ret, row_start, row_end).second)
        throw orcus::general_error("sheet::get_row_height: failed to search tree.");

    return ret;
}

void sheet::set_row_hidden(row_t row, bool hidden)
{
    mp_impl->m_row_hidden_pos =
        mp_impl->m_row_hidden.insert(mp_impl->m_row_hidden_pos, row, row+1, hidden).first;
}

bool sheet::is_row_hidden(row_t row, row_t* row_start, row_t* row_end) const
{
    row_hidden_store_type& row_hidden = mp_impl->m_row_hidden;
    if (!row_hidden.is_tree_valid())
        row_hidden.build_tree();

    bool hidden = false;
    if (!row_hidden.search_tree(row, hidden, row_start, row_end).second)
        throw orcus::general_error("sheet::is_row_hidden: failed to search tree.");

    return hidden;
}

void sheet::set_merge_cell_range(const range_t& range)
{
    detail::col_merge_size_type::iterator it_col = mp_impl->m_merge_ranges.find(range.first.column);
    if (it_col == mp_impl->m_merge_ranges.end())
    {
        auto p = orcus::make_unique<detail::merge_size_type>();
        pair<detail::col_merge_size_type::iterator, bool> r =
            mp_impl->m_merge_ranges.insert(
                detail::col_merge_size_type::value_type(range.first.column, std::move(p)));

        if (!r.second)
            // Insertion failed.
            return;

        it_col = r.first;
    }

    detail::merge_size_type& col_data = *it_col->second;
    detail::merge_size sz(range.last.column-range.first.column+1, range.last.row-range.first.row+1);
    col_data.insert(
        detail::merge_size_type::value_type(range.first.row, sz));
}

void sheet::fill_down_cells(row_t src_row, col_t src_col, row_t range_size)
{
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    ixion::abs_address_t src_pos(mp_impl->m_sheet, src_row, src_col);
    cxt.fill_down_cells(src_pos, range_size);
}

range_t sheet::get_merge_cell_range(row_t row, col_t col) const
{
    range_t ret;
    ret.first.column = col;
    ret.first.row = row;
    ret.last.column = col;
    ret.last.row = row;

    detail::col_merge_size_type::const_iterator it_col = mp_impl->m_merge_ranges.find(col);
    if (it_col == mp_impl->m_merge_ranges.end())
        return ret; // not a merged cell

    const detail::merge_size_type& col_data = *it_col->second;
    detail::merge_size_type::const_iterator it = col_data.find(row);
    if (it == col_data.end())
        return ret; // not a merged cell

    const detail::merge_size& ms = it->second;
    ret.last.column += ms.width - 1;
    ret.last.row += ms.height - 1;

    return ret;
}

size_t sheet::get_string_identifier(row_t row, col_t col) const
{
    const ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    return cxt.get_string_identifier(ixion::abs_address_t(mp_impl->m_sheet, row, col));
}

auto_filter_t* sheet::get_auto_filter_data()
{
    return mp_impl->mp_auto_filter_data.get();
}

const auto_filter_t* sheet::get_auto_filter_data() const
{
    return mp_impl->mp_auto_filter_data.get();
}

void sheet::set_auto_filter_data(auto_filter_t* p)
{
    mp_impl->mp_auto_filter_data.reset(p);
}

ixion::abs_range_t sheet::get_data_range() const
{
    return mp_impl->get_data_range();
}

sheet_range sheet::get_sheet_range(
    row_t row_start, col_t col_start, row_t row_end, col_t col_end) const
{
    if (row_end < row_start || col_end < col_start)
    {
        std::ostringstream os;
        os << "sheet::get_sheet_range: invalid range (rows: "
            << row_start << "->" << row_end << "; columns: "
            << col_start << "->" << col_end << ")";
        throw orcus::general_error(os.str());
    }

    const ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    const ixion::column_stores_t* stores = cxt.get_columns(mp_impl->m_sheet);
    if (!stores)
        throw orcus::general_error(
            "sheet::get_sheet_range: failed to get column stores from the model.");

    return sheet_range(cxt, *stores, row_start, col_start, row_end, col_end);
}

row_t sheet::row_size() const
{
    return mp_impl->m_row_size;
}

col_t sheet::col_size() const
{
    return mp_impl->m_col_size;
}

sheet_t sheet::get_index() const
{
    return mp_impl->m_sheet;
}

date_time_t sheet::get_date_time(row_t row, col_t col) const
{
    const ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    const ixion::column_stores_t* stores = cxt.get_columns(mp_impl->m_sheet);
    if (!stores)
        throw orcus::general_error(
            "sheet::get_date_time: failed to get column stores from the model.");

    if (col < 0 || static_cast<size_t>(col) >= stores->size())
    {
        std::ostringstream os;
        os << "invalid column index (" << col << ")";
        throw std::invalid_argument(os.str());
    }

    const ixion::column_store_t& col_store = (*stores)[col];

    if (row < 0 || static_cast<size_t>(row) >= col_store.size())
    {
        std::ostringstream os;
        os << "invalid row index (" << row << ")";
        throw std::invalid_argument(os.str());
    }

    double dt_raw = col_store.get<double>(row); // raw value as days since epoch.

    double days_since_epoch = std::floor(dt_raw);
    double time_fraction = dt_raw - days_since_epoch;

    date_time_t dt_origin = mp_impl->m_doc.get_origin_date();

    posix_time::ptime origin(
        gregorian::date(
            gregorian::greg_year(dt_origin.year),
            gregorian::greg_month(dt_origin.month),
            gregorian::greg_day(dt_origin.day)
        )
    );

    posix_time::ptime date_part = origin + gregorian::days(days_since_epoch);

    long hours = 0;
    long minutes = 0;
    double seconds = 0.0;

    if (time_fraction)
    {
        // Convert a fraction day to microseconds.
        long long ms = time_fraction * 24.0 * 60.0 * 60.0 * 1000000.0;
        posix_time::time_duration td = posix_time::microsec(ms);

        hours = td.hours();
        minutes = td.minutes();
        seconds = td.seconds(); // long to double

        td -= posix_time::hours(hours);
        td -= posix_time::minutes(minutes);
        td -= posix_time::seconds((long)seconds);

        ms = td.total_microseconds(); // remaining microseconds.

        seconds += ms / 1000000.0;
    }

    gregorian::date d = date_part.date();

    return date_time_t(d.year(), d.month(), d.day(), hours, minutes, seconds);
}

void sheet::finalize()
{
    mp_impl->m_col_widths.build_tree();
    mp_impl->m_row_heights.build_tree();
}

void sheet::dump_flat(std::ostream& os) const
{
    detail::flat_dumper dumper(mp_impl->m_doc);
    dumper.dump(os, mp_impl->m_sheet);
}

namespace {

void write_cell_position(ostream& os, const pstring& sheet_name, row_t row, col_t col)
{
    os << sheet_name << '/' << row << '/' << col << ':';
}

string escape_chars(const string& str)
{
    if (str.empty())
        return str;

    string ret;
    const char* p = &str[0];
    const char* p_end = p + str.size();
    for (; p != p_end; ++p)
    {
        if (*p == '"')
            ret.push_back('\\');
        ret.push_back(*p);
    }
    return ret;
}

}

void sheet::dump_check(ostream& os, const pstring& sheet_name) const
{
    ixion::abs_range_t range = mp_impl->get_data_range();
    if (!range.valid())
        // Sheet is empty.  Nothing to print.
        return;

    const ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    const ixion::formula_name_resolver* resolver = mp_impl->m_doc.get_formula_name_resolver();

    size_t row_count = range.last.row + 1;
    size_t col_count = range.last.column + 1;

    for (size_t row = 0; row < row_count; ++row)
    {
        for (size_t col = 0; col < col_count; ++col)
        {
            ixion::abs_address_t pos(mp_impl->m_sheet, row, col);
            switch (cxt.get_celltype(pos))
            {
                case ixion::celltype_t::string:
                {
                    write_cell_position(os, sheet_name, row, col);
                    size_t sindex = cxt.get_string_identifier(pos);
                    const string* p = cxt.get_string(sindex);
                    assert(p);
                    os << "string:\"" << escape_chars(*p) << '"' << endl;
                    break;
                }
                case ixion::celltype_t::numeric:
                {
                    write_cell_position(os, sheet_name, row, col);
                    os << "numeric:";
                    detail::format_to_file_output(os, cxt.get_numeric_value(pos));
                    os << endl;
                    break;
                }
                case ixion::celltype_t::boolean:
                {
                    write_cell_position(os, sheet_name, row, col);
                    os << "boolean:" << (cxt.get_boolean_value(pos) ? "true" : "false") << endl;
                    break;
                }
                case ixion::celltype_t::formula:
                {
                    write_cell_position(os, sheet_name, row, col);
                    os << "formula";

                    // print the formula and the formula result.
                    const ixion::formula_cell* cell = cxt.get_formula_cell(pos);
                    assert(cell);
                    const ixion::formula_tokens_store_ptr_t& ts = cell->get_tokens();
                    if (ts)
                    {
                        const ixion::formula_tokens_t& tokens = ts->get();
                        string formula;
                        if (resolver)
                        {
                            pos = cell->get_parent_position(pos);
                            formula = ixion::print_formula_tokens(
                                mp_impl->m_doc.get_model_context(), pos, *resolver, tokens);
                        }
                        else
                            formula = "???";

                        os << ':';

                        ixion::formula_group_t fg = cell->get_group_properties();

                        if (fg.grouped)
                            os << '{' << formula << '}';
                        else
                            os << formula;

                        ixion::formula_result res = cell->get_result_cache();
                        os << ':' << res.str(mp_impl->m_doc.get_model_context());
                    }

                    os << endl;
                    break;
                }
                default:
                    ;
            }
        }
    }
}

void sheet::dump_html(std::ostream& os) const
{
    if (!mp_impl->m_col_widths.is_tree_valid())
        mp_impl->m_col_widths.build_tree();

    if (!mp_impl->m_row_heights.is_tree_valid())
        mp_impl->m_row_heights.build_tree();

    detail::html_dumper dumper(mp_impl->m_doc, mp_impl->m_merge_ranges, mp_impl->m_sheet);
    dumper.dump(os);
}

void sheet::dump_json(std::ostream& os) const
{
    detail::json_dumper dumper(mp_impl->m_doc);
    dumper.dump(os, mp_impl->m_sheet);
}

void sheet::dump_csv(std::ostream& os) const
{
    detail::csv_dumper dumper(mp_impl->m_doc);
    dumper.dump(os, mp_impl->m_sheet);
}

size_t sheet::get_cell_format(row_t row, col_t col) const
{
    cell_format_type::const_iterator itr = mp_impl->m_cell_formats.find(col);
    if (itr == mp_impl->m_cell_formats.end())
        return 0;

    segment_row_index_type& con = *itr->second;
    if (!con.is_tree_valid())
        con.build_tree();

    size_t index;
    if (!con.search_tree(row, index).second)
        return 0;

    return index;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
