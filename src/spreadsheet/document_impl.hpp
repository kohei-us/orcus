/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <orcus/global.hpp>
#include <orcus/spreadsheet/auto_filter.hpp>
#include <orcus/spreadsheet/config.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/pivot.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/styles.hpp>
#include <orcus/string_pool.hpp>
#include <orcus/types.hpp>

#include <boost/filesystem.hpp>
#include <ixion/config.hpp>
#include <ixion/formula.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>
#include <ixion/interface/table_handler.hpp>
#include <ixion/matrix.hpp>
#include <ixion/model_context.hpp>

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

class ixion_table_handler : public ixion::iface::table_handler
{
    const ixion::model_context& m_context;
    const table_store_type& m_tables;

    const table_t* find_table(const ixion::abs_address_t& pos) const;

    std::string_view get_string(ixion::string_id_t sid) const;

    col_t find_column(const table_t& tab, std::string_view name, size_t offset) const;

    ixion::abs_range_t get_range_from_table(
        const table_t& tab, ixion::string_id_t column_first, ixion::string_id_t column_last,
        ixion::table_areas_t areas) const;

public:
    ixion_table_handler(const ixion::model_context& cxt, const table_store_type& tables);

    virtual ixion::abs_range_t get_range(
        const ixion::abs_address_t& pos, ixion::string_id_t column_first, ixion::string_id_t column_last,
        ixion::table_areas_t areas) const override;

    virtual ixion::abs_range_t get_range(
        ixion::string_id_t table, ixion::string_id_t column_first, ixion::string_id_t column_last,
        ixion::table_areas_t areas) const override;
};

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
    std::unique_ptr<import_shared_strings> shared_strings;
    ixion::abs_range_set_t dirty_cells;

    pivot_collection pivots;

    std::unique_ptr<ixion::formula_name_resolver> name_resolver_global;
    std::unique_ptr<ixion::formula_name_resolver> name_resolver_named_exp_base;
    std::unique_ptr<ixion::formula_name_resolver> name_resolver_named_range;
    formula_grammar_t grammar;

    table_store_type tables;
    ixion_table_handler table_handler;

    document_impl(document& _doc, const range_size_t& sheet_size);
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
