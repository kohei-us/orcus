/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "formula_result.hpp"
#include "orcus/pstring.hpp"

#include <ostream>

namespace orcus {

formula_result::formula_result() : type(result_type::empty) {}
formula_result::formula_result(const formula_result& r) :
    type(r.type)
{
    switch (type)
    {
        case result_type::numeric:
            value_numeric = r.value_numeric;
            break;
        case result_type::boolean:
            value_boolean = r.value_boolean;
            break;
        case result_type::string:
            value_string.p = r.value_string.p;
            value_string.n = r.value_string.n;
            break;
        case result_type::empty:
        default:
            ;
    }
}

formula_result::formula_result(double v) : type(result_type::numeric), value_numeric(v) {}
formula_result::formula_result(const char* p, size_t n) :
    type(result_type::string)
{
    value_string.p = p;
    value_string.n = n;
}

formula_result::formula_result(bool b) : type(result_type::boolean), value_boolean(b) {}

size_t range_formula_results::to_array_pos(size_t row, size_t col) const
{
    return row * m_cols + col;
}

range_formula_results::range_formula_results(size_t rows, size_t cols) :
    m_store(rows*cols), m_rows(rows), m_cols(cols) {}

void range_formula_results::set(size_t row, size_t col, const formula_result& v)
{
    size_t pos = to_array_pos(row, col);
    m_store[pos] = v;
}

const formula_result& range_formula_results::get(size_t row, size_t col) const
{
    size_t pos = to_array_pos(row, col);
    return m_store[pos];
}

size_t range_formula_results::row_size() const
{
    return m_rows;
}

size_t range_formula_results::col_size() const
{
    return m_cols;
}

std::ostream& operator<< (std::ostream& os, const formula_result& v)
{
    switch (v.type)
    {
        case formula_result::result_type::numeric:
            os << v.value_numeric;
            break;
        case formula_result::result_type::boolean:
            os << v.value_boolean;
            break;
        case formula_result::result_type::string:
            os << pstring(v.value_string.p, v.value_string.n);
            break;
        case formula_result::result_type::empty:
            break;
    }
    return os;
}

std::ostream& operator<< (std::ostream& os, const orcus::range_formula_results& res)
{
    os << "{ ";
    size_t col_pos = 0;
    for (const formula_result& v : res.m_store)
    {
        if (col_pos == res.m_cols)
        {
            os << " | ";
            col_pos = 0;
        }
        else if (col_pos > 0)
            os << ", ";

        os << v;
        ++col_pos;
    }

    os << " }";

    return os;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
