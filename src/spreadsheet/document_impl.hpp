/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/spreadsheet/table.hpp>
#include <orcus/spreadsheet/tables.hpp>
#include <orcus/spreadsheet/config.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/pivot.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/styles.hpp>
#include <orcus/string_pool.hpp>
#include <orcus/types.hpp>

#include <ixion/config.hpp>
#include <ixion/formula.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>
#include <ixion/interface/table_handler.hpp>
#include <ixion/matrix.hpp>
#include <ixion/model_context.hpp>

#include "filesystem_env.hpp"

#include <ostream>

namespace orcus { namespace spreadsheet { namespace detail {

/**
 * Single sheet entry which consists of a sheet name and a sheet data.
 */
struct sheet_item
{
    sheet_item(const sheet_item&) = delete;
    sheet_item& operator=(const sheet_item&) = delete;

    std::string_view name;
    sheet data;
    sheet_item(document& doc, std::string_view _name, sheet_t sheet_index);
};

typedef std::map<std::string_view, std::unique_ptr<table_t>> table_store_type;
typedef std::vector<std::unique_ptr<sheet_item>> sheet_items_type;

using formula_context_to_resolver_type =
    std::map<formula_ref_context_t, ixion::formula_name_resolver_t>;

using name_resolver_store_type =
    std::map<ixion::formula_name_resolver_t, std::unique_ptr<ixion::formula_name_resolver>>;

struct document_impl
{
    document_impl(const document_impl&) = delete;
    document_impl& operator=(const document_impl&) = delete;

    document& doc;

    document_config doc_config;
    string_pool string_pool_store;
    ixion::model_context context;
    date_time_t origin_date;
    sheet_items_type sheets;
    styles styles_store;
    shared_strings ss_store;
    ixion::abs_range_set_t dirty_cells;

    pivot_collection pivots;

    formula_context_to_resolver_type formula_context_to_resolver;
    name_resolver_store_type name_resolver_store;
    formula_grammar_t grammar;

    tables table_store;

    document_impl(document& _doc, const range_size_t& sheet_size);

    void dump(dump_format_t format, const fs::path& outpath) const;
    void dump_flat(const fs::path& outdir) const;
    void dump_html(const fs::path& outdir) const;
    void dump_json(const fs::path& outdir) const;
    void dump_csv(const fs::path& outdir) const;
    void dump_debug_state(const fs::path& outdir) const;
    void dump_check(std::ostream& os) const;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
