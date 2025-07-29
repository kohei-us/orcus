/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_json_filter.hpp"
#include <orcus/stream.hpp>

namespace ss = orcus::spreadsheet;

namespace orcus {

orcus_json_filter::orcus_json_filter(ss::iface::import_factory* im_fact) :
    iface::import_filter(format_t::json), m_core(im_fact) {}

orcus_json_filter::~orcus_json_filter() = default;

bool orcus_json_filter::detect(const unsigned char* blob, std::size_t size)
{
    return orcus_json::detect(blob, size);
}

void orcus_json_filter::read_stream(std::string_view stream)
{
    m_core.detect_map_definition(stream);
    m_core.read_stream(stream);
}

void orcus_json_filter::read_file(std::string_view filepath)
{
    file_content content(filepath);
    m_core.read_stream(content.str());
}

void orcus_json_filter::read_file(std::u16string_view filepath)
{
    file_content content(filepath);
    m_core.read_stream(content.str());
}

std::string_view orcus_json_filter::get_name() const
{
    return "json";
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
