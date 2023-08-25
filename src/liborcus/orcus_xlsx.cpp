/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_xlsx.hpp"

#include "orcus/xml_namespace.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/exception.hpp"
#include "orcus/config.hpp"
#include "orcus/measurement.hpp"

#include "xlsx_types.hpp"
#include "xlsx_handler.hpp"
#include "xlsx_context.hpp"
#include "xlsx_workbook_context.hpp"
#include "xlsx_revision_context.hpp"
#include "ooxml_tokens.hpp"

#include "xml_stream_parser.hpp"
#include "xml_simple_stream_handler.hpp"
#include "opc_reader.hpp"
#include "ooxml_namespace_types.hpp"
#include "xlsx_session_data.hpp"
#include "opc_context.hpp"
#include "ooxml_global.hpp"
#include "spreadsheet_iface_util.hpp"
#include "ooxml_content_types.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <cstring>
#include <sstream>

using namespace std;

namespace orcus {

class xlsx_opc_handler : public opc_reader::part_handler
{
    orcus_xlsx& m_parent;
public:
    xlsx_opc_handler(orcus_xlsx& parent) : m_parent(parent) {}
    virtual ~xlsx_opc_handler() {}

    virtual bool handle_part(
        schema_t type, const std::string& dir_path, const std::string& file_name, opc_rel_extra* data)
    {
        if (type == SCH_od_rels_office_doc)
        {
            m_parent.read_workbook(dir_path, file_name);
            return true;
        }
        else if (type == SCH_od_rels_worksheet)
        {
            m_parent.read_sheet(dir_path, file_name, static_cast<xlsx_rel_sheet_info*>(data));
            return true;
        }
        else if (type == SCH_od_rels_shared_strings)
        {
            m_parent.read_shared_strings(dir_path, file_name);
            return true;
        }
        else if (type == SCH_od_rels_styles)
        {
            m_parent.read_styles(dir_path, file_name);
            return true;
        }
        else if (type == SCH_od_rels_drawing)
        {
            m_parent.read_drawing(dir_path, file_name);
            return true;
        }
        else if (type == SCH_od_rels_table)
        {
            m_parent.read_table(dir_path, file_name, static_cast<xlsx_rel_table_info*>(data));
            return true;
        }
        else if (type == SCH_od_rels_pivot_cache_def)
        {
            m_parent.read_pivot_cache_def(
                dir_path, file_name, static_cast<xlsx_rel_pivot_cache_info*>(data));
            return true;
        }
        else if (type == SCH_od_rels_pivot_cache_rec)
        {
            m_parent.read_pivot_cache_rec(
                dir_path, file_name,
                static_cast<const xlsx_rel_pivot_cache_record_info*>(data));
            return true;
        }
        else if (type == SCH_od_rels_pivot_table)
        {
            m_parent.read_pivot_table(dir_path, file_name);
            return true;
        }
        else if (type == SCH_od_rels_rev_headers)
        {
            m_parent.read_rev_headers(dir_path, file_name);
            return true;
        }
        else if (type == SCH_od_rels_rev_log)
        {
            m_parent.read_rev_log(dir_path, file_name);
            return true;
        }

        return false;
    }
};

struct orcus_xlsx::impl
{
    session_context m_cxt;
    xmlns_repository m_ns_repo;
    spreadsheet::iface::import_factory* mp_factory;
    xlsx_opc_handler m_opc_handler;
    opc_reader m_opc_reader;

