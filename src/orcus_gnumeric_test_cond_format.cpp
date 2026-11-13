/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_gnumeric_test.hpp"

#include <orcus/spreadsheet/cond_format.hpp>

namespace ss = orcus::spreadsheet;
namespace test = orcus::test;

namespace {

test::rc_range_resolver to_range(ixion::formula_name_resolver_t::excel_a1);

} // anonymous namespace

void test_gnumeric_cond_format_basic()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/conditional-format/basic.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    const ss::styles& styles = doc->get_styles();

    const ss::sheet* sh = doc->get_sheet("Single");
    assert(sh);

    struct check
    {
        std::string_view range;
        ss::cond_format_operator_t op;
        std::vector<std::string_view> formulas;
        ss::color_t fill_color;
    };

    // one format per column, each with a single condition rule

    const check checks[] = {
        // range    operator                             formulas     fill color of the format to apply
        { "A2:A13", ss::cond_format_operator_t::greater, {"2"},       {0xFF, 0xFF, 0x99, 0xCC} },
        { "B2:B13", ss::cond_format_operator_t::less,    {"2"},       {0xFF, 0xFF, 0x99, 0x00} },
        { "C2:C13", ss::cond_format_operator_t::between, {"2", "3"},  {0xFF, 0xC0, 0xC0, 0xC0} },
        { "D2:D13", ss::cond_format_operator_t::equal,   {"2"},       {0xFF, 0x99, 0xCC, 0x00} },
    };

    const auto& formats = sh->get_cond_formats();
    assert(formats.size() == std::size(checks));

    for (std::size_t i = 0; i < std::size(checks); ++i)
    {
        const check& c = checks[i];
        const ss::cond_format_t& format = formats[i];

        assert(format.ranges.size() == 1);
        assert(format.ranges[0] == to_range(c.range));
        assert(format.rules.size() == 1);

        auto* rule = std::get_if<ss::cond_format_condition_rule_t>(&format.rules[0]);
        assert(rule);
        assert(rule->op == c.op);

        assert(rule->values.size() == c.formulas.size());
        for (std::size_t j = 0; j < c.formulas.size(); ++j)
            assert(rule->values[j].formula == c.formulas[j]);

        // the style of the condition is imported as a differential format
        assert(rule->dxf_id);
        const ss::cell_format_t* dxf = styles.get_dxf_format(*rule->dxf_id);
        assert(dxf);

        const ss::fill_t* fill = styles.get_fill(dxf->fill);
        assert(fill);
        assert(fill->pattern_type);
        assert(*fill->pattern_type == ss::fill_pattern_t::solid);
        assert(fill->fg_color);
        assert(*fill->fg_color == c.fill_color);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
