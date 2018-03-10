/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/spreadsheet/factory.hpp"

#include "orcus/spreadsheet/shared_strings.hpp"
#include "orcus/spreadsheet/styles.hpp"
#include "orcus/spreadsheet/sheet.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/view.hpp"
#include "orcus/spreadsheet/global_settings.hpp"
#include "orcus/exception.hpp"
#include "orcus/global.hpp"

#include "factory_pivot.hpp"
#include "factory_sheet.hpp"

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

public:
    import_ref_resolver(document& doc) :
        m_doc(doc)
    {}

    virtual address_t resolve_address(const char* p, size_t n) override
    {
        const ixion::formula_name_resolver* resolver = m_doc.get_formula_name_resolver();
        if (!resolver)
            throw std::runtime_error("import_ref_resolver::resolve_address: formula resolver is null!");

        ixion::formula_name_t name = resolver->resolve(p, n, ixion::abs_address_t());

        if (name.type != ixion::formula_name_t::cell_reference)
        {
            std::ostringstream os;
            os << pstring(p, n) << " is not a valid cell address.";
            throw orcus::invalid_arg_error(os.str());
        }

        address_t ret;
        ret.column = name.address.col;
        ret.row = name.address.row;
        return ret;
    }

    virtual range_t resolve_range(const char* p, size_t n) override
    {
        const ixion::formula_name_resolver* resolver = m_doc.get_formula_name_resolver();
        if (!resolver)
            throw std::runtime_error("import_ref_resolver::resolve_range: formula resolver is null!");

        ixion::formula_name_t name = resolver->resolve(p, n, ixion::abs_address_t());

        switch (name.type)
        {
            case ixion::formula_name_t::range_reference:
            {
                range_t ret;
                ret.first.column = name.range.first.col;
                ret.first.row = name.range.first.row;
                ret.last.column = name.range.last.col;
                ret.last.row = name.range.last.row;
                return ret;
            }
            case ixion::formula_name_t::cell_reference:
            {
                // Single cell address is still considered a valid "range".
                address_t cell;
                cell.column = name.address.col;
                cell.row = name.address.row;

                range_t ret;
                ret.first = cell;
                ret.last = cell;
                return ret;
            }
            default:
                ;
        }

        std::ostringstream os;
        os << pstring(p, n) << " is not a valid range address.";
        throw orcus::invalid_arg_error(os.str());
    }
};

class import_global_named_exp : public iface::import_named_expression
{
    document& m_doc;

public:
    import_global_named_exp(document& doc) : m_doc(doc) {}

    virtual void define_name(const char* p_name, size_t n_name, const char* p_exp, size_t n_exp) override
    {
        const ixion::formula_name_resolver* resolver = m_doc.get_formula_name_resolver();
        assert(resolver);

        ixion::model_context& cxt = m_doc.get_model_context();

        ixion::formula_tokens_t tokens =
            ixion::parse_formula_string(
                cxt, ixion::abs_address_t(0,0,0), *resolver, p_exp, n_exp);

        std::unique_ptr<ixion::formula_tokens_t> tokens_p =
            orcus::make_unique<ixion::formula_tokens_t>(std::move(tokens));

        cxt.set_named_expression(p_name, n_name, std::move(tokens_p));
    }
};

using sheet_ifaces_type = std::vector<std::unique_ptr<import_sheet>>;

}

struct import_factory::impl
{
    document& m_doc;
    view* m_view;
    row_t m_default_row_size;
    col_t m_default_col_size;

    import_global_settings m_global_settings;
    import_pivot_cache_def m_pc_def;
    import_pivot_cache_records m_pc_records;
    import_ref_resolver m_ref_resolver;
    import_global_named_exp m_global_named_exp;
    import_styles m_styles;

    sheet_ifaces_type m_sheets;

    impl(document& doc, row_t row_size, col_t col_size) :
        m_doc(doc),
        m_view(nullptr),
        m_default_row_size(row_size),
        m_default_col_size(col_size),
        m_global_settings(doc),
        m_pc_def(doc),
        m_pc_records(doc),
        m_ref_resolver(doc),
        m_global_named_exp(doc),
        m_styles(doc.get_styles(), doc.get_string_pool()) {}
};

import_factory::import_factory(document& doc, row_t row_size, col_t col_size) :
    mp_impl(orcus::make_unique<impl>(doc, row_size, col_size)) {}

import_factory::import_factory(document& doc, view& view, row_t row_size, col_t col_size) :
    mp_impl(orcus::make_unique<impl>(doc, row_size, col_size))
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

iface::import_reference_resolver* import_factory::get_reference_resolver()
{
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
    assert(sheet_index == static_cast<sheet_t>(mp_impl->m_doc.sheet_size()));

    sheet* sh = mp_impl->m_doc.append_sheet(
        pstring(sheet_name, sheet_name_length), mp_impl->m_default_row_size, mp_impl->m_default_col_size);

    if (!sh)
        return nullptr;

    sheet_view* sv = nullptr;
    if (mp_impl->m_view)
        sv = mp_impl->m_view->get_or_create_sheet_view(sheet_index);

    mp_impl->m_sheets.push_back(
        orcus::make_unique<import_sheet>(mp_impl->m_doc, *sh, sv));

    return mp_impl->m_sheets.back().get();
}

iface::import_sheet* import_factory::get_sheet(const char* sheet_name, size_t sheet_name_length)
{
    sheet_t si = mp_impl->m_doc.get_sheet_index(pstring(sheet_name, sheet_name_length));
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
}

void import_factory::set_default_row_size(row_t row_size)
{
    mp_impl->m_default_row_size = row_size;
}

void import_factory::set_default_column_size(col_t col_size)
{
    mp_impl->m_default_col_size = col_size;
}

struct export_factory_impl
{
    const document& m_doc;

    std::vector<std::unique_ptr<export_sheet>> m_sheets;
    std::unordered_map<pstring, sheet_t, pstring::hash> m_sheet_index_map;

    export_factory_impl(const document& doc) : m_doc(doc) {}

    export_sheet* get_sheet(const pstring& name)
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
        m_sheets.emplace_back(orcus::make_unique<export_sheet>(m_doc, *sh));

        m_sheet_index_map.insert(
            std::make_pair(name, sheet_pos));

        return m_sheets[sheet_pos].get();
    }
};

export_factory::export_factory(const document& doc) :
    mp_impl(new export_factory_impl(doc)) {}

export_factory::~export_factory()
{
    delete mp_impl;
}

const iface::export_sheet* export_factory::get_sheet(const char* sheet_name, size_t sheet_name_length) const
{
    pstring name(sheet_name, sheet_name_length);
    return mp_impl->get_sheet(name);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
