/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ORCUS_SPREADSHEET_MOCK_IMPORT_INTERFACE_HPP__
#define __ORCUS_SPREADSHEET_MOCK_IMPORT_INTERFACE_HPP__

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>

namespace orcus { namespace spreadsheet { namespace mock {

class import_shared_strings : public orcus::spreadsheet::iface::import_shared_strings
{
public:
    virtual ~import_shared_strings() override;

    virtual size_t append(std::string_view s) override;

    virtual size_t add(std::string_view s) override;

    virtual void set_segment_font(size_t font_index) override;
    virtual void set_segment_bold(bool b) override;
    virtual void set_segment_italic(bool b) override;
    virtual void set_segment_font_name(std::string_view s) override;
    virtual void set_segment_font_size(double point) override;
    virtual void set_segment_font_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void append_segment(std::string_view s) override;
    virtual size_t commit_segments() override;
};

class import_styles : public orcus::spreadsheet::iface::import_styles
{
public:
    virtual ~import_styles() override;

    // font

    virtual void set_font_count(size_t n) override;
    virtual iface::import_font_style* start_font_style() override;

    // fill

    virtual void set_fill_count(size_t n) override;
    virtual iface::import_fill_style* start_fill_style() override;

    // border

    virtual void set_border_count(size_t n) override;
    virtual iface::import_border_style* start_border_style() override;

    virtual void set_xf_count(xf_category_t cat, size_t n) = 0;
    virtual void set_cell_style_count(size_t n) override;
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

    virtual src_address_t resolve_address(std::string_view address) override;

    virtual src_range_t resolve_range(std::string_view range) override;
};

class import_array_formula : public orcus::spreadsheet::iface::import_array_formula
{
public:
    virtual ~import_array_formula() override;

    virtual void set_range(const range_t& range) override;

    virtual void set_formula(formula_grammar_t grammar, std::string_view formula) override;

    virtual void set_result_value(row_t row, col_t col, double value) override;

    virtual void set_result_string(row_t row, col_t col, std::string_view value) override;

    virtual void set_result_empty(row_t row, col_t col) override;

    virtual void set_result_bool(row_t row, col_t col, bool value) override;

    virtual void commit() override;
};

class import_formula : public orcus::spreadsheet::iface::import_formula
{
public:
    virtual ~import_formula() override;
    virtual void set_position(row_t row, col_t col) override;
    virtual void set_formula(formula_grammar_t grammar, std::string_view formula) override;
    virtual void set_shared_formula_index(size_t index) override;
    virtual void set_result_value(double value) override;
    virtual void set_result_string(std::string_view value) override;
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

    virtual void set_auto(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, std::string_view s) override;

    virtual void set_string(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, orcus::spreadsheet::string_id_t sindex) override;

    virtual void set_value(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, double value) override;

    virtual void set_bool(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, bool value) override;

    virtual void set_date_time(
        orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col,
        int year, int month, int day, int hours, int minutes, double seconds) override;

    virtual void set_format(orcus::spreadsheet::row_t row, orcus::spreadsheet::col_t col, size_t xf_index) override;

    virtual void set_format(orcus::spreadsheet::row_t row_start, orcus::spreadsheet::col_t col_start,
            orcus::spreadsheet::row_t row_end, orcus::spreadsheet::col_t col_end, size_t xf_index) override;

    virtual void set_column_format(col_t col, std::size_t xf_index) override;

    virtual void set_row_format(row_t col, std::size_t xf_index) override;

    virtual void fill_down_cells(row_t src_row, col_t src_col, row_t range_size) override;

    virtual orcus::spreadsheet::range_size_t get_sheet_size() const override;
};

class import_factory : public orcus::spreadsheet::iface::import_factory
{
public:
    virtual ~import_factory() override;

    virtual orcus::spreadsheet::iface::import_global_settings* get_global_settings() override;

    virtual orcus::spreadsheet::iface::import_shared_strings* get_shared_strings() override;

    virtual orcus::spreadsheet::iface::import_styles* get_styles() override;

    virtual orcus::spreadsheet::iface::import_sheet* append_sheet(
        orcus::spreadsheet::sheet_t sheet_index, std::string_view name) override;

    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(std::string_view name) override;

    virtual orcus::spreadsheet::iface::import_sheet* get_sheet(orcus::spreadsheet::sheet_t sheet_index) override;

    virtual void finalize() override;
};

}}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
