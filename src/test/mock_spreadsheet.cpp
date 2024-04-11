/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "mock_spreadsheet.hpp"

#include <cassert>

using namespace orcus::spreadsheet;

namespace orcus { namespace spreadsheet { namespace mock {

//import_factory

import_factory::~import_factory()
{
}

orcus::spreadsheet::iface::import_global_settings* import_factory::get_global_settings()
{
    assert(false);
    return nullptr;
}

orcus::spreadsheet::iface::import_shared_strings* import_factory::get_shared_strings()
{
    assert(false);
    return nullptr;
}

orcus::spreadsheet::iface::import_styles* import_factory::get_styles()
{
    assert(false);
    return nullptr;
}

orcus::spreadsheet::iface::import_sheet* import_factory::append_sheet(orcus::spreadsheet::sheet_t, std::string_view)
{
    assert(false);
    return nullptr;
}

orcus::spreadsheet::iface::import_sheet* import_factory::get_sheet(std::string_view)
{
    assert(false);
    return nullptr;
}

orcus::spreadsheet::iface::import_sheet* import_factory::get_sheet(orcus::spreadsheet::sheet_t)
{
    assert(false);
    return nullptr;
}

void import_factory::finalize() {}

// import_shared_strings

import_shared_strings::~import_shared_strings()
{
}

size_t import_shared_strings::append(std::string_view)
{
    assert(false);
    return 0;
}

size_t import_shared_strings::add(std::string_view)
{
    assert(false);
    return 0;
}

void import_shared_strings::set_segment_font(size_t)
{
    assert(false);
}

void import_shared_strings::set_segment_bold(bool)
{
    assert(false);
}

void import_shared_strings::set_segment_italic(bool)
{
    assert(false);
}

void import_shared_strings::set_segment_superscript(bool)
{
    assert(false);
}
void import_shared_strings::set_segment_subscript(bool)
{
    assert(false);
}

void import_shared_strings::set_segment_font_name(std::string_view)
{
    assert(false);
}

void import_shared_strings::set_segment_font_size(double)
{
    assert(false);
}

void import_shared_strings::set_segment_font_color(color_elem_t, color_elem_t, color_elem_t, color_elem_t)
{
    assert(false);
}

void import_shared_strings::append_segment(std::string_view)
{
    assert(false);
}

size_t import_shared_strings::commit_segments()
{
    assert(false);
    return 0;
}

// import sheet properties

import_sheet_properties::~import_sheet_properties()
{
}

void import_sheet_properties::set_column_width(col_t, col_t, double, length_unit_t)
{
    assert(false);
}

void import_sheet_properties::set_column_hidden(col_t, col_t, bool)
{
    assert(false);
}

void import_sheet_properties::set_row_height(row_t, row_t, double, length_unit_t)
{
    assert(false);
}

void import_sheet_properties::set_row_hidden(row_t, row_t, bool)
{
    assert(false);
}

void import_sheet_properties::set_merge_cell_range(const range_t&)
{
    assert(false);
}

import_reference_resolver::~import_reference_resolver()
{
}

spreadsheet::src_address_t import_reference_resolver::resolve_address(std::string_view)
{
    spreadsheet::src_address_t ret;
    ret.column = ret.row = ret.sheet = 0;
    assert(false);
    return ret;
}

spreadsheet::src_range_t import_reference_resolver::resolve_range(std::string_view)
{
    spreadsheet::src_range_t ret;
    ret.first.column = ret.first.row = ret.last.column = ret.last.row = 0;
    ret.first.sheet = ret.last.sheet = 0;
    assert(false);
    return ret;
}

import_array_formula::~import_array_formula()
{
}

void import_array_formula::set_range(const range_t&)
{
    assert(false);
}

void import_array_formula::set_formula(formula_grammar_t, std::string_view)
{
    assert(false);
}

void import_array_formula::set_result_value(row_t, col_t, double)
{
    assert(false);
}

void import_array_formula::set_result_string(row_t, col_t, std::string_view)
{
    assert(false);
}

void import_array_formula::set_result_empty(row_t, col_t)
{
    assert(false);
}

void import_array_formula::set_result_bool(row_t, col_t, bool)
{
    assert(false);
}

void import_array_formula::commit()
{
    assert(false);
}

import_formula::~import_formula()
{
}

void import_formula::set_position(row_t, col_t)
{
    assert(false);
}

void import_formula::set_formula(formula_grammar_t, std::string_view)
{
    assert(false);
}

void import_formula::set_shared_formula_index(size_t)
{
    assert(false);
}

void import_formula::set_result_value(double)
{
    assert(false);
}

void import_formula::set_result_string(std::string_view)
{
    assert(false);
}

void import_formula::set_result_bool(bool)
{
    assert(false);
}

void import_formula::set_result_empty()
{
    assert(false);
}

void import_formula::commit()
{
    assert(false);
}

// import_sheet

import_sheet::~import_sheet()
{
}

void import_sheet::set_auto(row_t, col_t, std::string_view)
{
    assert(false);
}

void import_sheet::set_value(row_t, col_t, double)
{
    assert(false);
}

void import_sheet::set_bool(row_t, col_t, bool)
{
    assert(false);
}

void import_sheet::set_date_time(row_t, col_t, int, int, int, int, int, double)
{
    assert(false);
}

void import_sheet::set_string(row_t, col_t, string_id_t)
{
    assert(false);
}

void import_sheet::set_format(row_t, col_t, size_t)
{
    assert(false);
}

void import_sheet::set_format(row_t, col_t, row_t, col_t, size_t)
{
    assert(false);
}

void import_sheet::set_column_format(col_t, col_t, std::size_t)
{
    assert(false);
}

void import_sheet::set_row_format(row_t, std::size_t)
{
    assert(false);
}

void import_sheet::fill_down_cells(row_t, col_t, row_t)
{
    assert(false);
}

orcus::spreadsheet::range_size_t import_sheet::get_sheet_size() const
{
    assert(false);
    orcus::spreadsheet::range_size_t ret;
    ret.columns = ret.rows = 0;
    return ret;
}

}}}

