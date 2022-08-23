/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xlsx_types.hpp"

#include "orcus/global.hpp"

#include <mdds/sorted_string_map.hpp>

namespace orcus {

namespace {

constexpr std::string_view str_unknown = "unknown";

namespace cell_type {

using map_type = mdds::sorted_string_map<xlsx_cell_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "b", xlsx_ct_boolean },
    { "e", xlsx_ct_error },
    { "inlineStr", xlsx_ct_inline_string },
    { "n", xlsx_ct_numeric },
    { "s", xlsx_ct_shared_string },
    { "str", xlsx_ct_formula_string }
};

const map_type& get()
{
    static const map_type map(entries, std::size(entries), xlsx_ct_unknown);
    return map;
}

} // namespace cell_type

namespace rca {

using map_type = mdds::sorted_string_map<xlsx_rev_row_column_action_t, mdds::string_view_map_entry>;

// Keys must be sorted.
constexpr map_type::entry entries[] = {
    { "deleteCol", xlsx_rev_rca_delete_column },
    { "deleteRow", xlsx_rev_rca_delete_row    },
    { "insertCol", xlsx_rev_rca_insert_column },
    { "insertRow", xlsx_rev_rca_insert_row    }
};

const map_type& get()
{
    static const map_type map(entries, std::size(entries), xlsx_rev_rca_unknown);
    return map;
}

} // namespace rca

} // anonymous namespace

xlsx_cell_t to_xlsx_cell_type(std::string_view s)
{
    return cell_type::get().find(s);
}

std::string_view to_string(xlsx_cell_t type)
{
    switch (type)
    {
        case xlsx_ct_boolean:
            return cell_type::entries[0].key;
        case xlsx_ct_error:
            return cell_type::entries[1].key;
        case xlsx_ct_inline_string:
            return cell_type::entries[2].key;
        case xlsx_ct_numeric:
            return cell_type::entries[3].key;
        case xlsx_ct_shared_string:
            return cell_type::entries[4].key;
        case xlsx_ct_formula_string:
            return cell_type::entries[5].key;
        default:
            ;
    }
    return str_unknown;
}

xlsx_rev_row_column_action_t to_xlsx_rev_row_column_action_type(std::string_view s)
{
    return rca::get().find(s);
}

std::string_view to_string(xlsx_rev_row_column_action_t type)
{
    switch (type)
    {
        case xlsx_rev_rca_delete_column:
            return rca::entries[0].key;
        case xlsx_rev_rca_delete_row:
            return rca::entries[1].key;
        case xlsx_rev_rca_insert_column:
            return rca::entries[2].key;
        case xlsx_rev_rca_insert_row:
            return rca::entries[3].key;
        case xlsx_rev_rca_unknown:
        default:
            ;
    }

    return str_unknown;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
