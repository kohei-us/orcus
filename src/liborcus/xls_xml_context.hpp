/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_XLS_XML_CONTEXT_HPP
#define INCLUDED_ORCUS_XLS_XML_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "orcus/spreadsheet/types.hpp"
#include "orcus/spreadsheet/view_types.hpp"
#include "orcus/string_pool.hpp"

#include <string>
#include <unordered_map>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;
class import_sheet;
class import_sheet_properties;
class import_named_expression;

}}

class xls_xml_context;

/**
 * Context for handling <Data> element scopes.
 */
class xls_xml_data_context : public xml_context_base
{
    struct format_type
    {
        bool bold = false;
        bool italic = false;

        spreadsheet::color_rgb_t color;

        void merge(const format_type& other);
        bool formatted() const;
    };

    struct string_segment_type
    {
        pstring str;
        format_type format;
        bool formatted = false;

        string_segment_type(const pstring& _str);
    };

    enum cell_type { ct_unknown = 0, ct_string, ct_number, ct_datetime };

    xls_xml_context& m_parent_cxt;
    pstring m_cell_formula;

    cell_type m_cell_type;
    std::vector<string_segment_type> m_cell_string;
    std::vector<format_type> m_format_stack;

    format_type m_current_format;

    double m_cell_value;
    date_time_t m_cell_datetime;

public:
    xls_xml_data_context(session_context& session_cxt, const tokens& tokens, xls_xml_context& parent_cxt);
    virtual ~xls_xml_data_context() override;

    virtual bool can_handle_element(xmlns_id_t ns, xml_token_t name) const override;
    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs) override;
    virtual void characters(const pstring& str, bool transient) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

    /**
     * Intendted to be called from the parent context instance, to reset its
     * internal state before its use.
     */
    void reset(const pstring& cell_formula);

private:

    void start_element_data(const xml_token_pair_t& parent, const xml_attrs_t& attrs);
    void end_element_data();

    void push_formula_cell();

    void update_current_format();
};

class xls_xml_context : public xml_context_base
{
    friend class xls_xml_data_context;

    struct border_style_type
    {
        spreadsheet::border_direction_t dir = spreadsheet::border_direction_t::unknown;
        spreadsheet::border_style_t style = spreadsheet::border_style_t::unknown;
        spreadsheet::color_rgb_t color;
    };

    struct font_style_type
    {
        bool bold = false;
        bool italic = false;

        spreadsheet::color_rgb_t color;
    };

    /**
     * TODO:  we only support solid fill for now.  More fill types to be added
     * later.
     */
    struct fill_style_type
    {
        bool solid = false;
        spreadsheet::color_rgb_t color;
    };

    struct text_alignment_type
    {
        spreadsheet::hor_alignment_t hor = spreadsheet::hor_alignment_t::unknown;
        spreadsheet::ver_alignment_t ver = spreadsheet::ver_alignment_t::unknown;
        int8_t indent = 0;
    };

    struct style_type
    {
        pstring id;
        pstring name;

        font_style_type font;
        fill_style_type fill;
        text_alignment_type text_alignment;
        std::vector<border_style_type> borders;
    };

    struct named_exp
    {
        pstring name;
        pstring expression;
        spreadsheet::sheet_t scope;

        named_exp(const pstring& _name, const pstring& _expression, spreadsheet::sheet_t _scope);
    };

    struct selection
    {
        spreadsheet::sheet_pane_t pane;
        spreadsheet::col_t col;
        spreadsheet::row_t row;
        spreadsheet::range_t range;

        selection();
        void reset();
        bool valid_cursor() const;
        bool valid_range() const;
    };

    struct split_pane
    {
        spreadsheet::pane_state_t pane_state;
        spreadsheet::sheet_pane_t active_pane;
        double split_horizontal;
        double split_vertical;
        spreadsheet::row_t top_row_bottom_pane;
        spreadsheet::col_t left_col_right_pane;

        split_pane();
        void reset();
        bool split() const;
        spreadsheet::address_t get_top_left_cell() const;
    };

    using named_expressions_type = std::vector<named_exp>;
    using styles_type = std::vector<std::unique_ptr<style_type>>;
    using style_id_xf_map_type = std::unordered_map<pstring, size_t, pstring::hash>;

public:
    xls_xml_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory);
    virtual ~xls_xml_context();

    virtual void declaration(const xml_declaration_t& decl) override;

    virtual bool can_handle_element(xmlns_id_t ns, xml_token_t name) const override;
    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;
    virtual void characters(const pstring& str, bool transient) override;

private:
    void start_element_borders(const xml_token_pair_t& parent, const xml_attrs_t& attrs);
    void start_element_border(const xml_token_pair_t& parent, const xml_attrs_t& attrs);
    void start_element_cell(const xml_token_pair_t& parent, const xml_attrs_t& attrs);
    void start_element_column(const xml_token_pair_t& parent, const xml_attrs_t& attrs);
    void start_element_row(const xml_token_pair_t& parent, const xml_attrs_t& attrs);

    void end_element_borders();
    void end_element_border();
    void end_element_cell();
    void end_element_column();
    void end_element_row();

    void end_element_workbook();
    void end_element_styles();
    void end_element_pane();
    void end_element_worksheet_options();

    void commit_split_pane();
    void commit_default_style();
    void commit_styles();

private:
    spreadsheet::iface::import_factory* get_import_factory();
    spreadsheet::iface::import_sheet* get_import_sheet();
    spreadsheet::address_t get_current_pos() const;

private:
    spreadsheet::iface::import_factory* mp_factory;
    spreadsheet::iface::import_sheet* mp_cur_sheet;
    spreadsheet::iface::import_sheet_properties* mp_sheet_props;

    std::vector<spreadsheet::iface::import_named_expression*> m_sheet_named_exps;

    spreadsheet::sheet_t m_cur_sheet;
    spreadsheet::row_t m_cur_row;
    spreadsheet::col_t m_cur_col;
    spreadsheet::col_t m_cur_prop_col; /// current column position for column properties.
    spreadsheet::row_t m_cur_merge_down;
    spreadsheet::col_t m_cur_merge_across;
    spreadsheet::range_t m_cur_array_range;
    pstring m_cur_cell_formula;
    pstring m_cur_cell_style_id;

    named_expressions_type m_named_exps_global;
    named_expressions_type m_named_exps_sheet;
    selection m_cursor_selection; /// cursor selection in a single pane.
    split_pane m_split_pane;
    spreadsheet::sheet_pane_t m_active_pane;

    std::unique_ptr<style_type> m_current_style;
    std::unique_ptr<style_type> m_default_style;
    styles_type m_styles;

    style_id_xf_map_type m_style_map;

    xls_xml_data_context m_cc_data;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
