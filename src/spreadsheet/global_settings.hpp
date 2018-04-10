/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_GLOBAL_SETTINGS_HPP
#define INCLUDED_ORCUS_SPREADSHEET_GLOBAL_SETTINGS_HPP

#include "orcus/spreadsheet/import_interface.hpp"

namespace orcus { namespace spreadsheet {

class document;
class import_factory;

struct import_global_settings_impl;

class import_global_settings : public spreadsheet::iface::import_global_settings
{
    import_global_settings_impl* mp_impl;

public:
    import_global_settings(import_factory& factory, document& doc);
    virtual ~import_global_settings() override;

    virtual void set_origin_date(int year, int month, int day) override;

    virtual void set_default_formula_grammar(orcus::spreadsheet::formula_grammar_t grammar) override;

    virtual orcus::spreadsheet::formula_grammar_t get_default_formula_grammar() const override;

    virtual void set_character_set(character_set_t charset) override;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
