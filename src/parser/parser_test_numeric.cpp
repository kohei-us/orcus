/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"
#include "orcus/global.hpp"
#include "numeric_parser.hpp"

#include <cassert>
#include <iostream>
#include <vector>
#include <limits>

using namespace orcus;
using std::cout;
using std::endl;

namespace {

struct check
{
    const char* str;
    size_t length;
    double expected;
};

const double invalid = std::numeric_limits<double>::quiet_NaN();

template<typename ParserT>
bool run_checks(const std::vector<check>& checks)
{
    for (const check& c : checks)
    {
        ParserT parser(c.str, c.str + c.length);
        double v = parser.parse();

        if (std::isnan(c.expected))
        {
            if (!std::isnan(v))
            {
                cout << "'" << c.str << "' was expected to be invalid, but parser parsed as if it was valid." << endl;
                return false;
            }
        }
        else
        {
            if (v != c.expected)
            {
                cout << "'" << c.str << "' was expected to be parsed as (" << c.expected << "), but the parser parsed it as (" << v << ")" << endl;
                return false;
            }
        }
    }

    return true;
}

}

void test_generic_number_parsing()
{
    using parser_type = detail::numeric_parser<detail::generic_parser_trait>;

    std::vector<check> checks = {
        { ORCUS_ASCII("-6.e3"), -6e3 },
        { ORCUS_ASCII("true"), invalid },
        { ORCUS_ASCII("1"), 1.0 },
        { ORCUS_ASCII("1.0"), 1.0 },
        { ORCUS_ASCII("-1.0"), -1.0 },
        { ORCUS_ASCII("-01"), -1.0 },
        { ORCUS_ASCII("2e2"), 200.0 },
        { ORCUS_ASCII("1.2"), 1.2 },
        { ORCUS_ASCII("-0.0001"), -0.0001 },
        { ORCUS_ASCII("-0.0"), 0.0 },
        { ORCUS_ASCII("+."), invalid },
        { ORCUS_ASCII("+e"), invalid },
        { ORCUS_ASCII("+e1"), invalid },
        { ORCUS_ASCII("+ "), invalid },
        { ORCUS_ASCII("- "), invalid }
    };

    assert(run_checks<parser_type>(checks));
}

void test_json_number_parsing()
{
    using parser_type = detail::numeric_parser<detail::json_parser_trait>;

    std::vector<check> checks = {
        { ORCUS_ASCII("-01"), invalid }, // Leading zeros are invalid.
    };

    assert(run_checks<parser_type>(checks));
}

int main()
{
    test_generic_number_parsing();
    test_json_number_parsing();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
