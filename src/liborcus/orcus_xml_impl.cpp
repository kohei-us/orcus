/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_xml_impl.hpp"

namespace orcus {

orcus_xml::impl::impl(xmlns_repository& _ns_repo) :
    im_factory(nullptr),
    ex_factory(nullptr),
    ns_repo(_ns_repo),
    ns_cxt_map(ns_repo.create_context()),
    map_tree(ns_repo),
    sheet_count(0) {}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
