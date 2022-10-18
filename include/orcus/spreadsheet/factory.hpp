/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_IMPORT_FACTORY_HPP
#define INCLUDED_ORCUS_SPREADSHEET_IMPORT_FACTORY_HPP

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>
#include <orcus/spreadsheet/export_interface.hpp>
#include <orcus/env.hpp>

#include <memory>

namespace orcus {

class string_pool;

namespace spreadsheet {

class document;
class view;
class styles;

class ORCUS_SPM_DLLPUBLIC import_factory : public iface::import_factory
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    import_factory(document& doc);
    import_factory(document& doc, view& view);
    virtual ~import_factory();

    virtual iface::import_global_settings* get_global_settings() override;
    virtual iface::import_shared_strings* get_shared_strings() override;
    virtual iface::import_styles* get_styles() override;
    virtual iface::import_named_expression* get_named_expression() override;
    virtual iface::import_reference_resolver* get_reference_resolver(formula_ref_context_t cxt) override;
    virtual iface::import_pivot_cache_definition* create_pivot_cache_definition(
        orcus::spreadsheet::pivot_cache_id_t cache_id) override;
    virtual iface::import_pivot_cache_records* create_pivot_cache_records(
        orcus::spreadsheet::pivot_cache_id_t cache_id) override;
    virtual iface::import_sheet* append_sheet(sheet_t sheet_index, std::string_view name) override;
    virtual iface::import_sheet* get_sheet(std::string_view name) override;
    virtual iface::import_sheet* get_sheet(sheet_t sheet_index) override;
    virtual void finalize() override;

    void set_default_row_size(row_t row_size);
    void set_default_column_size(col_t col_size);

    void set_character_set(character_set_t charset);
    character_set_t get_character_set() const;

    /**
     * When setting this flag to true, those formula cells with no cached
     * results will be re-calculated upon loading.
     *
     *
     * @param b value of this flag.
     */
    void set_recalc_formula_cells(bool b);

    void set_formula_error_policy(formula_error_policy_t policy);
};

class ORCUS_SPM_DLLPUBLIC import_styles : public iface::import_styles
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    import_styles(styles& styles, string_pool& sp);
    virtual ~import_styles() override;

    virtual iface::import_font_style* start_font_style() override;
    virtual iface::import_fill_style* start_fill_style() override;
    virtual iface::import_border_style* start_border_style() override;
    virtual iface::import_cell_protection* start_cell_protection() override;
    virtual iface::import_number_format* start_number_format() override;
    virtual iface::import_xf* start_xf(xf_category_t cat) override;
    virtual iface::import_cell_style* start_cell_style() override;

    virtual void set_font_count(size_t n) override;
    virtual void set_fill_count(size_t n) override;
    virtual void set_border_count(size_t n) override;
    virtual void set_number_format_count(size_t n) override;
    virtual void set_xf_count(xf_category_t cat, size_t n) override;
    virtual void set_cell_style_count(size_t n) override;
};

class ORCUS_SPM_DLLPUBLIC import_font_style : public iface::import_font_style
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_font_style() = delete;
    import_font_style(styles& _styles_model, string_pool& sp);
    virtual ~import_font_style() override;

    virtual void set_bold(bool b) override;
    virtual void set_bold_asian(bool b) override;
    virtual void set_bold_complex(bool b) override;

    virtual void set_italic(bool b) override;
    virtual void set_italic_asian(bool b) override;
    virtual void set_italic_complex(bool b) override;

    virtual void set_name(std::string_view s) override;
    virtual void set_name_asian(std::string_view s) override;
    virtual void set_name_complex(std::string_view s) override;

    virtual void set_size(double point) override;
    virtual void set_size_asian(double point) override;
    virtual void set_size_complex(double point) override;

    virtual void set_underline(underline_t e) override;
    virtual void set_underline_width(underline_width_t e) override;
    virtual void set_underline_mode(underline_mode_t e) override;
    virtual void set_underline_type(underline_type_t e) override;
    virtual void set_underline_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_strikethrough_style(strikethrough_style_t s) override;
    virtual void set_strikethrough_type(strikethrough_type_t s) override;
    virtual void set_strikethrough_width(strikethrough_width_t s) override;
    virtual void set_strikethrough_text(strikethrough_text_t s) override;
    virtual size_t commit() override;

    void reset();
};

