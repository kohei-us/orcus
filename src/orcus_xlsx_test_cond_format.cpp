/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_xlsx_test.hpp"

#include <orcus/spreadsheet/cond_format.hpp>

namespace ss = orcus::spreadsheet;
namespace test = orcus::test;

namespace {

test::rc_range_resolver to_range(ixion::formula_name_resolver_t::excel_a1);

ss::cond_format_value_t to_value(ss::cond_format_value_type_t type, std::string_view formula)
{
    ss::cond_format_value_t v;
    v.type = type;
    v.formula = formula;
    return v;
}

/**
 * Check the types and the formulas of the values of a rule.  The colors of
 * the values are not checked here.
 */
bool check_values(
    const std::vector<ss::cond_format_value_t>& values, const std::vector<ss::cond_format_value_t>& expected)
{
    if (values.size() != expected.size())
        return false;

    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (values[i].type != expected[i].type)
            return false;

        if (values[i].formula != expected[i].formula)
            return false;
    }

    return true;
}

const std::vector<ss::cond_format_t>& get_cond_formats(const ss::document& doc, std::string_view sheet_name)
{
    const ss::sheet* sh = doc.get_sheet(sheet_name);
    assert(sh);
    return sh->get_cond_formats();
}

} // anonymous namespace

void test_xlsx_cond_format_basic()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/conditional-format/basic.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path, false);

    {
        // one format per column, each with a single condition rule

        struct check
        {
            std::string_view range;
            ss::cond_format_operator_t op;
            std::size_t dxf_id;
            std::string_view rank;
            std::string_view text;
            std::vector<ss::cond_format_value_t> values;
        };

        const auto vt_unknown = ss::cond_format_value_type_t::unknown;

        const check checks[] = {
            // range    operator                                    dxf  rank  text  values
            { "A2:A13", ss::cond_format_operator_t::greater,        14,  {},   {},   {to_value(vt_unknown, "2")} },
            { "B2:B13", ss::cond_format_operator_t::less,           13,  {},   {},   {to_value(vt_unknown, "2")} },
            { "C2:C13", ss::cond_format_operator_t::between,        12,  {},   {},   {to_value(vt_unknown, "2"), to_value(vt_unknown, "3")} },
            { "D2:D13", ss::cond_format_operator_t::equal,          11,  {},   {},   {to_value(vt_unknown, "2")} },
            { "E2:E13", ss::cond_format_operator_t::contains,       10,  {},   "ck", {to_value(vt_unknown, R"(NOT(ISERROR(SEARCH("ck",E2))))")} },
            { "F2:F13", ss::cond_format_operator_t::duplicate,      9,   {},   {},   {} },
            { "G2:G13", ss::cond_format_operator_t::top_n,          8,   "5",  {},   {} },
            { "H2:H13", ss::cond_format_operator_t::top_percent,    7,   "30", {},   {} },
            { "I2:I13", ss::cond_format_operator_t::bottom_n,       6,   "5",  {},   {} },
            { "J2:J13", ss::cond_format_operator_t::bottom_percent, 5,   "30", {},   {} },
            { "K2:K13", ss::cond_format_operator_t::above_average,  4,   {},   {},   {} },
            { "L2:L13", ss::cond_format_operator_t::below_average,  3,   {},   {},   {} },
        };

        const auto& formats = get_cond_formats(*doc, "Single");
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
            assert(rule->dxf_id);
            assert(*rule->dxf_id == c.dxf_id);
            assert(rule->rank == c.rank);
            assert(rule->text == c.text);
            assert(rule->std_dev.empty());
            assert(check_values(rule->values, c.values));
        }
    }

    {
        // one format with two condition rules

        const auto& formats = get_cond_formats(*doc, "Multiple");
        assert(formats.size() == 1);

        const ss::cond_format_t& format = formats[0];
        assert(format.ranges.size() == 1);
        assert(format.ranges[0] == to_range("A2:A13"));
        assert(format.rules.size() == 2);

        auto* rule = std::get_if<ss::cond_format_condition_rule_t>(&format.rules[0]);
        assert(rule);
        assert(rule->op == ss::cond_format_operator_t::greater);
        assert(rule->dxf_id);
        assert(*rule->dxf_id == 2);
        assert(check_values(rule->values, {to_value(ss::cond_format_value_type_t::unknown, "2")}));

        rule = std::get_if<ss::cond_format_condition_rule_t>(&format.rules[1]);
        assert(rule);
        assert(rule->op == ss::cond_format_operator_t::equal);
        assert(rule->dxf_id);
        assert(*rule->dxf_id == 1);
        assert(check_values(rule->values, {to_value(ss::cond_format_value_type_t::unknown, "1")}));
    }
}