    impl(spreadsheet::iface::import_factory* factory, orcus_xlsx& parent) :
        m_cxt(std::make_unique<xlsx_session_data>()),
        mp_factory(factory),
        m_opc_handler(parent),
        m_opc_reader(parent.get_config(), m_ns_repo, m_cxt, m_opc_handler) {}
};

orcus_xlsx::orcus_xlsx(spreadsheet::iface::import_factory* factory) :
    iface::import_filter(format_t::xlsx),
    mp_impl(std::make_unique<impl>(factory, *this))
{
    if (!factory)
        throw std::invalid_argument("factory instance is required.");

    spreadsheet::iface::import_global_settings* gs = factory->get_global_settings();
    if (gs)
    {
        gs->set_origin_date(1899, 12, 30);
        gs->set_default_formula_grammar(spreadsheet::formula_grammar_t::xlsx);
    }

    mp_impl->m_ns_repo.add_predefined_values(NS_ooxml_all);
    mp_impl->m_ns_repo.add_predefined_values(NS_opc_all);
    mp_impl->m_ns_repo.add_predefined_values(NS_misc_all);
}

orcus_xlsx::~orcus_xlsx() {}

bool orcus_xlsx::detect(const unsigned char* blob, size_t size)
{
    zip_archive_stream_blob stream(blob, size);
    zip_archive archive(&stream);

    try
    {
        archive.load();

        // Find and parse [Content_Types].xml which is required for OPC package.
        std::vector<unsigned char> buf = archive.read_file_entry("[Content_Types].xml");

        if (buf.empty())
            return false;

        config opt(format_t::xlsx);
        xmlns_repository ns_repo;
        ns_repo.add_predefined_values(NS_opc_all);
        session_context session_cxt;
        xml_stream_parser parser(
            opt, ns_repo, opc_tokens, reinterpret_cast<const char*>(&buf[0]), buf.size());

        xml_simple_stream_handler handler(
            session_cxt, opc_tokens,
            std::make_unique<opc_content_types_context>(session_cxt, opc_tokens));
        parser.set_handler(&handler);
        parser.parse();

        opc_content_types_context& context =
            static_cast<opc_content_types_context&>(handler.get_context());

        std::vector<xml_part_t> parts;
        context.pop_parts(parts);

        if (parts.empty())
            return false;

        // See if we can find the workbook stream.
        xml_part_t workbook_part("/xl/workbook.xml", CT_ooxml_xlsx_sheet_main);
        return std::find(parts.begin(), parts.end(), workbook_part) != parts.end();
    }
    catch (const std::exception&)
    {
        return false;
    }
}

void orcus_xlsx::read_file(std::string_view filepath)
{
    std::unique_ptr<zip_archive_stream> stream(
        new zip_archive_stream_fd(std::string{filepath}.c_str()));
    mp_impl->m_opc_reader.read_file(std::move(stream));

    // Formulas need to be inserted to the document after the shared string
    // table get imported, because tokenization of formulas may add new shared
    // string instances.
    set_formulas_to_doc();

    mp_impl->mp_factory->finalize();
}

void orcus_xlsx::read_stream(std::string_view stream)
{
    std::unique_ptr<zip_archive_stream> blob(
        new zip_archive_stream_blob(
            reinterpret_cast<const uint8_t*>(stream.data()), stream.size()));
    mp_impl->m_opc_reader.read_file(std::move(blob));

    // Formulas need to be inserted to the document after the shared string
    // table get imported, because tokenization of formulas may add new shared
    // string instances.
    set_formulas_to_doc();

    mp_impl->mp_factory->finalize();
}

std::string_view orcus_xlsx::get_name() const
{
    return "xlsx";
}

void orcus_xlsx::set_formulas_to_doc()
{
    auto push_formula_result = [this](spreadsheet::iface::import_formula* formula, const formula_result& res)
    {
        switch (res.type)
        {
            case formula_result::result_type::numeric:
                formula->set_result_value(res.value_numeric);
                break;
            case formula_result::result_type::string:
                formula->set_result_string({res.value_string.p, res.value_string.n});
                break;
            case formula_result::result_type::empty:
                break;
            default:
            {
                if (get_config().debug)
                    std::cerr << "warning: unhandled formula result (orcus_xlsx::set_formulas_to_doc)" << std::endl;
            }
        }
    };

    auto& sdata = mp_impl->m_cxt.get_data<xlsx_session_data>();

    // Insert shared formulas first.
    for (auto& p : sdata.m_shared_formulas)
    {
        xlsx_session_data::shared_formula& sf = *p;
        spreadsheet::iface::import_sheet* sheet = mp_impl->mp_factory->get_sheet(sf.sheet);
        if (!sheet)
            continue;

        spreadsheet::iface::import_formula* formula = sheet->get_formula();
        if (!formula)
            continue;

        formula->set_position(sf.row, sf.column);
        if (sf.master)
            formula->set_formula(orcus::spreadsheet::formula_grammar_t::xlsx, sf.formula);
        formula->set_shared_formula_index(sf.identifier);

        push_formula_result(formula, sf.result);
        formula->commit();
    }

    // Insert regular (non-shared) formulas.
    for (auto& p : sdata.m_formulas)
    {
        xlsx_session_data::formula& f = *p;
        spreadsheet::iface::import_sheet* sheet = mp_impl->mp_factory->get_sheet(f.sheet);
        if (!sheet)
            continue;

        spreadsheet::iface::import_formula* formula = sheet->get_formula();
        if (!formula)
            continue;

        formula->set_position(f.ref.row, f.ref.column);
        formula->set_formula(orcus::spreadsheet::formula_grammar_t::xlsx, f.exp);

        push_formula_result(formula, f.result);
        formula->commit();
    }

    // Insert array formulas.
    for (auto& p : sdata.m_array_formulas)
    {
        xlsx_session_data::array_formula& af = *p;
        spreadsheet::iface::import_sheet* sheet = mp_impl->mp_factory->get_sheet(af.sheet);
        if (!sheet)
            continue;

        spreadsheet::iface::import_array_formula* xaf = sheet->get_array_formula();
        push_array_formula(xaf, af.ref, af.exp, spreadsheet::formula_grammar_t::xlsx, *af.results);
    }
}

namespace {

size_t get_schema_rank(const schema_t sch)
{
    using map_type = std::unordered_map <schema_t, size_t>;

    static const schema_t schema_rank[] = {
        SCH_od_rels_shared_strings,
        SCH_od_rels_pivot_cache_def,
        SCH_od_rels_worksheet,
        nullptr
    };

    static map_type rank_map;

    if (rank_map.empty())
    {
        // initialize it.
        size_t rank = 0;
        for (const schema_t* p = schema_rank; *p; ++rank, ++p)
        {
            rank_map.insert(
                map_type::value_type(*p, rank));
        }
    }

    auto it = rank_map.find(sch);
    return it == rank_map.end() ? numeric_limits<size_t>::max() : it->second;
}

}

void orcus_xlsx::read_workbook(const string& dir_path, const string& file_name)
{
    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
        cout << "read_workbook: file path = " << filepath << endl;

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    auto handler = std::make_unique<xml_simple_stream_handler>(
        mp_impl->m_cxt, ooxml_tokens,
        std::make_unique<xlsx_workbook_context>(mp_impl->m_cxt, ooxml_tokens, *mp_impl->mp_factory));

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());
    parser.set_handler(handler.get());
    parser.parse();

