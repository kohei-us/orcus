/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ORCUS_SPREADSHEET_DUMPER_GLOBAL_HPP
#define INCLUDED_ORCUS_SPREADSHEET_DUMPER_GLOBAL_HPP

#include <ixion/model_context.hpp>
#include <mdds/multi_type_vector/collection.hpp>

#include <ostream>
#include <functional>

namespace orcus { namespace spreadsheet { namespace detail {

using columns_type = mdds::mtv::collection<ixion::column_store_t>;

using func_str_handler = std::function<void(std::ostream&, const std::string&)>;
using func_empty_handler = std::function<void(std::ostream&)>;

void dump_cell_value(
    std::ostream& os, const ixion::model_context& cxt,
    const columns_type::const_iterator::value_type& node,
    func_str_handler str_handler,
    func_empty_handler empty_handler);

}}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
