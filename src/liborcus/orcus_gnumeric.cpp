/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_gnumeric.hpp"
#include "orcus/xml_namespace.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/stream.hpp"
#include "orcus/config.hpp"

#include "xml_stream_parser.hpp"
#include "gnumeric_handler.hpp"
#include "gnumeric_tokens.hpp"
#include "gnumeric_namespace_types.hpp"
#include "gnumeric_detection_handler.hpp"
#include "session_context.hpp"
#include "detection_result.hpp"

#define ORCUS_DEBUG_GNUMERIC 0
#define BOOST_IOSTREAMS_NO_LIB 1

#include <iostream>
#include <string>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>

using namespace std;

namespace orcus {

namespace {

bool decompress_gzip(const char* buffer, size_t size, string& decompressed)
{
    string buf;

    try
    {
        boost::iostreams::filtering_ostream os;
        os.push(boost::iostreams::gzip_decompressor());
        os.push(boost::iostreams::back_inserter(buf));
        boost::iostreams::write(os, buffer, size);
        os.flush();
    }
    catch (const exception&)
    {
        return false;
    }

    buf.swap(decompressed);
    return true;
}

}

struct orcus_gnumeric::impl
{
    xmlns_repository m_ns_repo;
    session_context m_cxt;
    spreadsheet::iface::import_factory* mp_factory;

    impl(spreadsheet::iface::import_factory* im_factory) :
        mp_factory(im_factory) {}

    void read_content_xml(std::string_view s, const config& conf)
    {
        xml_stream_parser parser(conf, m_ns_repo, gnumeric_tokens, s.data(), s.size());

        auto handler = std::make_unique<gnumeric_content_xml_handler>(
            m_cxt, gnumeric_tokens, mp_factory);

        parser.set_handler(handler.get());
        parser.parse();
    }
};

orcus_gnumeric::orcus_gnumeric(spreadsheet::iface::import_factory* factory) :
    iface::import_filter(format_t::gnumeric),
    mp_impl(std::make_unique<impl>(factory))
{
    mp_impl->m_ns_repo.add_predefined_values(NS_gnumeric_all);
}

orcus_gnumeric::~orcus_gnumeric()
{
}

bool orcus_gnumeric::detect(const unsigned char* buffer, size_t size)
{
    // Detect gnumeric format that's already in memory.

    string decompressed;
    if (!decompress_gzip(reinterpret_cast<const char*>(buffer), size, decompressed))
        return false;

    if (decompressed.empty())
        return false;

    // Parse this xml stream for detection.
    config opt(format_t::gnumeric);
    xmlns_repository ns_repo;
    ns_repo.add_predefined_values(NS_gnumeric_all);
    session_context cxt;
    xml_stream_parser parser(opt, ns_repo, gnumeric_tokens, &decompressed[0], decompressed.size());
    gnumeric_detection_handler handler(cxt, gnumeric_tokens);
    parser.set_handler(&handler);

    try
    {
        parser.parse();
    }
    catch (const detection_result& res)
    {
        return res.get_result();
    }
    catch (...) {}

    return false;
}

void orcus_gnumeric::read_file(std::string_view filepath)
{
#if ORCUS_DEBUG_GNUMERIC
    cout << "reading " << filepath << endl;
#endif

    file_content content(filepath);
    if (content.empty())
        return;

    read_stream(content.str());
}

void orcus_gnumeric::read_stream(std::string_view stream)
{
    if (stream.empty())
        return;

    std::string file_content;
    if (!decompress_gzip(stream.data(), stream.size(), file_content))
        return;

    if (auto* gs = mp_impl->mp_factory->get_global_settings(); gs)
    {
        gs->set_origin_date(1899, 12, 30);
        gs->set_default_formula_grammar(spreadsheet::formula_grammar_t::gnumeric);
    }

    mp_impl->read_content_xml(file_content, get_config());
    mp_impl->mp_factory->finalize();
}

std::string_view orcus_gnumeric::get_name() const
{
    return "gnumeric";
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
