/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/exception.hpp"

#include "json_dumper.hpp"
#include "check_dumper.hpp"
#include "csv_dumper.hpp"
#include "flat_dumper.hpp"
#include "html_dumper.hpp"
#include "sheet_impl.hpp"

#include <iostream>
#include <algorithm>
#include <vector>
#include <cassert>
#include <cstdlib>

#include <ixion/exceptions.hpp>
#include <ixion/formula.hpp>
#include <ixion/model_context.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/greg_date.hpp>
#include <boost/filesystem.hpp>

#define ORCUS_DEBUG_SHEET 0

using namespace std;
namespace fs = boost::filesystem;
namespace gregorian = boost::gregorian;
namespace posix_time = boost::posix_time;

namespace orcus { namespace spreadsheet {

namespace {

ixion::abs_range_t to_ixion_range(sheet_t sheet, const range_t& range)
{
    ixion::abs_range_t pos;

    pos.first.sheet  = sheet;
    pos.first.row    = range.first.row;
    pos.first.column = range.first.column;
    pos.last.sheet   = sheet;
    pos.last.row     = range.last.row;
    pos.last.column  = range.last.column;

    return pos;
}

}

const row_t sheet::max_row_limit = 1048575;
const col_t sheet::max_col_limit = 1023;

sheet::sheet(document& doc, sheet_t sheet_index) :
    mp_impl(new sheet_impl(doc, *this, sheet_index)) {}

sheet::~sheet()
{
    delete mp_impl;
}

void sheet::set_auto(row_t row, col_t col, std::string_view s)
{
    if (s.empty())
        return;

    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();

    // First, see if this can be parsed as a number.
    char* endptr = nullptr;
    double val = strtod(s.data(), &endptr);
    const char* endptr_check = s.data() + s.size();
    if (endptr == endptr_check)
        // Treat this as a numeric value.
        cxt.set_numeric_cell(ixion::abs_address_t(mp_impl->m_sheet,row,col), val);
    else
        // Treat this as a string value.
        cxt.set_string_cell(ixion::abs_address_t(mp_impl->m_sheet,row,col), s);
}

void sheet::set_string(row_t row, col_t col, string_id_t sindex)
{
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    cxt.set_string_cell(ixion::abs_address_t(mp_impl->m_sheet,row,col), sindex);

#if ORCUS_DEBUG_SHEET
    cout << "sheet::set_string: sheet=" << mp_impl->m_sheet << "; row=" << row << "; col=" << col << "; si=" << sindex << endl;
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
            auto p = std::make_unique<segment_row_index_type>(0, mp_impl->m_doc.get_sheet_size().rows+1, 0);

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
    try
    {
        ixion::register_formula_cell(cxt, pos);
        mp_impl->m_doc.insert_dirty_cell(pos);
    }
    catch (const ixion::formula_registration_error& e)
    {
#if ORCUS_DEBUG_SHEET
        cout << "sheet::set_formula: sheet=" << mp_impl->m_sheet << "; row=" << row << "; col=" << col << "; e=" << e.what() << endl;
#endif
    }
}

void sheet::set_formula(
    row_t row, col_t col, const ixion::formula_tokens_store_ptr_t& tokens,
    ixion::formula_result result)
{
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();
    ixion::abs_address_t pos(mp_impl->m_sheet, row, col);

    cxt.set_formula_cell(pos, tokens, result);

    try
    {
        ixion::register_formula_cell(cxt, pos);
        mp_impl->m_doc.insert_dirty_cell(pos);
    }
    catch (const ixion::formula_registration_error& e)
    {
#if ORCUS_DEBUG_SHEET
        cout << "sheet::set_formula: sheet=" << mp_impl->m_sheet << "; row=" << row << "; col=" << col << "; e=" << e.what() << endl;
#endif
    }
}

void sheet::set_grouped_formula(const range_t& range, ixion::formula_tokens_t tokens)
{
    ixion::abs_range_t pos = to_ixion_range(mp_impl->m_sheet, range);
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();

    cxt.set_grouped_formula_cells(pos, std::move(tokens));
    try
    {
        ixion::register_formula_cell(cxt, pos.first);
        mp_impl->m_doc.insert_dirty_cell(pos.first);
    }
    catch (const ixion::formula_registration_error& e)
    {
#if ORCUS_DEBUG_SHEET
        cout << "sheet::set_formula: sheet=" << mp_impl->m_sheet << "; range=" << range << "; e=" << e.what() << endl;
#endif
    }
}

void sheet::set_grouped_formula(const range_t& range, ixion::formula_tokens_t tokens, ixion::formula_result result)
{
    ixion::abs_range_t pos = to_ixion_range(mp_impl->m_sheet, range);
    ixion::model_context& cxt = mp_impl->m_doc.get_model_context();

    cxt.set_grouped_formula_cells(pos, std::move(tokens), std::move(result));
    try
    {
        ixion::register_formula_cell(cxt, pos.first);
        mp_impl->m_doc.insert_dirty_cell(pos.first);
    }
    catch (const ixion::formula_registration_error& e)
    {
#if ORCUS_DEBUG_SHEET
        cout << "sheet::set_formula: sheet=" << mp_impl->m_sheet << "; range=" << range << "; e=" << e.what() << endl;
#endif
    }
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
        auto p = std::make_unique<detail::merge_size_type>();
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

sheet_t sheet::get_index() const
{
    return mp_impl->m_sheet;
}

date_time_t sheet::get_date_time(row_t row, col_t col) const
{
    const ixion::model_context& cxt = mp_impl->m_doc.get_model_context();

    // raw value as days since epoch.
    double dt_raw = cxt.get_numeric_value(
        ixion::abs_address_t(mp_impl->m_sheet, row, col));

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

void sheet::dump_check(ostream& os, std::string_view sheet_name) const
{
    detail::check_dumper dumper(*mp_impl, sheet_name);
    dumper.dump(os);
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

void sheet::dump_debug_state(const std::string& output_dir) const
{
    fs::path outdir{output_dir};

    // TODO : continue on...
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
