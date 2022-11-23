/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_ODF_STYLES_CONTEXT_HPP
#define ORCUS_ODF_STYLES_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "odf_styles.hpp"
#include "odf_style_context.hpp"
#include "odf_number_format_context.hpp"

#include <unordered_map>

namespace orcus {

namespace spreadsheet { namespace iface {
    class import_styles;
}}

/**
 * Context that handles <office:automatic-styles> or <office:styles> scope.
 */
class styles_context : public xml_context_base
{
public:
    styles_context(
        session_context& session_cxt, const tokens& tk, spreadsheet::iface::import_styles* iface_styles);

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name) override;
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child) override;
    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;
    virtual void characters(std::string_view str, bool transient) override;

    void reset();
    odf_styles_map_type pop_styles();

private:
    void commit_default_styles();

    void push_number_style(std::unique_ptr<odf_number_format> num_style);

    std::optional<std::size_t> query_parent_style_xfid(std::string_view parent_name) const;

private:
    spreadsheet::iface::import_styles* mp_styles;
    odf_styles_map_type m_styles;

    // an automatic style corresponds to a cell format and not a real style
    bool m_automatic_styles;

    style_context m_cxt_style;
    number_style_context m_cxt_number_style;
    currency_style_context m_cxt_currency_style;
    boolean_style_context m_cxt_boolean_style;
    text_style_context m_cxt_text_style;
    percentage_style_context m_cxt_percentage_style;
    date_style_context m_cxt_date_style;
    time_style_context m_cxt_time_style;
};

} // namespace orcus

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
