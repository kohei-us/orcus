/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "global_settings.hpp"
#include "orcus/spreadsheet/document.hpp"
#include "orcus/spreadsheet/factory.hpp"

namespace orcus { namespace spreadsheet {

struct import_global_settings::impl
{
    import_factory& m_factory;
    document& m_doc;

    impl(import_factory& factory, document& doc) :
        m_factory(factory), m_doc(doc) {}
};

import_global_settings::import_global_settings(import_factory& factory, document& doc) :
    mp_impl(std::make_unique<impl>(factory, doc)) {}

import_global_settings::~import_global_settings() {}

void import_global_settings::set_origin_date(int year, int month, int day)
{
    mp_impl->m_doc.set_origin_date(year, month, day);
}

void import_global_settings::set_default_formula_grammar(formula_grammar_t grammar)
{
    mp_impl->m_doc.set_formula_grammar(grammar);
}

formula_grammar_t import_global_settings::get_default_formula_grammar() const
{
    return mp_impl->m_doc.get_formula_grammar();
}

void import_global_settings::set_character_set(character_set_t charset)
{
    mp_impl->m_factory.set_character_set(charset);
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