    // Get sheet info from the context instance.
    xlsx_workbook_context& context =
        static_cast<xlsx_workbook_context&>(handler->get_context());
    opc_rel_extras_t workbook_data;
    context.pop_workbook_info(workbook_data);
    if (get_config().debug)
    {
        for_each(workbook_data.data.begin(), workbook_data.data.end(),
            [](const opc_rel_extras_t::map_type::value_type& v)
            {
                const xlsx_rel_sheet_info* info =
                    dynamic_cast<const xlsx_rel_sheet_info*>(v.second.get());

                if (info)
                {
                    cout << "relationship id: " << v.first << "; sheet name: " << info->name << "; sheet id: " << info->id << endl;
                }

                const xlsx_rel_pivot_cache_info* info_pc =
                    dynamic_cast<const xlsx_rel_pivot_cache_info*>(v.second.get());

                if (info_pc)
                {
                    cout << "relationship id: " << v.first << "; pivot cache id: " << info_pc->id << endl;
                }
            }
        );
    }

    handler.reset();

    // Re-order the relation items so that shared strings get imported first,
    // pivot caches get imported before the sheets and so on.

    opc_reader::sort_compare_type sort_func =
        [](const opc_rel_t& left, const opc_rel_t& right)
        {
            size_t rank_left = get_schema_rank(left.type), rank_right = get_schema_rank(right.type);
            if (rank_left != rank_right)
                return rank_left < rank_right;

            std::string_view rid1 = left.rid, rid2 = right.rid;

            if (rid1.size() > 1 && rid2.size() > 1)
            {
                // numerical comparison of relation ID's.
                rid1 = std::string_view(rid1.data()+1, rid1.size()-1); // remove the 'r' prefix.
                rid2 = std::string_view(rid2.data()+1, rid2.size()-1); // remove the 'r' prefix.
                return to_long(rid1) < to_long(rid2);
            }

            // textural comparison of relation ID's.
            return left.rid < right.rid;
        };

    mp_impl->m_opc_reader.check_relation_part(file_name, &workbook_data, &sort_func);
}

