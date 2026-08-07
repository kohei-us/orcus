/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/spreadsheet/tables.hpp>
#include <orcus/spreadsheet/table.hpp>
#include <orcus/string_pool.hpp>

#include <ixion/model_context.hpp>

#include <map>

namespace orcus { namespace spreadsheet {

struct tables::impl
{
    string_pool& sp;
    ixion::model_context& cxt;
    std::map<std::string_view, table_t> store;

    impl(string_pool& _sp, ixion::model_context& _cxt) : sp(_sp), cxt(_cxt) {}
};

tables::tables(string_pool& sp, ixion::model_context& context) : mp_impl(std::make_unique<impl>(sp, context)) {}
tables::~tables() = default;

void tables::insert(ixion::table_t core, table_t pres)
{
    std::string_view name = mp_impl->sp.intern(core.name).first;

    // The ixion model context validates the name and the range of the table.
    mp_impl->cxt.set_table(std::move(core));

    pres.name = name;
    mp_impl->store.insert_or_assign(name, std::move(pres));
}

const table_t* tables::get(std::string_view name) const
{
    auto it = mp_impl->store.find(name);
    if (it == mp_impl->store.end())
        return nullptr;

    return &it->second;
}

std::map<std::string_view, const table_t*> tables::get_by_sheet(sheet_t pos) const
{
    std::map<std::string_view, const table_t*> ret;

    for (const ixion::table_t* p : mp_impl->cxt.get_tables(pos))
    {
        auto it = mp_impl->store.find(p->name);
        if (it != mp_impl->store.end())
            ret.insert_or_assign(it->first, &it->second);
    }

    return ret;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
