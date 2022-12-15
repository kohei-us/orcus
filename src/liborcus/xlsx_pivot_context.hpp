/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_XLSX_PIVOT_CONTEXT_HPP
#define ORCUS_XLSX_PIVOT_CONTEXT_HPP

#include "xml_context_base.hpp"
#include "ooxml_types.hpp"
#include "orcus/spreadsheet/types.hpp"

namespace orcus {

namespace spreadsheet { namespace iface {

class import_pivot_cache_definition;
class import_pivot_cache_field_group;
class import_pivot_cache_records;

}}

/**
 * Base context for pivotCacheDefinition[n].xml part, which defines the
 * structure of a pivot cache.
 */
class xlsx_pivot_cache_def_context : public xml_context_base
{
public:
    enum class source_type {
        unknown = 0,
        worksheet,
        external,
        consolidation,
        scenario
    };

private:
    spreadsheet::iface::import_pivot_cache_definition& m_pcache;
    spreadsheet::pivot_cache_id_t m_pcache_id;
    spreadsheet::iface::import_pivot_cache_field_group* m_pcache_field_group = nullptr;
    source_type m_source_type = source_type::unknown;
    bool m_field_item_used = true;

    opc_rel_extras_t m_pcache_info;

public:
    xlsx_pivot_cache_def_context(
        session_context& cxt, const tokens& tokens,
        spreadsheet::iface::import_pivot_cache_definition& pcache,
        spreadsheet::pivot_cache_id_t pcache_id);

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name);
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child);
    virtual void start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs);
    virtual bool end_element(xmlns_id_t ns, xml_token_t name);
    virtual void characters(std::string_view str, bool transient);

    opc_rel_extras_t pop_rel_extras();

private:
    void start_element_s(const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs);
    void end_element_s();

    void start_element_n(const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs);
    void end_element_n();

    void start_element_d(const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs);
    void end_element_d();

    void start_element_e(const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs);
    void end_element_e();

    void start_element_shared_items(const xml_token_pair_t& parent, const std::vector<xml_token_attr_t>& attrs);
};

/**
 * Context for pivotCacheRecords[n].xml part, which contains the records in
 * a pivot cache.
 */
class xlsx_pivot_cache_rec_context : public xml_context_base
{
    spreadsheet::iface::import_pivot_cache_records& m_pc_records;

public:
    xlsx_pivot_cache_rec_context(
        session_context& cxt, const tokens& tokens,
        spreadsheet::iface::import_pivot_cache_records& pc_records);

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name);
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child);
    virtual void start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs);
    virtual bool end_element(xmlns_id_t ns, xml_token_t name);
};

/**
 * Context for pivotTable[n].xml part which defines the structure of a pivot
 * table model.
 */
class xlsx_pivot_table_context : public xml_context_base
{
public:
    xlsx_pivot_table_context(session_context& cxt, const tokens& tokens);

    virtual xml_context_base* create_child_context(xmlns_id_t ns, xml_token_t name);
    virtual void end_child_context(xmlns_id_t ns, xml_token_t name, xml_context_base* child);
    virtual void start_element(xmlns_id_t ns, xml_token_t name, const::std::vector<xml_token_attr_t>& attrs);
    virtual bool end_element(xmlns_id_t ns, xml_token_t name);
    virtual void characters(std::string_view str, bool transient);
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
