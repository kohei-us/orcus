/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus/interface.hpp"
#include "orcus/config.hpp"
#include "orcus/global.hpp"

namespace orcus { namespace iface {

struct import_filter::impl
{
    orcus::config m_config;

    impl(format_t input) : m_config(input) {}
};

import_filter::import_filter(format_t input) : mp_impl(std::make_unique<impl>(input)) {}

import_filter::~import_filter() {}

void import_filter::set_config(const config& v)
{
    mp_impl->m_config = v;
}

const config& import_filter::get_config() const
{
    return mp_impl->m_config;
}

document_dumper::~document_dumper() {}

}}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
