/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_xlsx_test.hpp"

namespace ss = orcus::spreadsheet;
namespace test = orcus::test;

namespace {

test::rc_range_resolver to_range(ixion::formula_name_resolver_t::excel_a1);

} // anonymous namespace

void test_xlsx_table_autofilter()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/table/autofilter.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path);

    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

#if 0 // FIXME
    const ss::old::auto_filter_t* af = sh->get_auto_filter_data();
    assert(af);

    // Autofilter is over B2:C11.
    assert(af->range.first.column == 1);
    assert(af->range.first.row == 1);
    assert(af->range.last.column == 2);
    assert(af->range.last.row == 10);

    // Check the match values of the 1st column filter criterion.
    auto it = af->columns.find(0);
    assert(it != af->columns.end());

    const ss::old::auto_filter_column_t* afc = &it->second;
    assert(afc->match_values.count("A") > 0);
    assert(afc->match_values.count("C") > 0);

    // And the 2nd column.
    it = af->columns.find(1);
    assert(it != af->columns.end());
    afc = &it->second;
    assert(afc->match_values.count("1") > 0);
#endif
}

void test_xlsx_table_autofilter_basic_number()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/table/autofilter-basic-number.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path);

    {
        const ss::sheet* sh = doc->get_sheet("Greater Than");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);
        assert(af->range == to_range("B3:G96"));

        // 1: filter-rule: v > 20; field: 2

        auto items = test::excel_field_filter_items::get(*af, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater, 20};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Greater Than Equal");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B3:G96"));

        // 1: filter-rule: v >= 20; field: 2

        auto items = test::excel_field_filter_items::get(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater_equal, 20};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Less Than");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B3:G96"));

        // 1: filter-rule: v < 5; field: 0

        auto items = test::excel_field_filter_items::get(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::less, 5};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Less Than Equal");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B3:G96"));

        // 1: filter-rule: v <= 10; field: 0

        auto items = test::excel_field_filter_items::get(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::less_equal, 10};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Between");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B3:G96"));

        // 1: filter-rule: v >= 10; field: 0
        // 2: filter-rule: v <= 20; field: 0
        // connector: AND

        auto items = test::excel_field_filter_items::get(*filter, 0);
        assert(items.size() == 2u);
        assert(items.connector == ss::auto_filter_node_op_t::op_and);

        ss::filter_item_t expected1{0, ss::auto_filter_op_t::greater_equal, 10};
        ss::filter_item_t expected2{0, ss::auto_filter_op_t::less_equal, 20};
        assert(items.contains(expected1));
        assert(items.contains(expected2));
    }

    {
        auto* sh = doc->get_sheet("Top 10");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // 1: filter-rule: top 5; field: 2

        auto items = test::excel_field_filter_items::get(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::top, 5};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Bottom 10");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // 1: filter-rule: bottom 3; field: 2

        auto items = test::excel_field_filter_items::get(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::bottom, 3};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Above Average");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // 1: filter-rule: v > 150547; field: 2

        auto items = test::excel_field_filter_items::get(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater, 150547};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Below Average");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // 1: filter-rule: v < 150547; field: 2

        auto items = test::excel_field_filter_items::get(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::less, 150547};
        assert(items.contains(expected));
    }
}

void test_xlsx_table_autofilter_basic_text()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/table/autofilter-basic-text.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path);

    {
        auto* sh = doc->get_sheet("Equals");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B3:G96"));

        // root
        //  |
        //  +- item-set {field: 1; values: Japan or China}

        assert(filter->root.op() == ss::auto_filter_node_op_t::op_and);
        assert(filter->root.children().size() == 1u);
        auto* p = dynamic_cast<const ss::filter_item_set_t*>(filter->root.children()[0]);
        assert(p);

        ss::filter_item_set_t expected{1, {"Japan", "China"}};
        assert(*p == expected);
    }
}

void test_xlsx_table()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/table/table-1.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path);

    std::string_view name("Table1");
    const ss::table_t* p = doc->get_table(name);
    assert(p);
    assert(p->identifier == 1);
    assert(p->name == name);
    assert(p->display_name == name);
    assert(p->totals_row_count == 1);

    // Table range is C3:D9.
    ixion::abs_range_t range;
    range.first.column = 2;
    range.first.row = 2;
    range.first.sheet = 0;
    range.last.column = 3;
    range.last.row = 8;
    range.last.sheet = 0;
    assert(p->range == range);

    // Table1 has 2 table columns.
    assert(p->columns.size() == 2);

    const ss::table_column_t* tcol = &p->columns[0];
    assert(tcol);
    assert(tcol->identifier == 1);
    assert(tcol->name == "Category");
    assert(tcol->totals_row_label == "Total");
    assert(tcol->totals_row_function == ss::totals_row_function_t::none);

    tcol = &p->columns[1];
    assert(tcol);
    assert(tcol->identifier == 2);
    assert(tcol->name == "Value");
    assert(tcol->totals_row_label.empty());
    assert(tcol->totals_row_function == ss::totals_row_function_t::sum);

#if 0 // FIXME
    const auto& filter = p->filter_old;

    // Auto filter range is C3:D8.
    range.last.row = 7;
    assert(filter.range == range);

    assert(filter.columns.size() == 1);
    const ss::old::auto_filter_column_t& afc = filter.columns.begin()->second;
    assert(afc.match_values.size() == 4);
    assert(afc.match_values.count("A") > 0);
    assert(afc.match_values.count("C") > 0);
    assert(afc.match_values.count("D") > 0);
    assert(afc.match_values.count("E") > 0);
#endif

    // Check table style.
    const ss::table_style_t& style = p->style;
    assert(style.name == "TableStyleLight9");
    assert(style.show_first_column == false);
    assert(style.show_last_column == false);
    assert(style.show_row_stripes == true);
    assert(style.show_column_stripes == false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
