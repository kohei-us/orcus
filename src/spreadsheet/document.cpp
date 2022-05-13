/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "document_impl.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;
namespace fs = boost::filesystem;

namespace orcus { namespace spreadsheet {

namespace {

class find_sheet_by_name
{
    std::string_view m_name;
public:
    find_sheet_by_name(std::string_view name) : m_name(name) {}
    bool operator() (const std::unique_ptr<sheet_item>& v) const
    {
        return v->name == m_name;
    }
};

}

document::document(const range_size_t& sheet_size) : mp_impl(new document_impl(*this, sheet_size)) {}

document::~document() {}

import_shared_strings* document::get_shared_strings()
{
    return mp_impl->shared_strings.get();
}

const import_shared_strings* document::get_shared_strings() const
{
    return mp_impl->shared_strings.get();
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

void document::insert_table(table_t* p)
{
    if (!p)
        return;

    std::string_view name = p->name;
    mp_impl->tables.insert(
        table_store_type::value_type(name, std::unique_ptr<table_t>(p)));
}

const table_t* document::get_table(std::string_view name) const
{
    auto it = mp_impl->tables.find(name);
    return it == mp_impl->tables.end() ? nullptr : it->second.get();
}

void document::finalize()
{
    std::for_each(mp_impl->sheets.begin(), mp_impl->sheets.end(),
        [](std::unique_ptr<sheet_item>& sh)
        {
            sh->data.finalize();
        }
    );
}

sheet* document::append_sheet(std::string_view sheet_name)
{
    std::string_view sheet_name_safe = mp_impl->string_pool_store.intern(sheet_name).first;
    sheet_t sheet_index = static_cast<sheet_t>(mp_impl->sheets.size());

    mp_impl->sheets.push_back(
        std::make_unique<sheet_item>(*this, sheet_name_safe, sheet_index));

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
    mp_impl.reset(new document_impl(*this, get_sheet_size()));
}

void document::dump(dump_format_t format, const std::string& output) const
{
    if (format == dump_format_t::none)
        return;

    if (format == dump_format_t::check)
    {
        // For this output, we write to a single file.
        std::ostream* ostrm = &std::cout;
        std::unique_ptr<std::ofstream> fs;

        if (!output.empty())
        {
            if (fs::is_directory(output))
            {
                std::ostringstream os;
                os << "Output file path points to an existing directory.";
                throw std::invalid_argument(os.str());
            }

            // Output to stdout when output path is not given.
            fs = std::make_unique<std::ofstream>(output.data());
            ostrm = fs.get();
        }

        dump_check(*ostrm);
        return;
    }

    if (output.empty())
        throw std::invalid_argument("No output directory.");

    if (fs::exists(output))
    {
        if (!fs::is_directory(output))
        {
            std::ostringstream os;
            os << "A file named '" << output << "' already exists, and is not a directory.";
            throw std::invalid_argument(os.str());
        }
    }
    else
        fs::create_directory(output);

    switch (format)
    {
        case dump_format_t::csv:
            dump_csv(output);
            break;
        case dump_format_t::flat:
            dump_flat(output);
            break;
        case dump_format_t::html:
            dump_html(output);
            break;
        case dump_format_t::json:
            dump_json(output);
            break;
        case dump_format_t::debug_state:
            dump_debug_state(output);
            break;
        // coverity[dead_error_line] - following conditions exist to avoid compiler warning
        case dump_format_t::none:
        case dump_format_t::unknown:
            break;
        default:
            ;
    }
}

void document::dump_flat(const string& outdir) const
{
    cout << "----------------------------------------------------------------------" << endl;
    cout << "  Document content summary" << endl;
    cout << "----------------------------------------------------------------------" << endl;
    mp_impl->shared_strings->dump();

    cout << "number of sheets: " << mp_impl->sheets.size() << endl;

    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->sheets)
    {
        fs::path outpath{outdir};
        outpath /= std::string{sheet->name};
        outpath.replace_extension(".txt");

        ofstream file(outpath);
        if (!file)
        {
            cerr << "failed to create file: " << outpath << endl;
            return;
        }

        file << "---" << endl;
        file << "Sheet name: " << sheet->name << endl;
        sheet->data.dump_flat(file);
    }
}

void document::dump_check(ostream& os) const
{
    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->sheets)
        sheet->data.dump_check(os, sheet->name);
}

void document::dump_html(const string& outdir) const
{
    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->sheets)
    {
        fs::path outpath{outdir};
        outpath /= std::string{sheet->name};
        outpath.replace_extension(".html");

        ofstream file(outpath);
        if (!file)
        {
            cerr << "failed to create file: " << outpath << endl;
            return;
        }

        sheet->data.dump_html(file);
    }
}

