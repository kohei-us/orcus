/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ORCUS_SPREADSHEET_MOCK_IMPORT_INTERFACE_HPP__
#define __ORCUS_SPREADSHEET_MOCK_IMPORT_INTERFACE_HPP__

#include <orcus/spreadsheet/import_interface.hpp>

namespace orcus { namespace spreadsheet { namespace mock {

class import_shared_strings : public orcus::spreadsheet::iface::import_shared_strings
{
public:
    virtual ~import_shared_strings() override;

    virtual size_t append(const char* s, size_t n) override;

    virtual size_t add(const char* s, size_t n) override;

    virtual void set_segment_font(size_t font_index) override;
    virtual void set_segment_bold(bool b) override;
    virtual void set_segment_italic(bool b) override;
    virtual void set_segment_font_name(const char* s, size_t n) override;
    virtual void set_segment_font_size(double point) override;
    virtual void set_segment_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void append_segment(const char* s, size_t n) override;
    virtual size_t commit_segments() override;
};

class import_styles : public orcus::spreadsheet::iface::import_styles
{
public:
    virtual ~import_styles() override;

    // font

    virtual void set_font_count(size_t n) override;
    virtual void set_font_bold(bool b) override;
    virtual void set_font_italic(bool b) override;
    virtual void set_font_name(const char* s, size_t n) override;
    virtual void set_font_size(double point) override;
    virtual void set_font_underline(orcus::spreadsheet::underline_t e) override;
    virtual size_t commit_font() override;

    // fill

    virtual void set_fill_count(size_t n) override;
    virtual void set_fill_pattern_type(fill_pattern_t fp) override;
    virtual void set_fill_fg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_fill_bg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual size_t commit_fill() override;

    // border

    virtual void set_border_count(size_t n) override;
    virtual void set_border_color(orcus::spreadsheet::border_direction_t dir,
            color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual size_t commit_border() override;

    // cell protection
    virtual void set_cell_hidden(bool b) override;
    virtual void set_cell_locked(bool b) override;
    virtual size_t commit_cell_protection() override;

    // cell style xf

    virtual void set_cell_style_xf_count(size_t n) override;
    virtual size_t commit_cell_style_xf() override;

    // cell xf

    virtual void set_cell_xf_count(size_t n) override;
    virtual size_t commit_cell_xf() override;

    // dxf
    virtual void set_dxf_count(size_t n) override;
    virtual size_t commit_dxf() override;

    // xf (cell format) - used both by cell xf and cell style xf.

    virtual void set_xf_number_format(size_t index) override;
    virtual void set_xf_font(size_t index) override;
    virtual void set_xf_fill(size_t index) override;
    virtual void set_xf_border(size_t index) override;
    virtual void set_xf_protection(size_t index) override;
    virtual void set_xf_style_xf(size_t index) override;
    virtual void set_xf_apply_alignment(bool b) override;
    virtual void set_xf_horizontal_alignment(orcus::spreadsheet::hor_alignment_t align) override;
    virtual void set_xf_vertical_alignment(orcus::spreadsheet::ver_alignment_t align) override;

    // cell style entry

    virtual void set_cell_style_count(size_t n) override;
    virtual void set_cell_style_name(const char* s, size_t n) override;
    virtual void set_cell_style_xf(size_t index) override;
    virtual void set_cell_style_builtin(size_t index) override;
    virtual size_t commit_cell_style() override;
};

class import_sheet_properties : public orcus::spreadsheet::iface::import_sheet_properties
{
public:
    virtual ~import_sheet_properties() override;

    virtual void set_column_width(orcus::spreadsheet::col_t col, double width, orcus::length_unit_t unit) override;

    virtual void set_column_hidden(orcus::spreadsheet::col_t col, bool hidden) override;

    virtual void set_row_height(orcus::spreadsheet::row_t row, double height, orcus::length_unit_t unit) override;

    virtual void set_row_hidden(orcus::spreadsheet::row_t row, bool hidden) override;

    virtual void set_merge_cell_range(const range_t& range) override;
};

class import_reference_resolver : public orcus::spreadsheet::iface::import_reference_resolver
{
public:
    virtual ~import_reference_resolver() override;

    virtual address_t resolve_address(const char* p, size_t n) override;

    virtual range_t resolve_range(const char* p, size_t n) override;
};

class import_array_formula : public orcus::spreadsheet::iface::import_array_formula
{
public:
    virtual ~import_array_formula() override;

    virtual void set_range(const range_t& range) override;

    virtual void set_formula(formula_grammar_t grammar, const char* p, size_t n) override;

    virtual void set_result_value(row_t row, col_t col, double value) override;

    virtual void set_result_string(row_t row, col_t col, size_t sindex) override;

    virtual void set_result_empty(row_t row, col_t col) override;

    virtual void set_result_bool(row_t row, col_t col, bool value) override;

    virtual void commit() override;
};

class import_formula : public orcus::spreadsheet::iface::import_formula
{
public:
    virtual ~import_formula() override;
    virtual void set_position(row_t row, col_t col) override;
    virtual void set_formula(formula_grammar_t grammar, const char* p, size_t n) override;
    virtual void set_shared_formula_index(size_t index) override;
    virtual void set_result_value(double value) override;
    virtual void set_result_string(size_t sindex) override;
    virtual void set_result_bool(bool value) override;
    virtual void set_result_empty() override;
    virtual void commit() override;
};

/**
 * Interface for sheet.
 */
class import_sheet : public orcus::spreadsheet::iface::import_sheet
{
public:
    virtual ~import_sheet() override;

    virtual void set_auto(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, const char* p, size_t n) override;

    virtual void set_string(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t sindex) override;

    virtual void set_value(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double value) override;

    virtual void set_bool(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, bool value) override;

    virtual void set_date_time(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col,
        int year, int month, int day, int hours, int minutes, double seconds) override;

    virtual void set_format(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t xf_index) override;

    virtual void set_format(orcus::spreadsheet::row_t row_start, orcus::spreadsheet::col_t col_start,
            orcus::spreadsheet::row_t row_end, orcus::spreadsheet::col_t col_end, size_t xf_index) override;

    virtual void fill_down_cells(row_t src_row, col_t src_col, row_t range_size) override;

    virtual orcus::spreadsheet::range_size_t get_sheet_size() const override;
};

class import_factory : public orcus::spreadsheet::iface::import_factory
{
public:
    virtual ~import_factory();

    virtual orcus::spreadsheet::iface::import_global_settings* get_global_settings();

    virtual orcus::spreadsheet::iface::import_shared_strings* get_shared_strings();

    virtual orcus::spreadsheet::iface::import_styles* get_styles();

    virtual orcus::spreadsheet::iface::import_sheet* append_sheet(
        orcus::spreadsheet::sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length);

    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(const char* sheet_name, size_t sheet_name_length);

    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(orcus::spreadsheet::sheet_t sheet_index);

    virtual void finalize();
};

}}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
