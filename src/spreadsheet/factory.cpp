/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/factory.hpp"

#include <orcus/spreadsheet/shared_strings.hpp>
#include <orcus/spreadsheet/styles.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/view.hpp>
#include <orcus/exception.hpp>
#include <orcus/global.hpp>
#include <orcus/string_pool.hpp>
#include "pstring.hpp"

#include "factory_pivot.hpp"
#include "factory_sheet.hpp"
#include "global_settings.hpp"

#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_tokens.hpp>
#include <ixion/formula.hpp>
#include <ixion/model_context.hpp>
#include <sstream>
#include <iostream>
#include <unordered_map>

namespace orcus { namespace spreadsheet {

namespace {

class import_ref_resolver : public iface::import_reference_resolver
{
    document& m_doc;
    const ixion::formula_name_resolver* m_resolver;

public:
    import_ref_resolver(document& doc) : m_doc(doc), m_resolver(nullptr) {}

    void set_formula_ref_context(formula_ref_context_t cxt)
    {
        m_resolver = m_doc.get_formula_name_resolver(cxt);
    }

    virtual src_address_t resolve_address(const char* p, size_t n) override
    {
        if (!m_resolver)
            throw std::runtime_error("import_ref_resolver::resolve_address: formula resolver is null!");

        ixion::formula_name_t name = m_resolver->resolve({p, n}, ixion::abs_address_t());

        if (name.type != ixion::formula_name_t::cell_reference)
        {
            std::ostringstream os;
            os << std::string_view(p, n) << " is not a valid cell address.";
            throw orcus::invalid_arg_error(os.str());
        }

        auto addr = std::get<ixion::address_t>(name.value);
        src_address_t ret;
        ret.sheet = addr.sheet;
        ret.column = addr.column;
        ret.row = addr.row;
        return ret;
    }

    virtual src_range_t resolve_range(const char* p, size_t n) override
    {
        if (!m_resolver)
            throw std::runtime_error("import_ref_resolver::resolve_range: formula resolver is null!");

        ixion::formula_name_t name = m_resolver->resolve({p, n}, ixion::abs_address_t());

        switch (name.type)
        {
            case ixion::formula_name_t::range_reference:
            {
                auto range = std::get<ixion::range_t>(name.value);
                src_range_t ret;
                ret.first.sheet = range.first.sheet;
                ret.first.column = range.first.column;
                ret.first.row = range.first.row;
                ret.last.sheet = range.last.sheet;
                ret.last.column = range.last.column;
                ret.last.row = range.last.row;
                return ret;
            }
            case ixion::formula_name_t::cell_reference:
            {
                // Single cell address is still considered a valid "range".
                auto addr = std::get<ixion::address_t>(name.value);
                src_address_t cell;
                cell.sheet = addr.sheet;
                cell.column = addr.column;
                cell.row = addr.row;

                src_range_t ret;
                ret.first = cell;
                ret.last = cell;
                return ret;
            }
            default:
                ;
        }

        std::ostringstream os;
        os << std::string_view(p, n) << " is not a valid range address.";
        throw orcus::invalid_arg_error(os.str());
    }
};

class import_global_named_exp : public iface::import_named_expression
{
    document& m_doc;
    std::string_view m_name;
    ixion::abs_address_t m_base;
    ixion::formula_tokens_t m_tokens;

    void define(const char* p_name, size_t n_name, const char* p_exp, size_t n_exp, formula_ref_context_t ref_cxt)
    {
        string_pool& sp = m_doc.get_string_pool();
        m_name = sp.intern({p_name, n_name}).first;

        const ixion::formula_name_resolver* resolver = m_doc.get_formula_name_resolver(ref_cxt);
        assert(resolver);

        ixion::model_context& cxt = m_doc.get_model_context();
        m_tokens = ixion::parse_formula_string(cxt, m_base, *resolver, {p_exp, n_exp});
    }
public:
    import_global_named_exp(document& doc) : m_doc(doc), m_base(0, 0, 0) {}
    virtual ~import_global_named_exp() override {}

