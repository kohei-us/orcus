/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xls_types.hpp"

#include <orcus/spreadsheet/import_interface_styles.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace detail {

void push_to_font_style(detail::xls_underline_t v, ss::iface::import_font_style& istyle)
{
    switch (v)
    {
        case detail::xls_underline_t::single_normal:
        {
            istyle.set_underline_style(ss::underline_style_t::solid);
            istyle.set_underline_count(ss::underline_count_t::single_count);
            break;
        }
        case detail::xls_underline_t::single_accounting:
        {
            istyle.set_underline_style(ss::underline_style_t::solid);
            istyle.set_underline_count(ss::underline_count_t::single_count);
            istyle.set_underline_spacing(ss::underline_spacing_t::continuous_over_field);
            break;
        }
        case detail::xls_underline_t::double_normal:
        {
            istyle.set_underline_style(ss::underline_style_t::solid);
            istyle.set_underline_count(ss::underline_count_t::double_count);
            break;
        }
        case detail::xls_underline_t::double_accounting:
        {
            istyle.set_underline_style(ss::underline_style_t::solid);
            istyle.set_underline_count(ss::underline_count_t::double_count);
            istyle.set_underline_spacing(ss::underline_spacing_t::continuous_over_field);
            break;
        }
        case detail::xls_underline_t::none:
            break;
    }
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
