/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "document_impl.hpp"

#include <algorithm>

namespace orcus { namespace spreadsheet { namespace detail {

sheet_item::sheet_item(document& doc, std::string_view _name, sheet_t sheet_index) :
    name(_name), data(doc, sheet_index) {}

document_impl::document_impl(document& _doc, const range_size_t& sheet_size) :
    doc(_doc),
    context({sheet_size.rows, sheet_size.columns}),
    styles_store(),
    ss_store(context),
    pivots(doc),
    name_resolver_global(ixion::formula_name_resolver::get(ixion::formula_name_resolver_t::excel_a1, &context)),
    grammar(formula_grammar_t::xlsx),
    table_store(string_pool_store, context)
{
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
