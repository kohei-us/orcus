/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace orcus {

namespace spreadsheet { namespace iface {

class import_font_style;

}}

namespace detail {

/**
 * Excel-specific underline types, shared between xlsx and xls-xml.
 */
enum class xls_underline_t
{
    none,
    double_accounting,
    double_normal,
    single_accounting,
    single_normal,
};

void push_to_font_style(detail::xls_underline_t v, spreadsheet::iface::import_font_style& istyle);

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
