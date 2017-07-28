/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dumper_global.hpp"

#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>

namespace orcus { namespace spreadsheet { namespace detail {

void dump_cell_value(
    std::ostream& os, const ixion::model_context& cxt,
    const columns_type::const_iterator::value_type& node,
    func_str_handler str_handler,
    func_empty_handler empty_handler)
{
    switch (node.type)
    {
        case ixion::element_type_empty:
            empty_handler(os);
            break;
        case ixion::element_type_boolean:
        {
            auto b = node.get<ixion::boolean_element_block>();
            os << (b ? "true" : "false");
            break;
        }
        case ixion::element_type_numeric:
        {
            auto v = node.get<ixion::numeric_element_block>();
            os << v;
            break;
        }
        case ixion::element_type_string:
        {
            ixion::string_id_t sindex = node.get<ixion::string_element_block>();
            const std::string* p = cxt.get_string(sindex);
            assert(p);
            str_handler(os, *p);
            break;
        }
        case ixion::element_type_formula:
        {
            const ixion::formula_cell* cell = node.get<ixion::formula_element_block>();
            assert(cell);

            const ixion::formula_result& res = cell->get_result_cache();

            switch (res.get_type())
            {
                case ixion::formula_result::result_type::value:
                    os << res.get_value();
                break;
                case ixion::formula_result::result_type::string:
                {
                    ixion::string_id_t sid = res.get_string();
                    const std::string* p = cxt.get_string(sid);
                    assert(p);
                    str_handler(os, *p);
                }
                break;
                case ixion::formula_result::result_type::error:
                    os << "\"#ERR!\"";
                break;
            }
            break;
        }
        default:
            ;
    }
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