void document::dump_json(const string& outdir) const
{
    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->sheets)
    {
        fs::path outpath{outdir};
        outpath /= std::string{sheet->name};
        outpath.replace_extension(".json");

        ofstream file(outpath);
        if (!file)
        {
            cerr << "failed to create file: " << outpath << endl;
            return;
        }

        sheet->data.dump_json(file);
    }
}

void document::dump_csv(const std::string& outdir) const
{
    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->sheets)
    {
        fs::path outpath{outdir};
        outpath /= std::string{sheet->name};
        outpath.replace_extension(".csv");

        ofstream file(outpath.c_str());
        if (!file)
        {
            cerr << "failed to create file: " << outpath << endl;
            return;
        }

        sheet->data.dump_csv(file);
    }
}

void document::dump_debug_state(const std::string& outdir) const
{
    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->sheets)
    {
        fs::path outpath{outdir};
        outpath /= std::string{sheet->name};
        fs::create_directories(outpath);
        sheet->data.dump_debug_state(outpath.string(), sheet->name);
    }
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

    ixion::formula_name_resolver_t resolver_type_global = ixion::formula_name_resolver_t::unknown;
    ixion::formula_name_resolver_t resolver_type_named_exp_base = ixion::formula_name_resolver_t::unknown;
    ixion::formula_name_resolver_t resolver_type_named_range = ixion::formula_name_resolver_t::unknown;
    char arg_sep = 0;

    switch (mp_impl->grammar)
    {
        case formula_grammar_t::xls_xml:
            resolver_type_global = ixion::formula_name_resolver_t::excel_r1c1;
            arg_sep = ',';
            break;
        case formula_grammar_t::xlsx:
            resolver_type_global = ixion::formula_name_resolver_t::excel_a1;
            arg_sep = ',';
            break;
        case formula_grammar_t::ods:
            resolver_type_global = ixion::formula_name_resolver_t::odff;
            resolver_type_named_exp_base = ixion::formula_name_resolver_t::calc_a1;
            resolver_type_named_range = ixion::formula_name_resolver_t::odf_cra;
            arg_sep = ';';
            break;
        case formula_grammar_t::gnumeric:
            // TODO : Use Excel A1 name resolver for now.
            resolver_type_global = ixion::formula_name_resolver_t::excel_a1;
            arg_sep = ',';
            break;
        default:
            ;
    }

    mp_impl->name_resolver_global.reset();
    mp_impl->name_resolver_named_exp_base.reset();

    if (resolver_type_global != ixion::formula_name_resolver_t::unknown)
    {
        mp_impl->name_resolver_global =
            ixion::formula_name_resolver::get(resolver_type_global, &mp_impl->context);

        if (resolver_type_named_exp_base != ixion::formula_name_resolver_t::unknown)
        {
            mp_impl->name_resolver_named_exp_base =
                ixion::formula_name_resolver::get(resolver_type_named_exp_base, &mp_impl->context);
        }

        if (resolver_type_named_range != ixion::formula_name_resolver_t::unknown)
        {
            mp_impl->name_resolver_named_range =
                ixion::formula_name_resolver::get(resolver_type_named_range, &mp_impl->context);
        }

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
    switch (cxt)
    {
        case formula_ref_context_t::global:
            return mp_impl->name_resolver_global.get();
        case formula_ref_context_t::named_expression_base:
            if (mp_impl->name_resolver_named_exp_base)
                return mp_impl->name_resolver_named_exp_base.get();
            break;
        case formula_ref_context_t::named_range:
            if (mp_impl->name_resolver_named_range)
                return mp_impl->name_resolver_named_range.get();
            break;
        default:
            ;
    }

    return mp_impl->name_resolver_global.get();
}

void document::insert_dirty_cell(const ixion::abs_address_t& pos)
{
    mp_impl->dirty_cells.insert(pos);
}

}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
