/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_XLS_XML_CONTEXT_HPP
#define ORCUS_XLS_XML_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "orcus/spreadsheet/types.hpp"
#include "orcus/string_pool.hpp"

#include <string>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;
class import_sheet;
class import_sheet_properties;
class import_named_expression;

}}

class xls_xml_context : public xml_context_base
{
    struct font_style_type
    {
        bool bold = false;
        bool italic = false;
    };

    struct style_type
    {
        pstring id;
        pstring name;

        font_style_type font;
    };

    struct named_exp
    {
        pstring name;
        pstring expression;
        spreadsheet::sheet_t scope;

        named_exp(const pstring& _name, const pstring& _expression, spreadsheet::sheet_t _scope);
    };

    using named_expressions_type = std::vector<named_exp>;
    using styles_type = std::vector<std::unique_ptr<style_type>>;

public:
    enum cell_type { ct_unknown = 0, ct_string, ct_number, ct_datetime };

    xls_xml_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory);
    virtual ~xls_xml_context();

    virtual bool can_handle_element(xmlns_id_t ns, xml_token_t name) const;
    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name);
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child);

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_attrs_t& attrs);
    virtual bool end_element(xmlns_id_t ns, xml_token_t name);
    virtual void characters(const pstring& str, bool transient);

private:
    void start_element_cell(const xml_token_pair_t& parent, const xml_attrs_t& attrs);
    void end_element_cell();

    void start_element_data(const xml_token_pair_t& parent, const xml_attrs_t& attrs);
    void end_element_data();

    void end_element_workbook();

    void push_formula_cell();

private:
    string_pool m_pool;

    spreadsheet::iface::import_factory* mp_factory;
    spreadsheet::iface::import_sheet* mp_cur_sheet;
    spreadsheet::iface::import_sheet_properties* mp_sheet_props;

    std::vector<spreadsheet::iface::import_named_expression*> m_sheet_named_exps;

    spreadsheet::sheet_t m_cur_sheet;
    spreadsheet::row_t m_cur_row;
    spreadsheet::col_t m_cur_col;
    spreadsheet::row_t m_cur_merge_down;
    spreadsheet::col_t m_cur_merge_across;
    cell_type m_cur_cell_type;
    std::vector<pstring> m_cur_cell_string;
    double m_cur_cell_value;
    date_time_t m_cur_cell_datetime;
    pstring m_cur_cell_formula;

    named_expressions_type m_named_exps_global;
    named_expressions_type m_named_exps_sheet;

    std::unique_ptr<style_type> m_current_style;
    std::unique_ptr<style_type> m_default_style;
    styles_type m_styles;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
