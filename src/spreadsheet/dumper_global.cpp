/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "dumper_global.hpp"
#include "number_format.hpp"

#include <ixion/formula_name_resolver.hpp>
#include <ixion/formula_result.hpp>
#include <ixion/cell.hpp>

namespace orcus { namespace spreadsheet { namespace detail {

void dump_cell_value(
    std::ostream& os, const ixion::model_context& cxt, const ixion::model_iterator::cell& cell,
    func_str_handler str_handler,
    func_empty_handler empty_handler)
{
    switch (cell.type)
    {
        case ixion::celltype_t::empty:
            empty_handler(os);
            break;
        case ixion::celltype_t::boolean:
        {
            os << (cell.value.boolean ? "true" : "false");
            break;
        }
        case ixion::celltype_t::numeric:
        {
            format_to_file_output(os, cell.value.numeric);
            break;
        }
        case ixion::celltype_t::string:
        {
            const std::string* p = cxt.get_string(cell.value.string);
            assert(p);
            str_handler(os, *p);
            break;
        }
        case ixion::celltype_t::formula:
        {
            assert(cell.value.formula);
            ixion::formula_result res;

            try
            {
                res = cell.value.formula->get_result_cache(
                    ixion::formula_result_wait_policy_t::throw_exception);
            }
            catch (const std::exception&)
            {
                os << "\"#RES!\"";
                break;
            }

            switch (res.get_type())
            {
                case ixion::formula_result::result_type::value:
                    format_to_file_output(os, res.get_value());
                break;
                case ixion::formula_result::result_type::string:
                {
                    const std::string& s = res.get_string();
                    str_handler(os, s);
                }
                break;
                case ixion::formula_result::result_type::error:
                    os << "\"#ERR!\"";
                break;
                default:
                    ;
            }
            break;
        }
        default:
            ;
    }
}

}}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
