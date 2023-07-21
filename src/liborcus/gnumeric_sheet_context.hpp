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
#include "gnumeric_filter_context.hpp"
#include "gnumeric_names_context.hpp"
#include "gnumeric_styles_context.hpp"

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
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;
    virtual void characters(std::string_view str, bool transient) override;

    void reset();

private:
    void start_style_region(const xml_token_attrs_t& attrs);
    void start_style(const xml_token_attrs_t& attrs);
    void start_font(const xml_token_attrs_t& attrs);
    void start_col(const xml_token_attrs_t& attrs);
    void start_row(const xml_token_attrs_t& attrs);
    void start_condition(const xml_token_attrs_t& attrs);
    void start_name(const xml_token_attrs_t& attrs);

    void end_style(bool conditional_format);
    void end_font();
    void end_style_region();
    void end_condition();
    void end_expression();
    void end_merge();
    void end_name();
    void end_names();

private:
    spreadsheet::iface::import_factory* mp_factory = nullptr;
    spreadsheet::iface::import_sheet* mp_sheet = nullptr;
    spreadsheet::iface::import_xf* mp_xf = nullptr;

    std::optional<style_region> m_region_data;

    spreadsheet::color_rgb_t m_front_color;

    /**
     * Used for temporary storage of characters
     */
    std::string_view m_chars;
    std::string_view m_name;
    std::string_view m_merge_area;

    gnumeric_cell_context m_cxt_cell;
    gnumeric_filter_context m_cxt_filter;
    gnumeric_names_context m_cxt_names;
    gnumeric_styles_context m_cxt_styles;
};

} // namespace orcus

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
