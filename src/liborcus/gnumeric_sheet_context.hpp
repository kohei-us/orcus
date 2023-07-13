/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_GNUMERIC_SHEET_CONTEXT_HPP
#define INCLUDED_ORCUS_GNUMERIC_SHEET_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "gnumeric_cell_context.hpp"

#include <orcus/spreadsheet/types.hpp>

#include <memory>
#include <optional>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;
class import_sheet;
class import_auto_filter;
class import_xf;

}}

class gnumeric_sheet_context : public xml_context_base
{
    struct style_region
    {
        spreadsheet::row_t start_row = 0;
        spreadsheet::row_t end_row = 0;
        spreadsheet::col_t start_col = 0;
        spreadsheet::col_t end_col = 0;

        std::size_t xf_id = 0;
        bool contains_conditional_format = false;
    };

public:
    gnumeric_sheet_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_factory* factory);

    virtual ~gnumeric_sheet_context() override;

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;
    virtual void characters(std::string_view str, bool transient) override;

    void reset(spreadsheet::sheet_t sheet_index);

private:
    void start_style_region(const xml_token_attrs_t& attrs);
    void start_style(const xml_token_attrs_t& attrs);
    void start_font(const xml_token_attrs_t& attrs);
    void start_col(const xml_token_attrs_t& attrs);
    void start_row(const xml_token_attrs_t& attrs);
    void start_condition(const xml_token_attrs_t& attrs);
    void start_field(const xml_token_attrs_t& attrs);

    void end_table();
    void end_style(bool conditional_format);
    void end_font();
    void end_style_region();
    void end_condition();
    void end_field();
    void end_expression();

private:
    spreadsheet::iface::import_factory* mp_factory = nullptr;
    spreadsheet::sheet_t m_sheet_index = -1;

    spreadsheet::iface::import_sheet* mp_sheet = nullptr;
    spreadsheet::iface::import_auto_filter* mp_auto_filter = nullptr;
    spreadsheet::iface::import_xf* mp_xf = nullptr;

    std::optional<style_region> m_region_data;

    spreadsheet::color_rgb_t m_front_color;

    /**
     * Used for temporary storage of characters
     */
    std::string_view m_chars;

    gnumeric_cell_context m_cxt_cell;
};

} // namespace orcus

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
