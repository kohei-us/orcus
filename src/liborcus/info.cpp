/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <orcus/info.hpp>
#include <orcus/version.hpp>

namespace orcus {

int get_version_major()
{
    return ORCUS_VERSION_MAJOR;
}

int get_version_minor()
{
    return ORCUS_VERSION_MINOR;
}

int get_version_micro()
{
    return ORCUS_VERSION_MICRO;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