    virtual void set_base_position(const src_address_t& pos) override
    {
        m_base.sheet = pos.sheet;
        m_base.row = pos.row;
        m_base.column = pos.column;
    }

    virtual void set_named_expression(const char* p_name, size_t n_name, const char* p_exp, size_t n_exp) override
    {
        define(p_name, n_name, p_exp, n_exp, formula_ref_context_t::global);
    }

    virtual void set_named_range(const char* p_name, size_t n_name, const char* p_range, size_t n_range) override
    {
        define(p_name, n_name, p_range, n_range, formula_ref_context_t::named_range);
    }

    virtual void commit() override
    {
        ixion::model_context& cxt = m_doc.get_model_context();
        cxt.set_named_expression({m_name.data(), m_name.size()}, m_base, std::move(m_tokens));

        m_name = std::string_view{};
        m_base.sheet = 0;
        m_base.row = 0;
        m_base.column = 0;
    }
};

using sheet_ifaces_type = std::vector<std::unique_ptr<import_sheet>>;

}

struct import_factory::impl
{
    import_factory& m_envelope;
    document& m_doc;
    view* m_view;
    character_set_t m_charset;

    import_global_settings m_global_settings;
    import_pivot_cache_def m_pc_def;
    import_pivot_cache_records m_pc_records;
    import_ref_resolver m_ref_resolver;
    import_global_named_exp m_global_named_exp;
    import_styles m_styles;

    sheet_ifaces_type m_sheets;

    bool m_recalc_formula_cells;
    formula_error_policy_t m_error_policy;

