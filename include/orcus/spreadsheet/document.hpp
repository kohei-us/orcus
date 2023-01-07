/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_DOCUMENT_HPP
#define INCLUDED_ORCUS_SPREADSHEET_DOCUMENT_HPP

#include "orcus/env.hpp"
#include "orcus/interface.hpp"
#include "orcus/spreadsheet/types.hpp"

#include <ostream>
#include <memory>

namespace ixion {

class formula_name_resolver;
class model_context;
struct abs_address_t;

}

namespace orcus {

class string_pool;
struct date_time_t;

namespace spreadsheet {

class shared_strings;
class styles;
class pivot_collection;
class sheet;
class import_factory;

struct document_config;
struct table_t;

namespace detail {

struct document_impl;

}

/**
 * Store spreadsheet document content.  It uses the @p model_context class
 * from the ixion library to store raw cell values required in the computation
 * of formula expressions.
 */
class ORCUS_SPM_DLLPUBLIC document : public orcus::iface::document_dumper
{
    friend class sheet;
    friend class import_factory;

public:
    document(const document&) = delete;
    document& operator= (const document&) = delete;

    document(const range_size_t& sheet_size);
    ~document();

    /** See @ref iface::document_dumper. */
    virtual void dump(dump_format_t format, const std::string& output) const override;

    /** See @ref iface::document_dumper. */
    virtual void dump_check(std::ostream& os) const override;

    shared_strings& get_shared_strings();
    const shared_strings& get_shared_strings() const;

    styles& get_styles();
    const styles& get_styles() const;

    pivot_collection& get_pivot_collection();
    const pivot_collection& get_pivot_collection() const;

    sheet* append_sheet(std::string_view sheet_name);
    sheet* get_sheet(std::string_view sheet_name);
    const sheet* get_sheet(std::string_view sheet_name) const;
    sheet* get_sheet(sheet_t sheet_pos);
    const sheet* get_sheet(sheet_t sheet_pos) const;

    /**
     * Clear document content, to make it empty.
     */
    void clear();

    /**
     * Calculate those formula cells that have been newly inserted and have
     * not yet been calculated.
     */
    void recalc_formula_cells();

    sheet_t get_sheet_index(std::string_view name) const;
    std::string_view get_sheet_name(sheet_t sheet_pos) const;

    range_size_t get_sheet_size() const;
    void set_sheet_size(const range_size_t& sheet_size);
    size_t get_sheet_count() const;

    void set_origin_date(int year, int month, int day);
    date_time_t get_origin_date() const;

    void set_formula_grammar(formula_grammar_t grammar);
    formula_grammar_t get_formula_grammar() const;

    const ixion::formula_name_resolver* get_formula_name_resolver(formula_ref_context_t cxt) const;

    ixion::model_context& get_model_context();
    const ixion::model_context& get_model_context() const;

    const document_config& get_config() const;
    void set_config(const document_config& cfg);

    string_pool& get_string_pool();
    const string_pool& get_string_pool() const;

    /**
     * Insert a new table object into the document.  The document will take
     * ownership of the inserted object after the call.  The object will get
     * inserted only when there is no pre-existing table object of the same
     * name.  The object not being inserted will be deleted.
     *
     * @param p table object to insert.
     */
    void insert_table(table_t* p);

    /**
     * Get a structure containing properties of a named table.
     *
     * @param name Name of the table.
     *
     * @return Pointer to the structure containing the properties of a named
     *         table, or @p nullptr if no such table exists for the given name.
     */
    const table_t* get_table(std::string_view name) const;

private:
    void dump_flat(const std::string& outdir) const;
    void dump_html(const ::std::string& outdir) const;
    void dump_json(const ::std::string& outdir) const;
    void dump_csv(const std::string& outdir) const;
    void dump_debug_state(const std::string& outdir) const;

    void finalize_import();
    void insert_dirty_cell(const ixion::abs_address_t& pos);

private:
    std::unique_ptr<detail::document_impl> mp_impl;
};

}}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