void test_xlsx_cond_format_data_bars()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/conditional-format/data-bars.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path, false);

    struct check
    {
        std::string_view range;
        ss::color_t color;
        bool show_value;
    };

    // preset data bar colors of Excel
    const ss::color_t blue{0xFF, 0x63, 0x8E, 0xC6};
    const ss::color_t green{0xFF, 0x63, 0xC3, 0x84};
    const ss::color_t red{0xFF, 0xFF, 0x55, 0x5A};
    const ss::color_t orange{0xFF, 0xFF, 0xB6, 0x28};
    const ss::color_t light_blue{0xFF, 0x00, 0x8A, 0xEF};
    const ss::color_t purple{0xFF, 0xD6, 0x00, 0x7B};

    const check checks[] = {
        // range    color       show value
        { "A2:A13", blue,       true },
        { "B2:B13", green,      true },
        { "C2:C13", red,        true },
        { "D2:D13", orange,     true },
        { "E2:E13", light_blue, true },
        { "F2:F13", purple,     true },
        { "G2:G13", blue,       true },
        { "H2:H13", green,      true },
        { "I2:I13", red,        true },
        { "J2:J13", blue,       false },
    };

    const auto& formats = get_cond_formats(*doc, "Single");
    assert(formats.size() == std::size(checks));

    for (std::size_t i = 0; i < std::size(checks); ++i)
    {
        const check& c = checks[i];
        const ss::cond_format_t& format = formats[i];

        assert(format.ranges.size() == 1);
        assert(format.ranges[0] == to_range(c.range));
        assert(format.rules.size() == 1);

        auto* rule = std::get_if<ss::cond_format_databar_rule_t>(&format.rules[0]);
        assert(rule);
        assert(rule->show_value == c.show_value);

        assert(rule->color_positive);
        assert(*rule->color_positive == c.color);

        // TODO: the file stores the following in the x14 extension block,
        // which we don't import yet.  Check them once we do.
        //
        // attributes of x14:dataBar
        //   minLength
        //   maxLength
        //   gradient
        //   border
        //   negativeBarBorderColorSameAsPositive
        //
        // child elements of x14:dataBar
        //   x14:cfvo
        //   x14:borderColor
        //   x14:negativeFillColor
        //   x14:negativeBorderColor
        //   x14:axisColor

        const std::vector<ss::cond_format_value_t> expected = {
            to_value(ss::cond_format_value_type_t::min, {}),
            to_value(ss::cond_format_value_type_t::max, {}),
        };

        assert(check_values(rule->values, expected));
    }
}

void test_xlsx_cond_format_color_scales()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/conditional-format/color-scales.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path, false);

    const auto& formats = get_cond_formats(*doc, "Single");
    assert(formats.size() == 2);

    {
        // 2-color scale
        const ss::cond_format_t& format = formats[0];
        assert(format.ranges.size() == 1);
        assert(format.ranges[0] == to_range("A2:A13"));
        assert(format.rules.size() == 1);

        auto* rule = std::get_if<ss::cond_format_colorscale_rule_t>(&format.rules[0]);
        assert(rule);

        const std::vector<ss::cond_format_value_t> expected = {
            to_value(ss::cond_format_value_type_t::min, {}),
            to_value(ss::cond_format_value_type_t::max, {}),
        };

        assert(check_values(rule->values, expected));

        const ss::color_t orange{0xFF, 0xFF, 0x71, 0x28};
        const ss::color_t light_yellow{0xFF, 0xFF, 0xEF, 0x9C};

        assert(rule->values[0].color);
        assert(*rule->values[0].color == orange);
        assert(rule->values[1].color);
        assert(*rule->values[1].color == light_yellow);
    }

    {
        // 3-color scale
        const ss::cond_format_t& format = formats[1];
        assert(format.ranges.size() == 1);
        assert(format.ranges[0] == to_range("B2:B13"));
        assert(format.rules.size() == 1);

        auto* rule = std::get_if<ss::cond_format_colorscale_rule_t>(&format.rules[0]);
        assert(rule);

        const std::vector<ss::cond_format_value_t> expected = {
            to_value(ss::cond_format_value_type_t::min, {}),
            to_value(ss::cond_format_value_type_t::percentile, "50"),
            to_value(ss::cond_format_value_type_t::max, {}),
        };

        assert(check_values(rule->values, expected));

        const ss::color_t red{0xFF, 0xF8, 0x69, 0x6B};
        const ss::color_t yellow{0xFF, 0xFF, 0xEB, 0x84};
        const ss::color_t green{0xFF, 0x63, 0xBE, 0x7B};

        assert(rule->values[0].color);
        assert(*rule->values[0].color == red);
        assert(rule->values[1].color);
        assert(*rule->values[1].color == yellow);
        assert(rule->values[2].color);
        assert(*rule->values[2].color == green);
    }
}

