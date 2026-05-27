/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test_global.hpp"

#include <orcus/spreadsheet/document.hpp>
#include <orcus/spreadsheet/factory.hpp>
#include <orcus/spreadsheet/sheet.hpp>
#include <orcus/spreadsheet/shared_strings.hpp>

#include <climits>

namespace ss = orcus::spreadsheet;

void test_sheet()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::range_size_t ssize{200, 10};
    ss::document doc{ssize};

    auto* sh = doc.append_sheet("Sheet 1");
    assert(sh);
    sh->set_string(2, 1, "string value");
    auto sid = sh->get_string_identifier(2, 1);
    const auto* s = doc.get_shared_strings().get_string(sid);
    assert(s);
    assert(*s == "string value");
}

void test_array_formula_malformed_range()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::range_size_t ssize{200, 10};
    ss::document doc{ssize};
    ss::import_factory factory{doc};

    auto* import_sh = factory.append_sheet(0, "Sheet 1");
    assert(import_sh);

    const auto drive = [&](const ss::range_t& range)
    {
        auto* af = import_sh->get_array_formula();
        assert(af);
        af->set_range(range);
        af->set_result_value(0, 0, 1.0);
        af->commit();
    };

    // Inverted: last.row < first.row. last - first + 1 is 0 in int32_t,
    // which previously asked ixion::matrix for a 0xN allocation but left
    // m_range inverted so commit propagated nonsense to ixion.
    drive(ss::range_t{{1, 0}, {0, 0}});

    // Negative subtraction. last - first + 1 = -98 in int32_t, cast to
    // size_t demands ~2^64 bytes from ixion::matrix.
    drive(ss::range_t{{99, 0}, {0, 0}});

    // INT32_MAX last with zero first. last - first + 1 wraps signed
    // int32_t and the resulting size_t lands near 2^63.
    drive(ss::range_t{{0, 0}, {INT32_MAX, INT32_MAX}});

    // Wholly out of the configured sheet bounds (sheet is 200x10).
    drive(ss::range_t{{500, 50}, {600, 60}});

    // Negative first.
    drive(ss::range_t{{-1, -1}, {5, 5}});
}

int main()
{
    test_sheet();
    test_array_formula_malformed_range();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
