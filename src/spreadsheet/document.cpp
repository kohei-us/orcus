/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/document.hpp"

#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/shared_strings.hpp"
#include "orcus/spreadsheet/styles.hpp"
#include "orcus/spreadsheet/auto_filter.hpp"
#include "orcus/spreadsheet/pivot.hpp"
#include "orcus/spreadsheet/config.hpp"

#include "pstring.hpp"
#include "orcus/types.hpp"
#include "orcus/string_pool.hpp"
#include "orcus/global.hpp"

#include <ixion/formula.hpp>
#include <ixion/formula_result.hpp>
#include <ixion/matrix.hpp>
#include <ixion/model_context.hpp>
#include <ixion/formula_name_resolver.hpp>
#include <ixion/interface/table_handler.hpp>
#include <ixion/config.hpp>
#include <boost/filesystem.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace std;
namespace fs = boost::filesystem;

namespace orcus { namespace spreadsheet {

namespace {

/**
 * Single sheet entry which consists of a sheet name and a sheet data.
 */
struct sheet_item
{
    sheet_item(const sheet_item&) = delete;
    sheet_item& operator=(const sheet_item&) = delete;

    pstring name;
    sheet   data;
    sheet_item(document& doc, std::string_view _name, sheet_t sheet_index);
};

typedef std::map<pstring, std::unique_ptr<table_t>> table_store_type;

sheet_item::sheet_item(document& doc, std::string_view _name, sheet_t sheet_index) :
    name(_name), data(doc, sheet_index) {}

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

class find_column_by_name
{
    std::string_view m_name;
public:
    find_column_by_name(std::string_view name) : m_name(name) {}

    bool operator() (const table_column_t& col) const
    {
        return col.name == m_name;
    }
};

void adjust_row_range(ixion::abs_range_t& range, const table_t& tab, ixion::table_areas_t areas)
{
    bool headers = (areas & ixion::table_area_headers);
    bool data    = (areas & ixion::table_area_data);
    bool totals  = (areas & ixion::table_area_totals);

    if (headers)
    {
        if (data)
        {
            if (totals)
            {
                // All areas.
                return;
            }

            // Headers + data
            range.last.row -= tab.totals_row_count;
            return;
        }

        if (totals)
        {
            // Header + total is invalid.
            range = ixion::abs_range_t(ixion::abs_range_t::invalid);
            return;
        }

        // Headers only.
        range.last.row = range.first.row;
        return;
    }

    if (data)
    {
        ++range.first.row;

        if (totals)
        {
            // Data + total
            return;
        }

        // Data only
        range.last.row -= tab.totals_row_count;
        return;
    }

    if (totals)
    {
        // Total only
        if (!tab.totals_row_count)
        {
            // This table has not total rows.  Return empty range.
            range = ixion::abs_range_t();
            return;
        }

        range.first.row = range.last.row - tab.totals_row_count - 1;
        return;
    }

    // Empty range.
    range = ixion::abs_range_t();
}

class table_handler : public ixion::iface::table_handler
{
    const ixion::model_context& m_context;
    const table_store_type& m_tables;

    const table_t* find_table(const ixion::abs_address_t& pos) const
    {
        auto it = m_tables.begin(), it_end = m_tables.end();
        for (; it != it_end; ++it)
        {
            const table_t* p = it->second.get();
            if (p->range.contains(pos))
                return p;
        }

        return nullptr;
    }

    pstring get_string(ixion::string_id_t sid) const
    {
        if (sid == ixion::empty_string_id)
            return pstring();

        const std::string* p = m_context.get_string(sid);
        if (!p || p->empty())
            return pstring();

        return pstring(&(*p)[0], p->size());
    }

    col_t find_column(const table_t& tab, std::string_view name, size_t offset) const
    {
        if (offset >= tab.columns.size())
            return -1;

        table_t::columns_type::const_iterator it_beg = tab.columns.begin();
        table_t::columns_type::const_iterator it_end = tab.columns.end();

        std::advance(it_beg, offset);
        table_t::columns_type::const_iterator it =
            std::find_if(it_beg, it_end, find_column_by_name(name));

        if (it == it_end)
            // not found.
            return -1;

        size_t dist = std::distance(tab.columns.begin(), it);
        return tab.range.first.column + dist;
    }

