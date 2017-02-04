/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __ORCUS_SPREADSHEET_IMPORT_FACTORY_HPP__
#define __ORCUS_SPREADSHEET_IMPORT_FACTORY_HPP__

#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/spreadsheet/export_interface.hpp"
#include "orcus/env.hpp"

namespace orcus { namespace spreadsheet {

class document;

struct import_factory_impl;

class ORCUS_SPM_DLLPUBLIC import_factory : public iface::import_factory
{
public:
    import_factory(document& doc, row_t row_size = 1048576, col_t col_size = 16384);
    virtual ~import_factory();

    virtual iface::import_global_settings* get_global_settings() override;
    virtual iface::import_shared_strings* get_shared_strings() override;
    virtual iface::import_styles* get_styles() override;
    virtual iface::import_reference_resolver* get_reference_resolver() override;
    virtual iface::import_pivot_cache_definition* create_pivot_cache_definition(
        orcus::spreadsheet::pivot_cache_id_t cache_id) override;
    virtual iface::import_pivot_cache_records* create_pivot_cache_records(
        orcus::spreadsheet::pivot_cache_id_t cache_id) override;
    virtual iface::import_sheet* append_sheet(const char* sheet_name, size_t sheet_name_length) override;
    virtual iface::import_sheet* get_sheet(const char* sheet_name, size_t sheet_name_length) override;
    virtual iface::import_sheet* get_sheet(sheet_t sheet_index) override;
    virtual void finalize() override;

private:
    import_factory_impl* mp_impl;
};

struct export_factory_impl;

class ORCUS_SPM_DLLPUBLIC export_factory : public iface::export_factory
{
public:
    export_factory(document& doc);
    virtual ~export_factory();

    virtual const iface::export_sheet* get_sheet(const char* sheet_name, size_t sheet_name_length) const;

private:
    export_factory_impl* mp_impl;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
