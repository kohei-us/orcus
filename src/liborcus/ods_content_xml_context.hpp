/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_ODSCONTEXT_XML_CONTEXT_HPP
#define INCLUDED_ORCUS_ODSCONTEXT_XML_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "odf_para_context.hpp"
#include "ods_dde_links_context.hpp"
#include "odf_styles.hpp"
#include "odf_styles_context.hpp"
#include "orcus/spreadsheet/types.hpp"

#include <vector>
#include <unordered_map>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;
class import_sheet;

}}

class ods_content_xml_context : public xml_context_base
{
    typedef std::unordered_map<std::string_view, std::size_t> name2id_type;

public:
    struct sheet_data
    {
        spreadsheet::iface::import_sheet* sheet;
        spreadsheet::sheet_t index;

        sheet_data();

        void reset();
    };

    struct row_attr
    {
        long number_rows_repeated;
        row_attr();
    };

    enum cell_value_type { vt_unknown, vt_float, vt_string, vt_date };

    struct cell_attr
    {
        long number_columns_repeated;
        cell_value_type type;
        double value;
        std::string_view date_value;
        std::string_view style_name;

        std::string_view formula;
        spreadsheet::formula_grammar_t formula_grammar;

        cell_attr();
    };

    ods_content_xml_context(session_context& session_cxt, const tokens& tokens, spreadsheet::iface::import_factory* factory);
    virtual ~ods_content_xml_context() override;

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;

private:
    void start_null_date(const xml_token_attrs_t& attrs);

    void start_table(const xml_token_pair_t& parent, const xml_token_attrs_t& attrs);
    void end_table();

    void start_named_range(const xml_token_attrs_t& attrs);
    void end_named_range();

    void start_named_expression(const xml_token_attrs_t& attrs);
    void end_named_expression();

    void start_column(const xml_token_attrs_t& attrs);
    void end_column();

    void start_row(const xml_token_attrs_t& attrs);
    void end_row();

    void start_cell(const xml_token_attrs_t& attrs);
    void end_cell();

    /**
     * Push a named cell style as a parent style of an automatic style, as we
     * cannot directly reference a named cell style from a cell, column etc.
     *
     * @param style_name name of the named cell style to push.
     *
     * @return xfid of the newly-created automatic style that "wraps" the named
     *         cell as its parent if the call is successful, else it's not set.
     */
    std::optional<std::size_t> push_named_cell_style(std::string_view style_name);
    void push_default_column_cell_style(std::string_view style_name, spreadsheet::col_t span);
    void push_cell_format();
    void push_cell_value();

    void end_spreadsheet();

private:
    spreadsheet::iface::import_factory* mp_factory;
    std::vector<spreadsheet::iface::import_sheet*> m_tables;
    sheet_data m_cur_sheet;

    row_attr    m_row_attr;
    cell_attr   m_cell_attr; /// attributes of current cell.

    int m_row;
    int m_col;
    int m_col_repeated;
    size_t m_para_index;
    bool m_has_content;

    odf_styles_map_type m_styles; /// map storing all automatic styles by their names.
    name2id_type m_cell_format_map; /// map of automatic style names to cell format (xf) IDs.

    styles_context m_child_styles;
    text_para_context m_child_para;
    ods_dde_links_context m_child_dde_links;
};

}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