    ixion::abs_range_t get_range_from_table(
        const table_t& tab, ixion::string_id_t column_first, ixion::string_id_t column_last,
        ixion::table_areas_t areas) const
    {
        if (column_first != ixion::empty_string_id)
        {
            pstring col1_name = get_string(column_first);
            if (col1_name.empty())
                return ixion::abs_range_t(ixion::abs_range_t::invalid);

            col_t col1_index = find_column(tab, col1_name, 0);
            if (col1_index < 0)
                return ixion::abs_range_t(ixion::abs_range_t::invalid);

            if (column_last != ixion::empty_string_id)
            {
                pstring col2_name = get_string(column_last);
                if (!col2_name.empty())
                {
                    // column range table reference.
                    col_t col2_index = find_column(tab, col2_name, col1_index);
                    ixion::abs_range_t range = tab.range;
                    range.first.column = col1_index;
                    range.last.column = col2_index;
                    adjust_row_range(range, tab, areas);
                    return range;
                }
            }

            // single column table reference.
            ixion::abs_range_t range = tab.range;
            range.first.column = range.last.column = col1_index;
            adjust_row_range(range, tab, areas);
            return range;
        }

        return ixion::abs_range_t();
    }

public:
    table_handler(const ixion::model_context& cxt, const table_store_type& tables) :
        m_context(cxt), m_tables(tables) {}

    virtual ixion::abs_range_t get_range(
        const ixion::abs_address_t& pos, ixion::string_id_t column_first, ixion::string_id_t column_last,
        ixion::table_areas_t areas) const
    {
        const table_t* tab = find_table(pos);
        if (!tab)
            return ixion::abs_range_t(ixion::abs_range_t::invalid);

        return get_range_from_table(*tab, column_first, column_last, areas);

    }

    virtual ixion::abs_range_t get_range(
        ixion::string_id_t table, ixion::string_id_t column_first, ixion::string_id_t column_last,
        ixion::table_areas_t areas) const
    {
        pstring tab_name = get_string(table);
        if (tab_name.empty())
            // no table name given.
            return ixion::abs_range_t(ixion::abs_range_t::invalid);

        auto it = m_tables.find(tab_name);
        if (it == m_tables.end())
            // no table by this name found.
            return ixion::abs_range_t(ixion::abs_range_t::invalid);

        const table_t* tab = it->second.get();
        return get_range_from_table(*tab, column_first, column_last, areas);
    }
};

typedef std::vector<std::unique_ptr<sheet_item>> sheet_items_type;

}

struct document_impl
{
    document_impl(const document_impl&) = delete;
    document_impl& operator=(const document_impl&) = delete;

    document& m_doc;

    document_config m_doc_config;
    string_pool m_string_pool;
    ixion::model_context m_context;
    date_time_t m_origin_date;
    sheet_items_type m_sheets;
    styles m_styles;
    import_shared_strings* mp_strings;
    ixion::abs_range_set_t m_dirty_cells;

    pivot_collection m_pivots;

    std::unique_ptr<ixion::formula_name_resolver> mp_name_resolver_global;
    std::unique_ptr<ixion::formula_name_resolver> mp_name_resolver_named_exp_base;
    std::unique_ptr<ixion::formula_name_resolver> mp_name_resolver_named_range;
    formula_grammar_t m_grammar;

    table_store_type m_tables;
    table_handler m_table_handler;

    document_impl(document& doc, const range_size_t& sheet_size) :
        m_doc(doc),
        m_context({sheet_size.rows, sheet_size.columns}),
        m_styles(),
        mp_strings(new import_shared_strings(m_string_pool, m_context, m_styles)),
        m_pivots(doc),
        mp_name_resolver_global(ixion::formula_name_resolver::get(ixion::formula_name_resolver_t::excel_a1, &m_context)),
        m_grammar(formula_grammar_t::xlsx),
        m_table_handler(m_context, m_tables)
    {
        m_context.set_table_handler(&m_table_handler);
    }

