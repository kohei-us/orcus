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

#include <ixion/model_context.hpp>
#include <ixion/address.hpp>
#include <ixion/types.hpp>

#include <climits>
#include <cmath>
#include <filesystem>
#include <limits>

namespace ss = orcus::spreadsheet;
namespace fs = std::filesystem;

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

void test_dump_unsafe_sheet_name()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path base = fs::temp_directory_path() / "orcus_h12_dump_test";
    fs::remove_all(base);
    fs::create_directories(base);

    fs::path outdir = base / "out";

    // A sheet named "../escapee" would compose outdir/../escapee.csv, i.e.
    // base/escapee.csv, escaping the chosen output directory. The unsafe
    // sheet must be skipped while the safe one still gets written.
    ss::range_size_t ssize{200, 10};
    ss::document doc{ssize};
    doc.append_sheet("../escapee");
    doc.append_sheet("Safe");

    doc.dump(orcus::dump_format_t::csv, outdir);

    assert(!fs::exists(base / "escapee.csv"));
    assert(fs::exists(outdir / "Safe.csv"));

    fs::remove_all(base);
}

void test_date_time_out_of_range_second()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::range_size_t ssize{200, 10};
    ss::document doc{ssize};

    doc.set_origin_date(1899, 12, 30);

    auto* sh = doc.append_sheet("Sheet 1");
    assert(sh);

    const ixion::model_context& cxt = doc.get_model_context();

    // an infinite second must be clamped, not cast to long
    sh->set_date_time(0, 0, 2020, 1, 1, 0, 0, std::numeric_limits<double>::infinity());
    double v_inf = cxt.get_numeric_value(ixion::abs_address_t{0, 0, 0});
    assert(std::isfinite(v_inf));

    // the clamped cell must land on the same day as the second-0 cell
    sh->set_date_time(1, 0, 2020, 1, 1, 0, 0, 0.0);
    double base = cxt.get_numeric_value(ixion::abs_address_t{0, 1, 0});
    assert(std::isfinite(base));
    assert(v_inf >= base && v_inf < base + 1.0);
}

void test_set_auto_numeric_bounds()
{
    ORCUS_TEST_FUNC_SCOPE;

    ss::range_size_t ssize{200, 10};
    ss::document doc{ssize};

    auto* sh = doc.append_sheet("Sheet 1");
    assert(sh);

    const ixion::model_context& cxt = doc.get_model_context();

    // The numeric text abuts trailing digits in the backing buffer. The parse
    // must stop at the view's end, so the cell is the number 12. An over-read
    // would see "1234", reject the unconsumed tail, and store it as a string.
    std::string backing = "1234";
    sh->set_auto(0, 0, std::string_view(backing.data(), 2));
    ixion::abs_address_t pos{0, 0, 0};
    assert(cxt.get_celltype(pos) == ixion::cell_t::numeric);
    assert(cxt.get_numeric_value(pos) == 12.0);
}

int main()
{
    test_sheet();
    test_array_formula_malformed_range();
    test_dump_unsafe_sheet_name();
    test_date_time_out_of_range_second();
    test_set_auto_numeric_bounds();

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
