/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xls_xml_handler.hpp"
#include "xls_xml_context.hpp"

namespace orcus {

xls_xml_handler::xls_xml_handler(
    session_context& session_cxt, const tokens& t, spreadsheet::iface::import_factory* factory) :
    xml_stream_handler(session_cxt, t, std::make_unique<xls_xml_context>(session_cxt, t, factory))
{
}

xls_xml_handler::~xls_xml_handler() {}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