    ~document_impl()
    {
        delete mp_strings;
    }
};

document::document(const range_size_t& sheet_size) : mp_impl(new document_impl(*this, sheet_size)) {}

document::~document() {}

import_shared_strings* document::get_shared_strings()
{
    return mp_impl->mp_strings;
}

const import_shared_strings* document::get_shared_strings() const
{
    return mp_impl->mp_strings;
}

styles& document::get_styles()
{
    return mp_impl->m_styles;
}

const styles& document::get_styles() const
{
    return mp_impl->m_styles;
}

pivot_collection& document::get_pivot_collection()
{
    return mp_impl->m_pivots;
}

const pivot_collection& document::get_pivot_collection() const
{
    return mp_impl->m_pivots;
}

ixion::model_context& document::get_model_context()
{
    return mp_impl->m_context;
}

const ixion::model_context& document::get_model_context() const
{
    return mp_impl->m_context;
}

const document_config& document::get_config() const
{
    return mp_impl->m_doc_config;
}

void document::set_config(const document_config& cfg)
{
    mp_impl->m_doc_config = cfg;
    ixion::config ixion_cfg = mp_impl->m_context.get_config();
    ixion_cfg.output_precision = cfg.output_precision;
    mp_impl->m_context.set_config(ixion_cfg);
}

string_pool& document::get_string_pool()
{
    return mp_impl->m_string_pool;
}

void document::insert_table(table_t* p)
{
    if (!p)
        return;

    pstring name = p->name;
    mp_impl->m_tables.insert(
        table_store_type::value_type(name, std::unique_ptr<table_t>(p)));
}

const table_t* document::get_table(std::string_view name) const
{
    auto it = mp_impl->m_tables.find(name);
    return it == mp_impl->m_tables.end() ? nullptr : it->second.get();
}

void document::finalize()
{
    std::for_each(mp_impl->m_sheets.begin(), mp_impl->m_sheets.end(),
        [](std::unique_ptr<sheet_item>& sh)
        {
            sh->data.finalize();
        }
    );
}

sheet* document::append_sheet(std::string_view sheet_name)
{
    pstring sheet_name_safe = mp_impl->m_string_pool.intern(sheet_name).first;
    sheet_t sheet_index = static_cast<sheet_t>(mp_impl->m_sheets.size());

    mp_impl->m_sheets.push_back(
        std::make_unique<sheet_item>(*this, sheet_name_safe, sheet_index));

    mp_impl->m_context.append_sheet(sheet_name_safe.str());

    return &mp_impl->m_sheets.back()->data;
}

sheet* document::get_sheet(std::string_view sheet_name)
{
    const sheet* sh = const_cast<const document*>(this)->get_sheet(sheet_name);
    return const_cast<sheet*>(sh);
}

const sheet* document::get_sheet(std::string_view sheet_name) const
{
    auto it = std::find_if(
        mp_impl->m_sheets.begin(), mp_impl->m_sheets.end(), find_sheet_by_name(sheet_name));

    if (it == mp_impl->m_sheets.end())
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
    if (static_cast<size_t>(sheet_pos) >= mp_impl->m_sheets.size())
        return nullptr;

    return &mp_impl->m_sheets[sheet_pos]->data;
}

void document::recalc_formula_cells()
{
    ixion::abs_range_set_t empty;

    ixion::model_context& cxt = get_model_context();
    std::vector<ixion::abs_range_t> sorted = ixion::query_and_sort_dirty_cells(
        cxt, empty, &mp_impl->m_dirty_cells);
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
    mp_impl->mp_strings->dump();

    cout << "number of sheets: " << mp_impl->m_sheets.size() << endl;

    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->m_sheets)
    {
        string this_file = outdir + '/' + sheet->name.str() + ".txt";

        ofstream file(this_file.c_str());
        if (!file)
        {
            cerr << "failed to create file: " << this_file << endl;
            return;
        }

        file << "---" << endl;
        file << "Sheet name: " << sheet->name << endl;
        sheet->data.dump_flat(file);
    }
}

void document::dump_check(ostream& os) const
{
    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->m_sheets)
        sheet->data.dump_check(os, sheet->name);
}

void document::dump_html(const string& outdir) const
{
    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->m_sheets)
    {
        string this_file = outdir + '/' + sheet->name.str() + ".html";

        ofstream file(this_file.c_str());
        if (!file)
        {
            cerr << "failed to create file: " << this_file << endl;
            return;
        }

        sheet->data.dump_html(file);
    }
}

void document::dump_json(const string& outdir) const
{
    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->m_sheets)
    {
        string this_file = outdir + '/' + sheet->name.str() + ".json";

        ofstream file(this_file.c_str());
        if (!file)
        {
            cerr << "failed to create file: " << this_file << endl;
            return;
        }

        sheet->data.dump_json(file);
    }
}

void document::dump_csv(const std::string& outdir) const
{
    for (const std::unique_ptr<sheet_item>& sheet : mp_impl->m_sheets)
    {
        string this_file = outdir + '/' + sheet->name.str() + ".csv";

        ofstream file(this_file.c_str());
        if (!file)
        {
            cerr << "failed to create file: " << this_file << endl;
            return;
        }

        sheet->data.dump_csv(file);
    }
}

