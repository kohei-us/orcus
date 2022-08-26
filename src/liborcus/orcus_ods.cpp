/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/orcus_ods.hpp>
#include <orcus/xml_namespace.hpp>
#include <orcus/zip_archive.hpp>
#include <orcus/zip_archive_stream.hpp>
#include <orcus/measurement.hpp>

#include "xml_stream_parser.hpp"
#include "ods_content_xml_context.hpp"
#include "ods_session_data.hpp"
#include "odf_document_styles_context.hpp"
#include "odf_tokens.hpp"
#include "odf_styles.hpp"
#include "odf_namespace_types.hpp"
#include "session_context.hpp"

#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

namespace orcus {

struct orcus_ods::impl
{
    xmlns_repository ns_repo;
    session_context cxt;
    spreadsheet::iface::import_factory* xfactory;

    impl(spreadsheet::iface::import_factory* im_factory) :
        cxt(new ods_session_data), xfactory(im_factory) {}
};

orcus_ods::orcus_ods(spreadsheet::iface::import_factory* factory) :
    iface::import_filter(format_t::ods),
    mp_impl(std::make_unique<impl>(factory))
{
    mp_impl->ns_repo.add_predefined_values(NS_odf_all);
}

orcus_ods::~orcus_ods() {}

void orcus_ods::list_content(const zip_archive& archive)
{
    size_t num = archive.get_file_entry_count();
    cout << "number of files this archive contains: " << num << endl;

    for (size_t i = 0; i < num; ++i)
    {
        pstring filename = archive.get_file_entry_name(i);
        if (filename.empty())
            cout << "(empty)" << endl;
        else
            cout << filename << endl;
    }
}

void orcus_ods::read_styles(const zip_archive& archive)
{
    auto* xstyles = mp_impl->xfactory->get_styles();
    if (!xstyles)
        return;

    std::vector<unsigned char> buf;
    if (!archive.read_file_entry("styles.xml", buf))
    {
        std::cout << "failed to get stat on styles.xml" << std::endl;
        return;
    }

    xml_stream_parser parser(
        get_config(), mp_impl->ns_repo, odf_tokens,
        reinterpret_cast<const char*>(buf.data()), buf.size());

    auto& ods_data = static_cast<ods_session_data&>(*mp_impl->cxt.mp_data);
    auto context = std::make_unique<document_styles_context>(
        mp_impl->cxt, odf_tokens, ods_data.styles_map, xstyles);

    xml_stream_handler handler(mp_impl->cxt, odf_tokens, std::move(context));

    parser.set_handler(&handler);
    parser.parse();
}

void orcus_ods::read_content(const zip_archive& archive)
{
    vector<unsigned char> buf;
    if (!archive.read_file_entry("content.xml", buf))
    {
        cout << "failed to get stat on content.xml" << endl;
        return;
    }

    read_content_xml(&buf[0], buf.size());
}

void orcus_ods::read_content_xml(const unsigned char* p, size_t size)
{
    bool use_threads = true;

    if (const char* p_env = std::getenv("ORCUS_ODS_USE_THREADS"); p_env)
        use_threads = to_bool(p_env);

    auto context = std::make_unique<ods_content_xml_context>(
        mp_impl->cxt, odf_tokens, mp_impl->xfactory);

    if (use_threads)
    {
        threaded_xml_stream_parser parser(
            get_config(), mp_impl->ns_repo, odf_tokens,
            reinterpret_cast<const char*>(p), size);

        xml_stream_handler handler(mp_impl->cxt, odf_tokens, std::move(context));
        parser.set_handler(&handler);
        parser.parse();

        string_pool this_pool;
        parser.swap_string_pool(this_pool);
        mp_impl->cxt.m_string_pool.merge(this_pool);
    }
    else
    {
        xml_stream_parser parser(
            get_config(), mp_impl->ns_repo, odf_tokens,
            reinterpret_cast<const char*>(p), size);

        xml_stream_handler handler(mp_impl->cxt, odf_tokens, std::move(context));
        parser.set_handler(&handler);
        parser.parse();
    }
}

bool orcus_ods::detect(const unsigned char* blob, size_t size)
{
    zip_archive_stream_blob stream(blob, size);
    zip_archive archive(&stream);
    try
    {
        archive.load();
    }
    catch (const zip_error&)
    {
        // Not a valid zip archive.
        return false;
    }

    vector<unsigned char> buf;
    if (!archive.read_file_entry("mimetype", buf))
        // Failed to read 'mimetype' entry.
        return false;

    if (buf.empty())
        // mimetype is empty.
        return false;

    const char* mimetype = "application/vnd.oasis.opendocument.spreadsheet";
    size_t n = strlen(mimetype);
    if (buf.size() < n)
        return false;

    if (strncmp(mimetype, reinterpret_cast<const char*>(&buf[0]), n))
        // The mimetype content differs.
        return false;

    return true;
}

void orcus_ods::read_file(const std::string& filepath)
{
    zip_archive_stream_fd stream(filepath.data());
    read_file_impl(&stream);
}

void orcus_ods::read_stream(std::string_view stream)
{
    zip_archive_stream_blob blob(
        reinterpret_cast<const uint8_t*>(stream.data()), stream.size());
    read_file_impl(&blob);
}

void orcus_ods::read_file_impl(zip_archive_stream* stream)
{
    zip_archive archive(stream);
    archive.load();
    if (get_config().debug)
        list_content(archive);

    spreadsheet::formula_grammar_t old_grammar = spreadsheet::formula_grammar_t::unknown;

    spreadsheet::iface::import_global_settings* gs = mp_impl->xfactory->get_global_settings();
    if (gs)
    {
        old_grammar = gs->get_default_formula_grammar();
        gs->set_default_formula_grammar(spreadsheet::formula_grammar_t::ods);
    }

    read_styles(archive);
    read_content(archive);

    mp_impl->xfactory->finalize();

    if (gs)
        // This grammar will be used
        gs->set_default_formula_grammar(old_grammar);
}

std::string_view orcus_ods::get_name() const
{
    return "ods";
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
