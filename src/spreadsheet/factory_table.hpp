/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_TABLE_HPP
#define INCLUDED_ORCUS_SPREADSHEET_TABLE_HPP

#include "orcus/spreadsheet/import_interface.hpp"

#include <memory>

namespace orcus { namespace spreadsheet {

class document;
class sheet;

class import_table : public iface::import_table
{
    struct impl;
    std::unique_ptr<impl> mp_impl;

public:
    import_table(document& doc, sheet& sh);
    ~import_table();

    virtual iface::old::import_auto_filter* get_auto_filter() override;

    virtual void set_range(const range_t& range) override;
    virtual void set_identifier(size_t id) override;
    virtual void set_name(std::string_view name) override;
    virtual void set_display_name(std::string_view name) override;
    virtual void set_totals_row_count(size_t row_count) override;

    virtual void set_column_count(size_t n) override;

    virtual void set_column_identifier(size_t id) override;
    virtual void set_column_name(std::string_view name) override;
    virtual void set_column_totals_row_label(std::string_view label) override;
    virtual void set_column_totals_row_function(orcus::spreadsheet::totals_row_function_t func) override;
    virtual void commit_column() override;

    virtual void set_style_name(std::string_view name) override;
    virtual void set_style_show_first_column(bool b) override;
    virtual void set_style_show_last_column(bool b) override;
    virtual void set_style_show_row_stripes(bool b) override;
    virtual void set_style_show_column_stripes(bool b) override;

    virtual void commit() override;

    void reset();
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