sheet_t document::get_sheet_index(std::string_view name) const
{
    auto it = std::find_if(
        mp_impl->m_sheets.begin(), mp_impl->m_sheets.end(), find_sheet_by_name(name));

    if (it == mp_impl->m_sheets.end())
        return ixion::invalid_sheet;

    auto it_beg = mp_impl->m_sheets.begin();
    size_t pos = std::distance(it_beg, it);
    return static_cast<sheet_t>(pos);
}

std::string_view document::get_sheet_name(sheet_t sheet_pos) const
{
    if (sheet_pos < 0)
        return std::string_view{};

    size_t pos = static_cast<size_t>(sheet_pos);
    if (pos >= mp_impl->m_sheets.size())
        return std::string_view{};

    return mp_impl->m_sheets[pos]->name;
}

range_size_t document::get_sheet_size() const
{
    ixion::rc_size_t ss = mp_impl->m_context.get_sheet_size();
    range_size_t ret;
    ret.rows = ss.row;
    ret.columns = ss.column;
    return ret;
}

void document::set_sheet_size(const range_size_t& sheet_size)
{
    mp_impl->m_context.set_sheet_size({sheet_size.rows, sheet_size.columns});
}

size_t document::get_sheet_count() const
{
    return mp_impl->m_sheets.size();
}

void document::set_origin_date(int year, int month, int day)
{
    mp_impl->m_origin_date.year = year;
    mp_impl->m_origin_date.month = month;
    mp_impl->m_origin_date.day = day;
}

date_time_t document::get_origin_date() const
{
    return mp_impl->m_origin_date;
}

void document::set_formula_grammar(formula_grammar_t grammar)
{
    if (mp_impl->m_grammar == grammar)
        return;

    mp_impl->m_grammar = grammar;

    ixion::formula_name_resolver_t resolver_type_global = ixion::formula_name_resolver_t::unknown;
    ixion::formula_name_resolver_t resolver_type_named_exp_base = ixion::formula_name_resolver_t::unknown;
    ixion::formula_name_resolver_t resolver_type_named_range = ixion::formula_name_resolver_t::unknown;
    char arg_sep = 0;

    switch (mp_impl->m_grammar)
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

    mp_impl->mp_name_resolver_global.reset();
    mp_impl->mp_name_resolver_named_exp_base.reset();

    if (resolver_type_global != ixion::formula_name_resolver_t::unknown)
    {
        mp_impl->mp_name_resolver_global =
            ixion::formula_name_resolver::get(resolver_type_global, &mp_impl->m_context);

        if (resolver_type_named_exp_base != ixion::formula_name_resolver_t::unknown)
        {
            mp_impl->mp_name_resolver_named_exp_base =
                ixion::formula_name_resolver::get(resolver_type_named_exp_base, &mp_impl->m_context);
        }

        if (resolver_type_named_range != ixion::formula_name_resolver_t::unknown)
        {
            mp_impl->mp_name_resolver_named_range =
                ixion::formula_name_resolver::get(resolver_type_named_range, &mp_impl->m_context);
        }

        ixion::config cfg = mp_impl->m_context.get_config();
        cfg.sep_function_arg = arg_sep;
        cfg.output_precision = mp_impl->m_doc_config.output_precision;
        mp_impl->m_context.set_config(cfg);
    }
}

formula_grammar_t document::get_formula_grammar() const
{
    return mp_impl->m_grammar;
}

const ixion::formula_name_resolver* document::get_formula_name_resolver(formula_ref_context_t cxt) const
{
    switch (cxt)
    {
        case formula_ref_context_t::global:
            return mp_impl->mp_name_resolver_global.get();
        case formula_ref_context_t::named_expression_base:
            if (mp_impl->mp_name_resolver_named_exp_base)
                return mp_impl->mp_name_resolver_named_exp_base.get();
            break;
        case formula_ref_context_t::named_range:
            if (mp_impl->mp_name_resolver_named_range)
                return mp_impl->mp_name_resolver_named_range.get();
            break;
        default:
            ;
    }

    return mp_impl->mp_name_resolver_global.get();
}

void document::insert_dirty_cell(const ixion::abs_address_t& pos)
{
    mp_impl->m_dirty_cells.insert(pos);
}

}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
