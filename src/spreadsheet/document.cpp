/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "document_impl.hpp"
#include "debug_state_dumper.hpp"
#include "debug_state_context.hpp"
#include "filesystem_env.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>

namespace orcus { namespace spreadsheet {

namespace {

class find_sheet_by_name
{
    std::string_view m_name;
public:
    find_sheet_by_name(std::string_view name) : m_name(name) {}
    bool operator() (const std::unique_ptr<detail::sheet_item>& v) const
    {
        return v->name == m_name;
    }
};

}

document::document(const range_size_t& sheet_size) :
    mp_impl(std::make_unique<detail::document_impl>(*this, sheet_size)) {}

document::~document() = default;

shared_strings& document::get_shared_strings()
{
    return mp_impl->ss_store;
}

const shared_strings& document::get_shared_strings() const
{
    return mp_impl->ss_store;
}

styles& document::get_styles()
{
    return mp_impl->styles_store;
}

const styles& document::get_styles() const
{
    return mp_impl->styles_store;
}

pivot_collection& document::get_pivot_collection()
{
    return mp_impl->pivots;
}

const pivot_collection& document::get_pivot_collection() const
{
    return mp_impl->pivots;
}

ixion::model_context& document::get_model_context()
{
    return mp_impl->context;
}

const ixion::model_context& document::get_model_context() const
{
    return mp_impl->context;
}

const document_config& document::get_config() const
{
    return mp_impl->doc_config;
}

void document::set_config(const document_config& cfg)
{
    mp_impl->doc_config = cfg;
    ixion::config ixion_cfg = mp_impl->context.get_config();
    ixion_cfg.output_precision = cfg.output_precision;
    mp_impl->context.set_config(ixion_cfg);
}

string_pool& document::get_string_pool()
{
    return mp_impl->string_pool_store;
}

const string_pool& document::get_string_pool() const
{
    return mp_impl->string_pool_store;
}

tables& document::get_tables()
{
    return mp_impl->table_store;
}

const tables& document::get_tables() const
{
    return mp_impl->table_store;
}

void document::finalize_import()
{
    std::for_each(mp_impl->sheets.begin(), mp_impl->sheets.end(),
        [](std::unique_ptr<detail::sheet_item>& sh)
        {
            sh->data.finalize_import();
        }
    );

    mp_impl->styles_store.finalize_import();
}

sheet* document::append_sheet(std::string_view sheet_name)
{
    std::string_view sheet_name_safe = mp_impl->string_pool_store.intern(sheet_name).first;
    sheet_t sheet_index = static_cast<sheet_t>(mp_impl->sheets.size());

    mp_impl->sheets.push_back(
        std::make_unique<detail::sheet_item>(*this, sheet_name_safe, sheet_index));

    mp_impl->context.append_sheet(std::string{sheet_name_safe});

    return &mp_impl->sheets.back()->data;
}

sheet* document::get_sheet(std::string_view sheet_name)
{
    const sheet* sh = const_cast<const document*>(this)->get_sheet(sheet_name);
    return const_cast<sheet*>(sh);
}

const sheet* document::get_sheet(std::string_view sheet_name) const
{
    auto it = std::find_if(
        mp_impl->sheets.begin(), mp_impl->sheets.end(), find_sheet_by_name(sheet_name));

    if (it == mp_impl->sheets.end())
        return nullptr;

    return &(*it)->data;
}

sheet* document::get_sheet(sheet_t sheet_pos)
{
    const sheet* sh = const_cast<const document*>(this)->get_sheet(sheet_pos);
    return const_cast<sheet*>(sh);
}

const sheet* document::get_sheet(sheet_t sheet_pos) const
{
    if (static_cast<size_t>(sheet_pos) >= mp_impl->sheets.size())
        return nullptr;

    return &mp_impl->sheets[sheet_pos]->data;
}

void document::recalc_formula_cells()
{
    ixion::abs_range_set_t empty;

    ixion::model_context& cxt = get_model_context();
    std::vector<ixion::abs_range_t> sorted = ixion::query_and_sort_dirty_cells(
        cxt, empty, &mp_impl->dirty_cells);
    ixion::calculate_sorted_cells(cxt, sorted, 0);
}

void document::clear()
{
    mp_impl = std::make_unique<detail::document_impl>(*this, get_sheet_size());
}

void document::dump(dump_format_t format, std::string_view output) const
{
    fs::path outpath{output};
    mp_impl->dump(format, outpath);
}

void document::dump(dump_format_t format, std::u16string_view output) const
{
    fs::path outpath{output};
    mp_impl->dump(format, outpath);
}

void document::dump_check(std::ostream& os) const
{
    mp_impl->dump_check(os);
}

sheet_t document::get_sheet_index(std::string_view name) const
{
    auto it = std::find_if(
        mp_impl->sheets.begin(), mp_impl->sheets.end(), find_sheet_by_name(name));

    if (it == mp_impl->sheets.end())
        return ixion::invalid_sheet;

    auto it_beg = mp_impl->sheets.begin();
    size_t pos = std::distance(it_beg, it);
    return static_cast<sheet_t>(pos);
}

std::string_view document::get_sheet_name(sheet_t sheet_pos) const
{
    if (sheet_pos < 0)
        return std::string_view{};

    size_t pos = static_cast<size_t>(sheet_pos);
    if (pos >= mp_impl->sheets.size())
        return std::string_view{};

    return mp_impl->sheets[pos]->name;
}

void document::set_sheet_name(sheet_t sheet_pos, std::string name)
{
    assert(mp_impl->sheets.size() == mp_impl->context.get_sheet_count());

    std::string_view name_interned = mp_impl->string_pool_store.intern(name).first;
    mp_impl->context.set_sheet_name(sheet_pos, std::move(name)); // will throw on invalid name or position
    mp_impl->sheets[sheet_pos]->name = name_interned;
}

range_size_t document::get_sheet_size() const
{
    ixion::rc_size_t ss = mp_impl->context.get_sheet_size();
    range_size_t ret;
    ret.rows = ss.row;
    ret.columns = ss.column;
    return ret;
}

void document::set_sheet_size(const range_size_t& sheet_size)
{
    mp_impl->context.set_sheet_size({sheet_size.rows, sheet_size.columns});
}

size_t document::get_sheet_count() const
{
    return mp_impl->sheets.size();
}

void document::set_origin_date(int year, int month, int day)
{
    mp_impl->origin_date.year = year;
    mp_impl->origin_date.month = month;
    mp_impl->origin_date.day = day;
}

date_time_t document::get_origin_date() const
{
    return mp_impl->origin_date;
}

void document::set_formula_grammar(formula_grammar_t grammar)
{
    if (mp_impl->grammar == grammar)
        return;

    mp_impl->grammar = grammar;

    char arg_sep = 0;

    mp_impl->formula_context_to_resolver.clear();

    switch (mp_impl->grammar)
    {
        case formula_grammar_t::xls_xml:
        {
            mp_impl->formula_context_to_resolver = {
                { formula_ref_context_t::global, ixion::formula_name_resolver_t::excel_r1c1 },
                { formula_ref_context_t::named_expression_base, ixion::formula_name_resolver_t::excel_r1c1 },
                { formula_ref_context_t::named_range, ixion::formula_name_resolver_t::excel_r1c1 },
                { formula_ref_context_t::table_range, ixion::formula_name_resolver_t::excel_r1c1 },
            };

            arg_sep = ',';
            break;
        }
        case formula_grammar_t::xlsx:
        {
            mp_impl->formula_context_to_resolver = {
                { formula_ref_context_t::global, ixion::formula_name_resolver_t::excel_a1 },
                { formula_ref_context_t::named_expression_base, ixion::formula_name_resolver_t::excel_a1 },
                { formula_ref_context_t::named_range, ixion::formula_name_resolver_t::excel_a1 },
                { formula_ref_context_t::table_range, ixion::formula_name_resolver_t::excel_a1 },
            };

            arg_sep = ',';
            break;
        }
        case formula_grammar_t::ods:
        {
            mp_impl->formula_context_to_resolver = {
                { formula_ref_context_t::global, ixion::formula_name_resolver_t::odff },
                { formula_ref_context_t::named_expression_base, ixion::formula_name_resolver_t::calc_a1 },
                { formula_ref_context_t::named_range, ixion::formula_name_resolver_t::odf_cra },
                { formula_ref_context_t::table_range, ixion::formula_name_resolver_t::odf_cra },
            };

            arg_sep = ';';
            break;
        }
        case formula_grammar_t::gnumeric:
        {
            // TODO : Use Excel A1 name resolver for now.
            mp_impl->formula_context_to_resolver = {
                { formula_ref_context_t::global, ixion::formula_name_resolver_t::excel_a1 },
                { formula_ref_context_t::named_expression_base, ixion::formula_name_resolver_t::excel_a1 },
                { formula_ref_context_t::named_range, ixion::formula_name_resolver_t::excel_a1 },
                { formula_ref_context_t::table_range, ixion::formula_name_resolver_t::excel_a1 },
            };

            arg_sep = ',';
            break;
        }
        default:
            ;
    }

    if (arg_sep)
    {
        ixion::config cfg = mp_impl->context.get_config();
        cfg.sep_function_arg = arg_sep;
        cfg.output_precision = mp_impl->doc_config.output_precision;
        mp_impl->context.set_config(cfg);
    }
}

formula_grammar_t document::get_formula_grammar() const
{
    return mp_impl->grammar;
}

const ixion::formula_name_resolver* document::get_formula_name_resolver(formula_ref_context_t cxt) const
{
    ixion::formula_name_resolver_t resolver_type = ixion::formula_name_resolver_t::unknown;

    {
        // Find the corresponding ixion's resolver type.
        auto it = mp_impl->formula_context_to_resolver.find(cxt);
        if (it != mp_impl->formula_context_to_resolver.end())
            resolver_type = it->second;
    }

    if (resolver_type == ixion::formula_name_resolver_t::unknown)
        return nullptr;

    auto it = mp_impl->name_resolver_store.find(resolver_type);
    if (it == mp_impl->name_resolver_store.end())
    {
        auto res = mp_impl->name_resolver_store.insert_or_assign(
            resolver_type, ixion::formula_name_resolver::get(resolver_type, &mp_impl->context));

        it = res.first;
    }

    return it->second.get();
}

void document::insert_dirty_cell(const ixion::abs_address_t& pos)
{
    mp_impl->dirty_cells.insert(pos);
}

}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
