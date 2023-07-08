/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_GNUMERIC_CELL_CONTEXT_HPP
#define INCLUDED_ORCUS_GNUMERIC_CELL_CONTEXT_HPP

#include "xml_context_base.hpp"

#include <orcus/string_pool.hpp>
#include <orcus/spreadsheet/types.hpp>

#include <optional>

namespace orcus {

namespace spreadsheet { namespace iface {

class import_factory;
class import_sheet;

}}

struct gnumeric_cell_data;

class gnumeric_cell_context : public xml_context_base
{
    enum cell_type
    {
        cell_type_unknown,
        cell_type_bool,
        cell_type_value,
        cell_type_string,
        cell_type_formula,
        cell_type_shared_formula,
        cell_type_array,
    };

    struct cell_data
    {
        cell_type type = cell_type_unknown;
        spreadsheet::row_t row = 0;
        spreadsheet::col_t col = 0;
        spreadsheet::row_t array_rows = 0;
        spreadsheet::col_t array_cols = 0;
        std::size_t shared_formula_id = 0;
    };

public:
    gnumeric_cell_context(
        session_context& session_cxt, const tokens& tokens,
        spreadsheet::iface::import_factory* factory);
    virtual ~gnumeric_cell_context() override;

    virtual void start_element(xmlns_id_t ns, xml_token_t name, const xml_token_attrs_t& attrs) override;
    virtual bool end_element(xmlns_id_t ns, xml_token_t name) override;
    virtual void characters(std::string_view str, bool transient) override;

    void reset(spreadsheet::iface::import_sheet* sheet);

private:
    void start_cell(const xml_token_attrs_t& attrs);
    void end_cell();

private:
    spreadsheet::iface::import_factory* mp_factory;
    spreadsheet::iface::import_sheet* mp_sheet;

    std::optional<cell_data> m_cell_data;
    std::string_view m_chars;
};

} // namespace orcus

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
