/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "xml_context_base.hpp"
#include "xlsx_types.hpp"

#include <orcus/string_pool.hpp>

#include <vector>

namespace orcus {

namespace spreadsheet { namespace iface {
    class import_styles;
    class import_font_style;
    class import_fill_style;
    class import_border_style;
    class import_cell_protection;
    class import_number_format;
    class import_xf;
    class import_cell_style;
}}

/**
 * Context for xl/styles.xml part.  This part contains various styles used
 * in the sheets.
 */
class xlsx_styles_context : public xml_context_base
{
public:
    xlsx_styles_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_styles* import_styles);
    virtual ~xlsx_styles_context();

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs);
    virtual bool end_element(xmlns_id_t ns, xml_token_t name);
    virtual void characters(std::string_view str, bool transient);

private:
    void start_element_number_format(const xml_token_attrs_t& attrs);

    void start_element_border(const xml_token_attrs_t& attrs);
    void start_element_diagonal(const xml_token_attrs_t& attrs);
    void start_border_color(const xml_token_attrs_t& attrs);
    void start_font_color(const xml_token_attrs_t& attrs);
    void start_xf(const xml_token_attrs_t& attrs);

    void end_element_number_format();

private:
    spreadsheet::iface::import_styles* mp_styles = nullptr;
    spreadsheet::iface::import_font_style* mp_font = nullptr;
    spreadsheet::iface::import_fill_style* mp_fill = nullptr;
    spreadsheet::iface::import_border_style* mp_border = nullptr;
    spreadsheet::iface::import_cell_protection* mp_protection = nullptr;
    spreadsheet::iface::import_number_format* mp_numfmt = nullptr;
    spreadsheet::iface::import_xf* mp_xf = nullptr;
    spreadsheet::iface::import_cell_style* mp_cell_style = nullptr;

    string_pool m_pool;
    bool m_diagonal_up;
    bool m_diagonal_down;
    spreadsheet::border_direction_t m_cur_border_dir;
    bool m_cell_style_xf;

    std::vector<std::size_t> m_font_ids;
};

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
