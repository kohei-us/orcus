/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_FORMAT_DETECTION_HPP
#define ORCUS_FORMAT_DETECTION_HPP

#include <orcus/env.hpp>
#include <orcus/types.hpp>

#include <cstdlib>
#include <memory>

namespace orcus {

namespace iface {

class import_filter;

}

namespace spreadsheet { namespace iface {

class import_factory;

}}

/**
 * Detect the format of a given document stream.
 *
 * @param strm document stream to detect the format of.
 */
ORCUS_DLLPUBLIC format_t detect(std::string_view strm);

/**
 * Create an instance of import_filter for a specified format.
 *
 * @param type Format type to create an instace of import_filter of.
 * @param factory Pointer to an import factory instance.  It must not be null.
 *
 * @return Pointer to an instance of import_filter for specified format.
 */
ORCUS_DLLPUBLIC std::shared_ptr<iface::import_filter> create_filter(
    format_t type, spreadsheet::iface::import_factory* factory);

} // namespace orcus

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