    impl(import_factory& envelope, document& doc) :
        m_envelope(envelope),
        m_doc(doc),
        m_view(nullptr),
        m_charset(character_set_t::unspecified),
        m_global_settings(envelope, doc),
        m_pc_def(doc),
        m_pc_records(doc),
        m_ref_resolver(doc),
        m_global_named_exp(doc),
        m_styles(doc.get_styles(), doc.get_string_pool()),
        m_recalc_formula_cells(false),
        m_error_policy(formula_error_policy_t::fail) {}
};

import_factory::import_factory(document& doc) :
    mp_impl(std::make_unique<impl>(*this, doc)) {}

import_factory::import_factory(document& doc, view& view) :
    mp_impl(std::make_unique<impl>(*this, doc))
{
    // Store the optional view store.
    mp_impl->m_view = &view;
}

import_factory::~import_factory() {}

iface::import_global_settings* import_factory::get_global_settings()
{
    return &mp_impl->m_global_settings;
}

iface::import_shared_strings* import_factory::get_shared_strings()
{
    return mp_impl->m_doc.get_shared_strings();
}

iface::import_styles* import_factory::get_styles()
{
    return &mp_impl->m_styles;
}

iface::import_named_expression* import_factory::get_named_expression()
{
    return &mp_impl->m_global_named_exp;
}

iface::import_reference_resolver* import_factory::get_reference_resolver(formula_ref_context_t cxt)
{
    mp_impl->m_ref_resolver.set_formula_ref_context(cxt);
    return &mp_impl->m_ref_resolver;
}

iface::import_pivot_cache_definition* import_factory::create_pivot_cache_definition(
    pivot_cache_id_t cache_id)
{
    mp_impl->m_pc_def.create_cache(cache_id);
    return &mp_impl->m_pc_def;
}

iface::import_pivot_cache_records* import_factory::create_pivot_cache_records(
    orcus::spreadsheet::pivot_cache_id_t cache_id)
{
    pivot_collection& pcs = mp_impl->m_doc.get_pivot_collection();
    pivot_cache* pc = pcs.get_cache(cache_id);
    if (!pc)
        return nullptr;

    mp_impl->m_pc_records.set_cache(pc);
    return &mp_impl->m_pc_records;
}

iface::import_sheet* import_factory::append_sheet(
    sheet_t sheet_index, const char* sheet_name, size_t sheet_name_length)
{
    assert(sheet_index == static_cast<sheet_t>(mp_impl->m_doc.get_sheet_count()));

    sheet* sh = mp_impl->m_doc.append_sheet({sheet_name, sheet_name_length});

    if (!sh)
        return nullptr;

    sheet_view* sv = nullptr;
    if (mp_impl->m_view)
        sv = mp_impl->m_view->get_or_create_sheet_view(sheet_index);

    mp_impl->m_sheets.push_back(
        std::make_unique<import_sheet>(mp_impl->m_doc, *sh, sv));

    import_sheet* p = mp_impl->m_sheets.back().get();
    p->set_character_set(mp_impl->m_charset);
    p->set_fill_missing_formula_results(!mp_impl->m_recalc_formula_cells);
    p->set_formula_error_policy(mp_impl->m_error_policy);
    return p;
}

iface::import_sheet* import_factory::get_sheet(const char* sheet_name, size_t sheet_name_length)
{
    sheet_t si = mp_impl->m_doc.get_sheet_index(std::string_view(sheet_name, sheet_name_length));
    if (si == ixion::invalid_sheet)
        return nullptr;

    return mp_impl->m_sheets.at(si).get();
}

iface::import_sheet* import_factory::get_sheet(sheet_t sheet_index)
{
    if (sheet_index < 0 || size_t(sheet_index) >= mp_impl->m_sheets.size())
        return nullptr;

    return mp_impl->m_sheets[sheet_index].get();
}

void import_factory::finalize()
{
    mp_impl->m_doc.finalize();

    if (mp_impl->m_recalc_formula_cells)
        mp_impl->m_doc.recalc_formula_cells();
}

void import_factory::set_default_row_size(row_t row_size)
{
    range_size_t ss = mp_impl->m_doc.get_sheet_size();
    ss.rows = row_size;
    mp_impl->m_doc.set_sheet_size(ss);
}

void import_factory::set_default_column_size(col_t col_size)
{
    range_size_t ss = mp_impl->m_doc.get_sheet_size();
    ss.columns = col_size;
    mp_impl->m_doc.set_sheet_size(ss);
}

void import_factory::set_character_set(character_set_t charset)
{
    mp_impl->m_charset = charset;

    for (std::unique_ptr<import_sheet>& sheet : mp_impl->m_sheets)
        sheet->set_character_set(charset);
}

character_set_t import_factory::get_character_set() const
{
    return mp_impl->m_charset;
}

void import_factory::set_recalc_formula_cells(bool b)
{
    mp_impl->m_recalc_formula_cells = b;
}

void import_factory::set_formula_error_policy(formula_error_policy_t policy)
{
    mp_impl->m_error_policy = policy;
}

struct export_factory::impl
{
    const document& m_doc;

    std::vector<std::unique_ptr<export_sheet>> m_sheets;
    std::unordered_map<std::string_view, sheet_t> m_sheet_index_map;

    impl(const document& doc) : m_doc(doc) {}

    export_sheet* get_sheet(std::string_view name)
    {
        auto it = m_sheet_index_map.find(name);
        if (it != m_sheet_index_map.end())
        {
            // Instance for this sheet already exists.
            sheet_t sheet_pos = it->second;
            assert(size_t(sheet_pos) < m_sheets.size());
            return m_sheets[sheet_pos].get();
        }

        const sheet* sh = m_doc.get_sheet(name);
        if (!sh)
            return nullptr;

        sheet_t sheet_pos = m_sheets.size();
        m_sheets.emplace_back(std::make_unique<export_sheet>(m_doc, *sh));

        m_sheet_index_map.insert(
            std::make_pair(name, sheet_pos));

        return m_sheets[sheet_pos].get();
    }
};

export_factory::export_factory(const document& doc) :
    mp_impl(std::make_unique<impl>(doc)) {}

export_factory::~export_factory() {}

const iface::export_sheet* export_factory::get_sheet(std::string_view sheet_name) const
{
    return mp_impl->get_sheet(sheet_name);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
