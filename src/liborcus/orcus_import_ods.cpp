/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_import_ods.hpp"

#include "orcus/xml_namespace.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/config.hpp"

#include "odf_styles_context.hpp"
#include "odf_tokens.hpp"
#include "odf_namespace_types.hpp"
#include "session_context.hpp"
#include "ods_session_data.hpp"

#include "xml_stream_parser.hpp"

namespace orcus {

void import_ods::read_styles(std::string_view s, spreadsheet::iface::import_styles* styles)
{
    if (!styles)
        return;

    if (s.empty())
        return;

    session_context cxt{std::make_unique<ods_session_data>()};
    auto context = std::make_unique<styles_context>(cxt, odf_tokens, styles);

    xml_stream_handler stream_handler(cxt, odf_tokens, std::move(context));

    xmlns_repository ns_repo;
    ns_repo.add_predefined_values(NS_odf_all);

    orcus::config config(format_t::ods);
    config.debug = true;
    xml_stream_parser parser(
        config, ns_repo, odf_tokens,
        s.data(), s.size());
    parser.set_handler(&stream_handler);
    parser.parse();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
