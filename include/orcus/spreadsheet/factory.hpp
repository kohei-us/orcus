/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_IMPORT_FACTORY_HPP
#define INCLUDED_ORCUS_SPREADSHEET_IMPORT_FACTORY_HPP

#include <orcus/spreadsheet/import_interface.hpp>
#include <orcus/spreadsheet/import_interface_styles.hpp>
#include <orcus/spreadsheet/export_interface.hpp>
#include <orcus/env.hpp>

#include <memory>

namespace orcus {

class string_pool;

namespace spreadsheet {

class document;
class view;
class styles;

struct ORCUS_SPM_DLLPUBLIC import_factory_config
{
    /**
     * When the font cache is enabled, the import factory checks each incoming
     * font entry against the pool of existing font entries and insert it only
     * when an equal entry doesn't already exist in the pool.
     *
     * @note It should not be enabled for a file format that already has
     * font entries normalized, such as xlsx.
     */
    bool enable_font_cache = true;

    import_factory_config();
    import_factory_config(const import_factory_config& other);
    ~import_factory_config();

    import_factory_config& operator=(const import_factory_config& other);
};

/**
 * Wraps @ref document and @ref view stores.  This is to be used by the import
 * filter to populate the document and view stores.
 */
class ORCUS_SPM_DLLPUBLIC import_factory : public iface::import_factory
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    import_factory(document& doc);
    import_factory(document& doc, view& view_store);
    virtual ~import_factory();

    virtual iface::import_global_settings* get_global_settings() override;
    virtual iface::import_shared_strings* get_shared_strings() override;
    virtual iface::import_styles* get_styles() override;
    virtual iface::import_named_expression* get_named_expression() override;
    virtual iface::import_reference_resolver* get_reference_resolver(formula_ref_context_t cxt) override;
    virtual iface::import_pivot_cache_definition* create_pivot_cache_definition(
        orcus::spreadsheet::pivot_cache_id_t cache_id) override;
    virtual iface::import_pivot_cache_records* create_pivot_cache_records(
        orcus::spreadsheet::pivot_cache_id_t cache_id) override;
    virtual iface::import_sheet* append_sheet(sheet_t sheet_index, std::string_view name) override;
    virtual iface::import_sheet* get_sheet(std::string_view name) override;
    virtual iface::import_sheet* get_sheet(sheet_t sheet_index) override;
    virtual void finalize() override;

    void set_config(const import_factory_config& config);

    void set_default_row_size(row_t row_size);
    void set_default_column_size(col_t col_size);

    void set_character_set(character_set_t charset);
    character_set_t get_character_set() const;

    /**
     * When setting this flag to true, those formula cells with no cached
     * results will be re-calculated upon loading.
     *
     *
     * @param b value of this flag.
     */
    void set_recalc_formula_cells(bool b);

    void set_formula_error_policy(formula_error_policy_t policy);
};

/**
 * Wraps @ref styles store.  This is to be used by an import styles parser to
 * populate the styles store.
 */
class ORCUS_SPM_DLLPUBLIC import_styles : public iface::import_styles
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    import_styles(styles& styles_store, string_pool& sp);
    import_styles(std::shared_ptr<import_factory_config> config, styles& styles_store, string_pool& sp);
    virtual ~import_styles() override;

    virtual iface::import_font_style* start_font_style() override;
    virtual iface::import_fill_style* start_fill_style() override;
    virtual iface::import_border_style* start_border_style() override;
    virtual iface::import_cell_protection* start_cell_protection() override;
    virtual iface::import_number_format* start_number_format() override;
    virtual iface::import_xf* start_xf(xf_category_t cat) override;
    virtual iface::import_cell_style* start_cell_style() override;

    virtual void set_font_count(size_t n) override;
    virtual void set_fill_count(size_t n) override;
    virtual void set_border_count(size_t n) override;
    virtual void set_number_format_count(size_t n) override;
    virtual void set_xf_count(xf_category_t cat, size_t n) override;
    virtual void set_cell_style_count(size_t n) override;
};

/**
 * Wraps @ref document store and faciliates export of its content.
 *
 * @warning It currently provides very limited functionality especially when
 *          compared to that of the @ref import_factory.
 */
class ORCUS_SPM_DLLPUBLIC export_factory : public iface::export_factory
{
    struct impl;
    std::unique_ptr<impl> mp_impl;
public:
    export_factory(const document& doc);
    virtual ~export_factory();

    virtual const iface::export_sheet* get_sheet(std::string_view sheet_name) const override;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