void orcus_xlsx::read_sheet(
    const std::string& dir_path, const std::string& file_name, xlsx_rel_sheet_info* data)
{
    if (!data || !data->id)
        // Sheet ID must not be 0.
        return;

    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_sheet: file path = " << filepath << endl;
    }

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    if (get_config().debug)
    {
        cout << "relationship sheet data: " << endl;
        cout << "  sheet name: " << data->name << "  sheet ID: " << data->id << endl;
    }

    spreadsheet::iface::import_sheet* sheet = mp_impl->mp_factory->get_sheet(data->name);
    if (!sheet)
    {
        std::ostringstream os;
        os << "orcus_xlsx::read_sheet: ";
        os << "sheet named '" << data->name << "' doesn't exist.";
        throw general_error(os.str());
    }

    spreadsheet::iface::import_reference_resolver* resolver =
        mp_impl->mp_factory->get_reference_resolver(spreadsheet::formula_ref_context_t::global);
    if (!resolver)
        throw general_error("orcus_xlsx::read_sheet: reference resolver interface is not available.");

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());

    auto handler = std::make_unique<xlsx_sheet_xml_handler>(
        mp_impl->m_cxt, ooxml_tokens, data->id-1, *resolver, *sheet);

    parser.set_handler(handler.get());
    parser.parse();

    opc_rel_extras_t table_info;
    handler->pop_rel_extras(table_info);
    handler.reset();
    mp_impl->m_opc_reader.check_relation_part(file_name, &table_info);
}

void orcus_xlsx::read_shared_strings(const std::string& dir_path, const std::string& file_name)
{
    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_shared_strings: file path = " << filepath << endl;
    }

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());

    auto handler = std::make_unique<xml_simple_stream_handler>(
        mp_impl->m_cxt, ooxml_tokens,
        std::make_unique<xlsx_shared_strings_context>(
            mp_impl->m_cxt, ooxml_tokens, mp_impl->mp_factory->get_shared_strings()));

    parser.set_handler(handler.get());
    parser.parse();
}

void orcus_xlsx::read_styles(const std::string& dir_path, const std::string& file_name)
{
    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_styles: file path = " << filepath << endl;
    }

    spreadsheet::iface::import_styles* styles = mp_impl->mp_factory->get_styles();
    if (!styles)
        // Client code doesn't support styles.
        return;

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
        return;

    if (buffer.empty())
        return;

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());

    auto handler = std::make_unique<xml_simple_stream_handler>(
        mp_impl->m_cxt, ooxml_tokens,
        std::make_unique<xlsx_styles_context>(
            mp_impl->m_cxt, ooxml_tokens, mp_impl->mp_factory->get_styles()));

    parser.set_handler(handler.get());
    parser.parse();
}

void orcus_xlsx::read_table(const std::string& dir_path, const std::string& file_name, xlsx_rel_table_info* data)
{
    if (!data || !data->sheet_interface)
        return;

    spreadsheet::iface::import_table* table = data->sheet_interface->get_table();
    if (!table)
        // Client code doesn't support tables. No point going further.
        return;

    spreadsheet::iface::import_reference_resolver* resolver =
        mp_impl->mp_factory->get_reference_resolver(spreadsheet::formula_ref_context_t::global);

    if (!resolver)
        // This client doesn't support reference resolver, but is required.
        return;

    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_table: file path = " << filepath << endl;
    }

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
    {
        cerr << "failed to open zip stream: " << filepath << endl;
        return;
    }

    if (buffer.empty())
        return;

    auto handler = std::make_unique<xlsx_table_xml_handler>(
        mp_impl->m_cxt, ooxml_tokens, *table, *resolver);

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());
    parser.set_handler(handler.get());
    parser.parse();

    handler.reset();
}

void orcus_xlsx::read_pivot_cache_def(
    const std::string& dir_path, const std::string& file_name,
    const xlsx_rel_pivot_cache_info* data)
{
    if (!data)
    {
        if (get_config().debug)
        {
            cout << "---" << endl;
            cout << "required pivot cache relation info was not present." << endl;
        }
        return;
    }

    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_pivot_cache_def: file path = " << filepath
            << "; cache id = " << data->id << endl;
    }

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
    {
        cerr << "failed to open zip stream: " << filepath << endl;
        return;
    }

    if (buffer.empty())
        return;

    spreadsheet::iface::import_pivot_cache_definition* pcache =
        mp_impl->mp_factory->create_pivot_cache_definition(data->id);

    if (!pcache)
        // failed to create a cache instance for whatever reason.
        return;

    auto handler = std::make_unique<xlsx_pivot_cache_def_xml_handler>(
        mp_impl->m_cxt, ooxml_tokens, *pcache, data->id);

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());
    parser.set_handler(handler.get());
    parser.parse();

    opc_rel_extras_t pcache_info = handler->pop_rel_extras();

    handler.reset();
    mp_impl->m_opc_reader.check_relation_part(file_name, &pcache_info);
}

