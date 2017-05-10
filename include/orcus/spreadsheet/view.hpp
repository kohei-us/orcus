/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_VIEW_HPP
#define INCLUDED_ORCUS_SPREADSHEET_VIEW_HPP

#include "orcus/env.hpp"
#include "orcus/spreadsheet/types.hpp"
#include "orcus/spreadsheet/view_types.hpp"

#include <memory>

namespace orcus { namespace spreadsheet {

class sheet_view;
class document;

class ORCUS_SPM_DLLPUBLIC view
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    view(document& doc);
    ~view();

    sheet_view* get_or_create_sheet_view(sheet_t sheet);

    void set_active_sheet(sheet_t sheet);
};

class ORCUS_SPM_DLLPUBLIC sheet_view
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    sheet_view();
    ~sheet_view();

    void set_selection(sheet_pane_t pos, const range_t& range);
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
