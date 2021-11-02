/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xml.hpp"
#include "pstring.hpp"
#include "orcus/sax_parser_base.hpp"
#include "orcus/sax_parser.hpp"
#include "orcus/stream.hpp"
#include "orcus/xml_structure_tree.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/xml_writer.hpp"
#include "orcus/measurement.hpp"

#include "orcus_xml_impl.hpp"

#include <vector>
#include <iostream>

namespace orcus {

namespace {

class xml_map_sax_handler
{
    struct scope
    {
        pstring ns;
        pstring name;

        scope(const pstring& _ns, const pstring& _name) : ns(_ns), name(_name) {}
    };

    std::vector<sax::parser_attribute> m_attrs;
    std::vector<scope> m_scopes;
    orcus_xml& m_app;

public:

    xml_map_sax_handler(orcus_xml& app) : m_app(app) {}

    void doctype(const sax::doctype_declaration&) {}
    void start_declaration(const pstring& /*name*/) {}

    void end_declaration(const pstring& /*name*/)
    {
        m_attrs.clear();
    }

    void start_element(const sax::parser_element& elem);

    void end_element(const sax::parser_element& elem)
    {
        if (elem.name == "range")
            m_app.commit_range();

        m_scopes.pop_back();
    }

    void attribute(const sax::parser_attribute& attr)
    {
        m_attrs.push_back(attr);
    }

    void characters(const pstring&, bool) {}
};

void xml_map_sax_handler::start_element(const sax::parser_element& elem)
{
    pstring xpath, sheet, label;
    spreadsheet::row_t row = -1;
    spreadsheet::col_t col = -1;

    if (elem.name == "ns")
    {
        // empty alias is associated with default namespace.
        pstring alias, uri;
        bool default_ns = false;

        for (const sax::parser_attribute& attr : m_attrs)
        {
            if (attr.name == "alias")
                alias = attr.value;
            else if (attr.name == "uri")
                uri = attr.value;
            else if (attr.name == "default")
                default_ns = to_bool(attr.value);
        }

        if (!uri.empty())
            m_app.set_namespace_alias(alias, uri, default_ns);
    }
    else if (elem.name == "cell")
    {
        for (const sax::parser_attribute& attr : m_attrs)
        {
            if (attr.name == "path")
                xpath = attr.value;
            else if (attr.name == "sheet")
                sheet = attr.value;
            else if (attr.name == "row")
                row = strtol(attr.value.data(), nullptr, 10);
            else if (attr.name == "column")
                col = strtol(attr.value.data(), nullptr, 10);
        }

        m_app.set_cell_link(xpath, sheet, row, col);
    }
    else if (elem.name == "range")
    {
        for (const sax::parser_attribute& attr : m_attrs)
        {
            if (attr.name == "sheet")
                sheet = attr.value;
            else if (attr.name == "row")
                row = strtol(attr.value.data(), nullptr, 10);
            else if (attr.name == "column")
                col = strtol(attr.value.data(), nullptr, 10);
        }

        m_app.start_range(sheet, row, col);
    }
    else if (elem.name == "field")
    {
        for (const sax::parser_attribute& attr : m_attrs)
        {
            if (attr.name == "path")
                xpath = attr.value;
            else if (attr.name == "label")
                label = attr.value;
        }

        m_app.append_field_link(xpath, label);
    }
    else if (elem.name == "row-group")
    {
        for (const sax::parser_attribute& attr : m_attrs)
        {
            if (attr.name == "path")
            {
                xpath = attr.value;
                break;
            }
        }

        m_app.set_range_row_group(xpath);
    }
    else if (elem.name == "sheet")
    {
        pstring sheet_name;
        for (const sax::parser_attribute& attr : m_attrs)
        {
            if (attr.name == "name")
            {
                sheet_name = attr.value;
                break;
            }
        }

        if (!sheet_name.empty())
            m_app.append_sheet(sheet_name);
    }

    m_scopes.push_back(scope(elem.ns, elem.name));
    m_attrs.clear();
}

} // anonymous namespace

void orcus_xml::read_map_definition(std::string_view stream)
{
    try
    {
        xml_map_sax_handler handler(*this);
        sax_parser<xml_map_sax_handler> parser(stream.data(), stream.size(), handler);
        parser.parse();
    }
    catch (const parse_error& e)
    {
        std::ostringstream os;
        os << "Error parsing the map definition file:" << std::endl
            << std::endl
            << create_parse_error_output(stream, e.offset()) << std::endl
            << e.what();

        throw invalid_map_error(os.str());
    }
}

void orcus_xml::detect_map_definition(std::string_view stream)
{
    size_t range_count = 0;
    std::string sheet_name_prefix = "range-";

    xml_structure_tree::range_handler_type rh = [&](xml_table_range_t&& range)
    {
        // Build sheet name first and insert a new sheet.
        std::ostringstream os_sheet_name;
        os_sheet_name << sheet_name_prefix << range_count;
        std::string sheet_name = os_sheet_name.str();
        append_sheet(sheet_name);

        // Push the linked range.
        start_range(sheet_name, 0, 0);

        for (const auto& path : range.paths)
            append_field_link(path, pstring());

        for (const auto& row_group : range.row_groups)
            set_range_row_group(row_group);

        commit_range();

        ++range_count;
    };

    xmlns_repository repo;
    xmlns_context cxt = repo.create_context();
    xml_structure_tree structure(cxt);
    structure.parse(stream);

    // Register all namespace aliases first.
    for (const xmlns_id_t& ns : cxt.get_all_namespaces())
        set_namespace_alias(cxt.get_short_name(ns), pstring(ns));

    structure.process_ranges(rh);
}

void orcus_xml::write_map_definition(std::string_view stream, std::ostream& out) const
{
    xmlns_context cxt = mp_impl->ns_repo.create_context();
    xml_structure_tree tree(cxt);
    tree.parse(stream);

    xml_writer writer(mp_impl->ns_repo, out);
    xmlns_id_t default_ns = writer.add_namespace("", "https://gitlab.com/orcus/orcus/xml-map-definition");
    auto map_scope = writer.push_element_scope({default_ns, "map"});

    for (const xmlns_id_t& ns : cxt.get_all_namespaces())
    {
        writer.add_attribute({default_ns, "alias"}, cxt.get_short_name(ns));
        writer.add_attribute({default_ns, "uri"}, ns);
        writer.push_element_scope({default_ns, "ns"});
    }

    size_t range_count = 0;
    std::string sheet_name_prefix = "range-";

    xml_structure_tree::range_handler_type rh = [&](xml_table_range_t&& range)
    {
        std::ostringstream os_sheet_name;
        os_sheet_name << sheet_name_prefix << range_count;
        std::string sheet_name = os_sheet_name.str();

        writer.add_attribute({default_ns, "name"}, sheet_name);
        writer.push_element_scope({default_ns, "sheet"});

        writer.add_attribute({default_ns, "sheet"}, sheet_name);
        writer.add_attribute({default_ns, "row"}, "0");
        writer.add_attribute({default_ns, "column"}, "0");
        auto range_scope = writer.push_element_scope({default_ns, "range"});

        for (const auto& path : range.paths)
        {
            writer.add_attribute({default_ns, "path"}, path);
            writer.push_element_scope({default_ns, "field"});
        }

        for (const auto& row_group : range.row_groups)
        {
            writer.add_attribute({default_ns, "path"}, row_group);
            writer.push_element_scope({default_ns, "row-group"});
        }

        ++range_count;
    };

    tree.process_ranges(rh);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