class ORCUS_SPM_DLLPUBLIC import_fill_style : public iface::import_fill_style
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_fill_style() = delete;
    import_fill_style(styles& _styles_model, string_pool& sp);
    virtual ~import_fill_style() override;

    virtual void set_pattern_type(fill_pattern_t fp) override;
    virtual void set_fg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_bg_color(color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual size_t commit() override;

    void reset();
};

class ORCUS_SPM_DLLPUBLIC import_border_style : public iface::import_border_style
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_border_style() = delete;
    import_border_style(styles& _styles_model, string_pool& sp);
    virtual ~import_border_style() override;

    virtual void set_style(border_direction_t dir, border_style_t style) override;
    virtual void set_color(
        border_direction_t dir, color_elem_t alpha, color_elem_t red, color_elem_t green, color_elem_t blue) override;
    virtual void set_width(border_direction_t dir, double width, orcus::length_unit_t unit) override;
    virtual size_t commit() override;

    void reset();
};

class ORCUS_SPM_DLLPUBLIC import_cell_protection : public iface::import_cell_protection
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_cell_protection() = delete;
    import_cell_protection(styles& _styles_model, string_pool& sp);
    virtual ~import_cell_protection() override;

    virtual void set_hidden(bool b) override;
    virtual void set_locked(bool b) override;
    virtual void set_print_content(bool b) override;
    virtual void set_formula_hidden(bool b) override;
    virtual size_t commit() override;

    void reset();
};

class ORCUS_SPM_DLLPUBLIC import_number_format : public iface::import_number_format
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_number_format() = delete;
    import_number_format(styles& _styles_model, string_pool& sp);
    virtual ~import_number_format() override;

    virtual void set_identifier(std::size_t id) override;
    virtual void set_code(std::string_view s) override;
    virtual size_t commit() override;

    void reset();
};

class ORCUS_SPM_DLLPUBLIC import_xf : public iface::import_xf
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_xf() = delete;
    import_xf(styles& _styles_model, string_pool& sp);
    virtual ~import_xf() override;

    virtual void set_font(size_t index) override;
    virtual void set_fill(size_t index) override;
    virtual void set_border(size_t index) override;
    virtual void set_protection(size_t index) override;
    virtual void set_number_format(size_t index) override;
    virtual void set_style_xf(size_t index) override;
    virtual void set_apply_alignment(bool b) override;
    virtual void set_horizontal_alignment(hor_alignment_t align) override;
    virtual void set_vertical_alignment(ver_alignment_t align) override;
    virtual void set_wrap_text(bool b) override;
    virtual void set_shrink_to_fit(bool b) override;
    virtual size_t commit() override;

    void reset(xf_category_t cat);
};

class ORCUS_SPM_DLLPUBLIC import_cell_style : public iface::import_cell_style
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_cell_style() = delete;
    import_cell_style(styles& _styles_model, string_pool& sp);
    virtual ~import_cell_style() override;

    void set_name(std::string_view s) override;
    void set_display_name(std::string_view s) override;
    void set_xf(size_t index) override;
    void set_builtin(size_t index) override;
    void set_parent_name(std::string_view s) override;
    void commit() override;

    void reset();
};

class ORCUS_SPM_DLLPUBLIC export_factory : public iface::export_factory
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    export_factory(const document& doc);
    virtual ~export_factory();

    virtual const iface::export_sheet* get_sheet(std::string_view sheet_name) const override;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