void test_xlsx_cond_format_icon_sets()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/conditional-format/icon-sets.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path, false);

    const auto& formats = get_cond_formats(*doc, "Single");
    assert(formats.size() == 3);

    {
        // 3 arrows with the default thresholds
        const ss::cond_format_t& format = formats[0];
        assert(format.ranges.size() == 1);
        assert(format.ranges[0] == to_range("A2:A13"));
        assert(format.rules.size() == 1);

        auto* rule = std::get_if<ss::cond_format_iconset_rule_t>(&format.rules[0]);
        assert(rule);
        assert(rule->show_value);
        assert(rule->icon_set == ss::icon_set_t::three_arrows);
        assert(!rule->reverse);

        const std::vector<ss::cond_format_value_t> expected = {
            to_value(ss::cond_format_value_type_t::percent, "0"),
            to_value(ss::cond_format_value_type_t::percent, "33"),
            to_value(ss::cond_format_value_type_t::percent, "67"),
        };

        assert(check_values(rule->values, expected));
    }

    {
        // 5 ratings in reverse order, showing the icons only
        const ss::cond_format_t& format = formats[1];
        assert(format.ranges.size() == 1);
        assert(format.ranges[0] == to_range("B2:B13"));
        assert(format.rules.size() == 1);

        auto* rule = std::get_if<ss::cond_format_iconset_rule_t>(&format.rules[0]);
        assert(rule);
        assert(!rule->show_value);
        assert(rule->icon_set == ss::icon_set_t::five_rating);
        assert(rule->reverse);

        const std::vector<ss::cond_format_value_t> expected = {
            to_value(ss::cond_format_value_type_t::percent, "0"),
            to_value(ss::cond_format_value_type_t::percent, "20"),
            to_value(ss::cond_format_value_type_t::percent, "40"),
            to_value(ss::cond_format_value_type_t::percent, "60"),
            to_value(ss::cond_format_value_type_t::percent, "80"),
        };

        assert(check_values(rule->values, expected));
    }

    {
        // 4 red to black with number thresholds
        const ss::cond_format_t& format = formats[2];
        assert(format.ranges.size() == 1);
        assert(format.ranges[0] == to_range("C2:C13"));
        assert(format.rules.size() == 1);

        auto* rule = std::get_if<ss::cond_format_iconset_rule_t>(&format.rules[0]);
        assert(rule);
        assert(rule->show_value);
        assert(rule->icon_set == ss::icon_set_t::four_red_to_black);
        assert(!rule->reverse);

        const std::vector<ss::cond_format_value_t> expected = {
            to_value(ss::cond_format_value_type_t::percent, "0"),
            to_value(ss::cond_format_value_type_t::value, "25"),
            to_value(ss::cond_format_value_type_t::value, "50"),
            to_value(ss::cond_format_value_type_t::value, "75"),
        };

        assert(check_values(rule->values, expected));
    }
}

void test_xlsx_cond_format_dates()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/conditional-format/dates.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path, false);

    struct check
    {
        std::string_view range;
        ss::cond_format_date_t period;
        std::size_t dxf_id;
    };

    const check checks[] = {
        { "A2:A13", ss::cond_format_date_t::yesterday, 2 },
        { "B2:B13", ss::cond_format_date_t::last_7_days, 1 },
        { "C2:C13", ss::cond_format_date_t::next_month, 0 },
    };

    const auto& formats = get_cond_formats(*doc, "Single");
    assert(formats.size() == std::size(checks));

    for (std::size_t i = 0; i < std::size(checks); ++i)
    {
        const check& c = checks[i];
        const ss::cond_format_t& format = formats[i];

        assert(format.ranges.size() == 1);
        assert(format.ranges[0] == to_range(c.range));
        assert(format.rules.size() == 1);

        auto* rule = std::get_if<ss::cond_format_date_rule_t>(&format.rules[0]);
        assert(rule);
        assert(rule->period == c.period);
        assert(rule->dxf_id);
        assert(*rule->dxf_id == c.dxf_id);
    }
}

void test_xlsx_cond_format_multi_range()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/conditional-format/multi-range.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path, false);

    const auto& formats = get_cond_formats(*doc, "Single");
    assert(formats.size() == 1);

    // one format applied to two ranges
    const ss::cond_format_t& format = formats[0];
    assert(format.ranges.size() == 2);
    assert(format.ranges[0] == to_range("A2:A13"));
    assert(format.ranges[1] == to_range("C2:C13"));
    assert(format.rules.size() == 1);

    auto* rule = std::get_if<ss::cond_format_condition_rule_t>(&format.rules[0]);
    assert(rule);
    assert(rule->op == ss::cond_format_operator_t::greater);
    assert(rule->dxf_id);
    assert(*rule->dxf_id == 1);
    assert(check_values(rule->values, {to_value(ss::cond_format_value_type_t::unknown, "5")}));
}

void test_xlsx_cond_format_formula()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/conditional-format/formula.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path, false);

    const auto& formats = get_cond_formats(*doc, "Single");
    assert(formats.size() == 1);

    const ss::cond_format_t& format = formats[0];
    assert(format.ranges.size() == 1);
    assert(format.ranges[0] == to_range("A2:A13"));
    assert(format.rules.size() == 1);

    auto* rule = std::get_if<ss::cond_format_condition_rule_t>(&format.rules[0]);
    assert(rule);
    assert(rule->op == ss::cond_format_operator_t::expression);
    assert(rule->dxf_id);
    assert(*rule->dxf_id == 0);
    assert(check_values(rule->values, {to_value(ss::cond_format_value_type_t::unknown, "LEN(A2)>3")}));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