void orcus_xlsx::read_pivot_cache_rec(
    const std::string& dir_path, const std::string& file_name,
    const xlsx_rel_pivot_cache_record_info* data)
{
    if (!data)
    {
        if (get_config().debug)
        {
            cout << "---" << endl;
            cout << "required pivot cache record relation info was not present." << endl;
        }
        return;
    }

    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_pivot_cache_rec: file path = " << filepath << "; cache id = " << data->id << endl;
    }

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
    {
        cerr << "failed to open zip stream: " << filepath << endl;
        return;
    }

    if (buffer.empty())
        return;

    spreadsheet::iface::import_pivot_cache_records* pcache_records =
        mp_impl->mp_factory->create_pivot_cache_records(data->id);

    if (!pcache_records)
        return;

    auto handler = std::make_unique<xlsx_pivot_cache_rec_xml_handler>(
        mp_impl->m_cxt, ooxml_tokens, *pcache_records);

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());
    parser.set_handler(handler.get());
    parser.parse();

    handler.reset();
}

void orcus_xlsx::read_pivot_table(const std::string& dir_path, const std::string& file_name)
{
    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_pivot_table: file path = " << filepath << endl;
    }

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
    {
        cerr << "failed to open zip stream: " << filepath << endl;
        return;
    }

    if (buffer.empty())
        return;

    auto handler = std::make_unique<xlsx_pivot_table_xml_handler>(mp_impl->m_cxt, ooxml_tokens);

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());
    parser.set_handler(handler.get());
    parser.parse();

    handler.reset();
    mp_impl->m_opc_reader.check_relation_part(file_name, nullptr);
}

void orcus_xlsx::read_rev_headers(const std::string& dir_path, const std::string& file_name)
{
    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_rev_headers: file path = " << filepath << endl;
    }

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
    {
        cerr << "failed to open zip stream: " << filepath << endl;
        return;
    }

    if (buffer.empty())
        return;

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());

    auto handler = std::make_unique<xml_simple_stream_handler>(
        mp_impl->m_cxt, ooxml_tokens,
        std::make_unique<xlsx_revheaders_context>(mp_impl->m_cxt, ooxml_tokens));

    parser.set_handler(handler.get());
    parser.parse();

    handler.reset();
    mp_impl->m_opc_reader.check_relation_part(file_name, nullptr);
}

void orcus_xlsx::read_rev_log(const std::string& dir_path, const std::string& file_name)
{
    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_rev_log: file path = " << filepath << endl;
    }

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
    {
        cerr << "failed to open zip stream: " << filepath << endl;
        return;
    }

    if (buffer.empty())
        return;

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());

    auto handler = std::make_unique<xml_simple_stream_handler>(
        mp_impl->m_cxt, ooxml_tokens,
        std::make_unique<xlsx_revlog_context>(mp_impl->m_cxt, ooxml_tokens));

    parser.set_handler(handler.get());
    parser.parse();

    handler.reset();
}

void orcus_xlsx::read_drawing(const std::string& dir_path, const std::string& file_name)
{
    std::string filepath = resolve_file_path(dir_path, file_name);
    if (get_config().debug)
    {
        cout << "---" << endl;
        cout << "read_drawing: file path = " << filepath << endl;
    }

    vector<unsigned char> buffer;
    if (!mp_impl->m_opc_reader.open_zip_stream(filepath, buffer))
    {
        cerr << "failed to open zip stream: " << filepath << endl;
        return;
    }

    if (buffer.empty())
        return;

    auto handler = std::make_unique<xlsx_drawing_xml_handler>(
        mp_impl->m_cxt, ooxml_tokens);

    xml_stream_parser parser(
        get_config(), mp_impl->m_ns_repo, ooxml_tokens,
        reinterpret_cast<const char*>(&buffer[0]), buffer.size());
    parser.set_handler(handler.get());
    parser.parse();

    handler.reset();
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
