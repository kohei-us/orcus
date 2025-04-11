/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <ixion/formula_name_resolver.hpp>

namespace orcus { namespace spreadsheet { namespace detail {

class debug_state_context
{
    std::unique_ptr<ixion::formula_name_resolver> m_resolver;
public:
    debug_state_context();

    /**
     * Print 2D range data in simple A1 format for readability.
     */
    std::string print_range(const ixion::abs_rc_range_t& range) const;

    void ensure_yaml_string(std::ostream& os, std::string_view s) const;
};

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
