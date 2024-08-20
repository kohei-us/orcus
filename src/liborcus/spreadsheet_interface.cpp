/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_auto_filter.hpp>
#include <orcus/spreadsheet/import_interface_pivot.hpp>
#include <orcus/spreadsheet/import_interface_strikethrough.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>
#include <orcus/spreadsheet/import_interface_underline.hpp>
#include <orcus/spreadsheet/import_interface_view.hpp>
#include <orcus/spreadsheet/export_interface.hpp>

namespace orcus { namespace spreadsheet { namespace iface {

import_sheet_view::~import_sheet_view() {}

import_pivot_cache_definition::~import_pivot_cache_definition() {}

import_pivot_cache_field_group::~import_pivot_cache_field_group() {}

import_pivot_cache_records::~import_pivot_cache_records() {}

import_shared_strings::~import_shared_strings() {}

import_underline* import_shared_strings::start_underline()
{
    return nullptr;
}

import_strikethrough* import_shared_strings::start_strikethrough()
{
    return nullptr;
}

import_strikethrough::~import_strikethrough() = default;

import_underline::~import_underline() = default;

import_styles::~import_styles() {}

import_font_style::~import_font_style() {}

import_underline* import_font_style::start_underline()
{
    return nullptr;
}

import_strikethrough* import_font_style::start_strikethrough()
{
    return nullptr;
}

import_fill_style::~import_fill_style() {}

import_border_style::~import_border_style() {}

import_cell_protection::~import_cell_protection() {}

import_number_format::~import_number_format() {}

import_xf::~import_xf() {}

import_cell_style::~import_cell_style() {}

import_sheet_properties::~import_sheet_properties() {}

import_named_expression::~import_named_expression() {}

import_data_table::~import_data_table() {}

import_auto_filter_node::~import_auto_filter_node() = default;

import_auto_filter::~import_auto_filter() = default;

namespace old {

import_auto_filter::~import_auto_filter() {}

}

import_table::~import_table() {}

import_conditional_format::~import_conditional_format() {}

old::import_auto_filter* import_table::get_auto_filter()
{
    return nullptr;
}

import_auto_filter* import_table::start_auto_filter()
{
    return nullptr;
}

import_formula::~import_formula() {}

import_array_formula::~import_array_formula() {}

import_sheet::~import_sheet() {}

import_sheet_view* import_sheet::get_sheet_view()
{
    return nullptr;
}

import_sheet_properties* import_sheet::get_sheet_properties()
{
    return nullptr;
}

import_data_table* import_sheet::get_data_table()
{
    return nullptr;
}

old::import_auto_filter* import_sheet::get_auto_filter()
{
    return nullptr;
}

import_auto_filter* import_sheet::start_auto_filter(const range_t& /*range*/)
{
    return nullptr;
}

import_table* import_sheet::get_table()
{
    return nullptr;
}

import_conditional_format* import_sheet::get_conditional_format()
{
    return nullptr;
}

import_named_expression* import_sheet::get_named_expression()
{
    return nullptr;
}

import_array_formula* import_sheet::get_array_formula()
{
    return nullptr;
}

import_formula* import_sheet::get_formula()
{
    return nullptr;
}

import_global_settings::~import_global_settings() {}

import_reference_resolver::~import_reference_resolver() {}

import_factory::~import_factory() {}

import_global_settings* import_factory::get_global_settings()
{
    return nullptr;
}

import_shared_strings* import_factory::get_shared_strings()
{
    return nullptr;
}

import_named_expression* import_factory::get_named_expression()
{
    return nullptr;
}

import_styles* import_factory::get_styles()
{
    return nullptr;
}

import_reference_resolver* import_factory::get_reference_resolver(formula_ref_context_t /*cxt*/)
{
    return nullptr;
}

import_pivot_cache_definition* import_factory::create_pivot_cache_definition(
        orcus::spreadsheet::pivot_cache_id_t /*cache_id*/)
{
    return nullptr;
}

import_pivot_cache_records* import_factory::create_pivot_cache_records(
        orcus::spreadsheet::pivot_cache_id_t /*cache_id*/)
{
    return nullptr;
}

export_sheet::~export_sheet() {}

export_factory::~export_factory() {}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
