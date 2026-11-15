/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "orcus/spreadsheet/types.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <utility>

namespace orcus {

class range_formula_results;

namespace spreadsheet {

namespace iface {

class import_array_formula;

}

}

void push_array_formula(
    spreadsheet::iface::import_array_formula* xformula,
    const spreadsheet::range_t& range, std::string_view formula,
    spreadsheet::formula_grammar_t grammar, const range_formula_results& results);

/**
 * Limit the count positions that begin at start to the positions from 0 to
 * size - 1, as for the rows or the columns of a sheet.
 *
 * @param start First position, which may lie outside the sheet.
 * @param count Number of positions.
 * @param size Number of positions the sheet has.
 *
 * @return First position and number of positions that lie inside the sheet,
 *         or nullopt if none does.
 */
inline std::optional<std::pair<std::int32_t, std::int32_t>> clip_span(
    std::int64_t start, std::int64_t count, std::int32_t size)
{
    std::int64_t first = std::max<std::int64_t>(start, 0);
    std::int64_t last = std::min<std::int64_t>(start + count - 1, std::int64_t{size} - 1);
    if (last < first)
        return std::nullopt;

    return std::make_pair(std::int32_t(first), std::int32_t(last - first + 1));
}

} // namespace orcus

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
