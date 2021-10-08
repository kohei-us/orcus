/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/orcus_import_xlsx.hpp"

#include "orcus/xml_namespace.hpp"
#include "orcus/global.hpp"
#include "orcus/spreadsheet/import_interface.hpp"
#include "orcus/config.hpp"

#include "xlsx_types.hpp"
#include "xlsx_handler.hpp"
#include "xlsx_context.hpp"
#include "ooxml_tokens.hpp"

#include "xml_stream_parser.hpp"
#include "ooxml_namespace_types.hpp"
#include "xlsx_session_data.hpp"
#include "ooxml_global.hpp"

namespace orcus {

void import_xlsx::read_table(
    std::string_view s,
    spreadsheet::iface::import_table& table,
    spreadsheet::iface::import_reference_resolver& resolver)
{
    if (s.empty())
        return;

    session_context cxt;
    auto handler = std::make_unique<xlsx_table_xml_handler>(cxt, ooxml_tokens, table, resolver);

    xmlns_repository ns_repo;
    ns_repo.add_predefined_values(NS_ooxml_all);
    ns_repo.add_predefined_values(NS_opc_all);
    ns_repo.add_predefined_values(NS_misc_all);

    orcus::config config(format_t::xlsx);
    xml_stream_parser parser(
        config, ns_repo, ooxml_tokens,
        s.data(), s.size());
    parser.set_handler(handler.get());
    parser.parse();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
