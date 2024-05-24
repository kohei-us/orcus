/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xls_types.hpp"

#include <orcus/spreadsheet/import_interface_styles.hpp>
#include <orcus/spreadsheet/import_interface_underline.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus { namespace detail {

void push_to_font_style(detail::xls_underline_t v, ss::iface::import_font_style& istyle)
{
    auto* ul = istyle.start_underline();
    if (!ul)
        return;

    switch (v)
    {
        case detail::xls_underline_t::single_normal:
        {
            ul->set_style(ss::underline_style_t::solid);
            ul->set_count(ss::underline_count_t::single_count);
            break;
        }
        case detail::xls_underline_t::single_accounting:
        {
            ul->set_style(ss::underline_style_t::solid);
            ul->set_count(ss::underline_count_t::single_count);
            ul->set_spacing(ss::underline_spacing_t::continuous_over_field);
            break;
        }
        case detail::xls_underline_t::double_normal:
        {
            ul->set_style(ss::underline_style_t::solid);
            ul->set_count(ss::underline_count_t::double_count);
            break;
        }
        case detail::xls_underline_t::double_accounting:
        {
            ul->set_style(ss::underline_style_t::solid);
            ul->set_count(ss::underline_count_t::double_count);
            ul->set_spacing(ss::underline_spacing_t::continuous_over_field);
            break;
        }
        case detail::xls_underline_t::none:
            break;
    }

    ul->commit();
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
