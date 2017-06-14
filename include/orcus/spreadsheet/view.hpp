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
    const sheet_view* get_sheet_view(sheet_t sheet) const;

    void set_active_sheet(sheet_t sheet);
    sheet_t get_active_sheet() const;
};

class ORCUS_SPM_DLLPUBLIC sheet_view
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    sheet_view(view& doc_view);
    ~sheet_view();

    const range_t& get_selection(sheet_pane_t pos) const;

    void set_selection(sheet_pane_t pos, const range_t& range);

    void set_active_pane(sheet_pane_t pos);
    sheet_pane_t get_active_pane() const;

    void set_split_pane(double hor_split, double ver_split, const address_t& top_left_cell);
    const split_pane_t& get_split_pane() const;

    void set_frozen_pane(col_t visible_cols, row_t visible_rows, const address_t& top_left_cell);
    const frozen_pane_t& get_frozen_pane() const;

    view& get_document_view();
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
