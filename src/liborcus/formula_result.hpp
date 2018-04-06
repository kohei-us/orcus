/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef ORCUS_FORMULA_RESULT_HPP
#define ORCUS_FORMULA_RESULT_HPP

#include <vector>
#include <cstdlib>

namespace orcus {

struct formula_result
{
    enum class result_type { empty, numeric, string, boolean };

    result_type type;

    union
    {
        double value_numeric;
        size_t value_string;
        bool value_boolean;
    };

    formula_result();
    formula_result(const formula_result& r);
    formula_result(double v);
    formula_result(size_t sid);
    formula_result(bool b);
};

class range_formula_results
{
    std::vector<formula_result> m_store;
    size_t m_rows;
    size_t m_cols;

    size_t to_array_pos(size_t row, size_t col) const;

public:
    range_formula_results(const range_formula_results&) = delete;
    range_formula_results(size_t rows, size_t cols);

    void set(size_t row, size_t col, const formula_result& v);
    const formula_result& get(size_t row, size_t col) const;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
