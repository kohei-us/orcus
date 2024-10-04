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

    {
        auto tab = test::get_table_from_sheet(*doc, "Multi-Select", "Table4");
        assert(tab);
        assert(tab->filter.range == to_range("B5:C14"));
        assert(tab->filter.root.size() == 2);
        assert(tab->filter.root.op() == ss::auto_filter_node_op_t::op_and);

        // field1 equals either 'A' or 'C'
        auto* f1 = dynamic_cast<const ss::filter_item_set_t*>(tab->filter.root.at(0));
        assert(f1);
        ss::filter_item_set_t expected1{0, {"A", "C"}};
        assert(*f1 == expected1);

        // field2 equals '1'
        auto* f2 = dynamic_cast<const ss::filter_item_set_t*>(tab->filter.root.at(1));
        assert(f2);
        ss::filter_item_set_t expected2{1, {"1",}};
        assert(*f2 == expected2);
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Multi-Select", "Table1");
        assert(tab);
        assert(tab->filter.range == to_range("B17:B37"));
        assert(tab->filter.root.size() == 1);

        // field1 equals either 'Tokyo', 'Paris' or 'New York'
        auto* f1 = dynamic_cast<const ss::filter_item_set_t*>(tab->filter.root.at(0));
        assert(f1);
        ss::filter_item_set_t expected{0, {"Tokyo", "Paris", "New York"}};
        assert(*f1 == expected);
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Does Not Equal", "Table2");
        assert(tab);
        assert(tab->filter.range == to_range("B4:C24"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 0; v != Houston}
        //       |
        //       +- item {field: 0; v != Chicago}

        auto items = test::excel_field_filter_items::get(tab->filter, 0);
        assert(items.size() == 2);
        assert(items.connector == ss::auto_filter_node_op_t::op_and);

        ss::filter_item_t expected1{0, ss::auto_filter_op_t::not_equal, "Houston"};
        ss::filter_item_t expected2{0, ss::auto_filter_op_t::not_equal, "Chicago"};
        assert(items.contains(expected1));
        assert(items.contains(expected2));
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Does Not Equal", "Table3");
        assert(tab);
        assert(tab->filter.range == to_range("B27:G120"));

        // root {and}
        //  |
        //  +- field {and}
        //  |    |
        //  |    +- item {field: 2; v != 1}
        //  |    |
        //  |    +- item {field: 2; v != 0}
        //  |
        //  +- field {and}
        //  |    |
        //  |    +- item {field: 3; v != 1}
        //  |    |
        //  |    +- item {field: 3; v != 0}
        //  |
        //  +- field {and}
        //  |    |
        //  |    +- item {field: 4; v != 1}
        //  |    |
        //  |    +- item {field: 4; v != 0}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 5; v != 1}
        //       |
        //       +- item {field: 5; v != 0}

        for (ss::col_t field = 2; field <= 4; ++field)
        {
            auto items = test::excel_field_filter_items::get(tab->filter, field);
            assert(items.size() == 2);
            assert(items.connector == ss::auto_filter_node_op_t::op_and);

            ss::filter_item_t expected1{field, ss::auto_filter_op_t::not_equal, "1"};
            ss::filter_item_t expected2{field, ss::auto_filter_op_t::not_equal, "0"};
            assert(items.contains(expected1));
            assert(items.contains(expected2));
        }
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Begins With | Ends With", "Table5");
        assert(tab);
        assert(tab->filter.range == to_range("C4:H97"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 1; begins with 'Ja'}

        auto items = test::excel_field_filter_items::get(tab->filter, 1);
        assert(items.size() == 1);

        ss::filter_item_t expected{1, ss::auto_filter_op_t::begin_with, "Ja"};
        assert(items.contains(expected));
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Begins With | Ends With", "Table6");
        assert(tab);
        assert(tab->filter.range == to_range("C102:H195"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 1; ends with 'land'}

        auto items = test::excel_field_filter_items::get(tab->filter, 1);
        assert(items.size() == 1);

        ss::filter_item_t expected{1, ss::auto_filter_op_t::end_with, "land"};
        assert(items.contains(expected));
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Greater Than", "Table9");
        assert(tab);
        assert(tab->filter.range == to_range("B6:G99"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 2; v > 20}

        auto items = test::excel_field_filter_items::get(tab->filter, 2);
        assert(items.size() == 1);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater, 20};
        assert(items.contains(expected));
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Greater Than", "Table911");
        assert(tab);
        assert(tab->filter.range == to_range("B104:G197"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 2; v >= 20}

        auto items = test::excel_field_filter_items::get(tab->filter, 2);
        assert(items.size() == 1);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater_equal, 20};
        assert(items.contains(expected));
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Less Than | Between", "Table912");
        assert(tab);
        assert(tab->filter.range == to_range("B4:G97"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 0; v < 5}

        auto items = test::excel_field_filter_items::get(tab->filter, 0);
        assert(items.size() == 1);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::less, 5};
        assert(items.contains(expected));
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Less Than | Between", "Table91113");
        assert(tab);
        assert(tab->filter.range == to_range("B100:G193"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 0; v <= 10}

        auto items = test::excel_field_filter_items::get(tab->filter, 0);
        assert(items.size() == 1);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::less_equal, 10};
        assert(items.contains(expected));
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Less Than | Between", "Table13");
        assert(tab);
        assert(tab->filter.range == to_range("B196:G289"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 0; v >= 10}
        //       |
        //       +- item {field: 0; v <= 20}

        auto items = test::excel_field_filter_items::get(tab->filter, 0);
        assert(items.size() == 2);
        assert(items.connector == ss::auto_filter_node_op_t::op_and);

        ss::filter_item_t expected1{0, ss::auto_filter_op_t::greater_equal, 10};
        ss::filter_item_t expected2{0, ss::auto_filter_op_t::less_equal, 20};
        assert(items.contains(expected1));
        assert(items.contains(expected2));
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Top 10 | Above Average", "Table16");
        assert(tab);
        assert(tab->filter.range == to_range("C3:F17"));

        // root {and}
        //  |
        //  +- item {field: 2; top 5}

        assert(tab->filter.root.size() == 1);
        auto* f = dynamic_cast<const ss::filter_item_t*>(tab->filter.root.at(0));
        assert(f);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::top, 5};
        assert(*f == expected);
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Top 10 | Above Average", "Table1616");
        assert(tab);
        assert(tab->filter.range == to_range("C21:F35"));

        // root {and}
        //  |
        //  +- item {field: 2; bottom 3}

        assert(tab->filter.root.size() == 1);
        auto* f = dynamic_cast<const ss::filter_item_t*>(tab->filter.root.at(0));
        assert(f);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::bottom, 3};
        assert(*f == expected);
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Top 10 | Above Average", "Table17");
        assert(tab);
        assert(tab->filter.range == to_range("C39:F53"));

        // root {and}
        //  |
        //  +- item {field: 2; above average; v > 150547}

        assert(tab->filter.root.size() == 1);
        auto* f = dynamic_cast<const ss::filter_item_t*>(tab->filter.root.at(0));
        assert(f);

        // We don't support dynamic filter yet; so treat it as a static one for now
        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater, 150547};
        assert(*f == expected);
    }

    {
        auto tab = test::get_table_from_sheet(*doc, "Top 10 | Above Average", "Table18");
        assert(tab);
        assert(tab->filter.range == to_range("C57:F71"));

        // root {and}
        //  |
        //  +- item {field: 2; below average; v < 150547}

        assert(tab->filter.root.size() == 1);
        auto* f = dynamic_cast<const ss::filter_item_t*>(tab->filter.root.at(0));
        assert(f);

        // We don't support dynamic filter yet; so treat it as a static one for now
        ss::filter_item_t expected{2, ss::auto_filter_op_t::less, 150547};
        assert(*f == expected);
    }
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

        // root
        //  |
        //  +- item {field 2; top 5}

        assert(filter->root.op() == ss::auto_filter_node_op_t::op_and);
        assert(filter->root.size() == 1u);
        auto* p = dynamic_cast<const ss::filter_item_t*>(filter->root.at(0));
        assert(p);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::top, 5};
        assert(*p == expected);
    }

    {
        auto* sh = doc->get_sheet("Bottom 10");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // root
        //  |
        //  +- item {field 2; bottom 3}

        assert(filter->root.op() == ss::auto_filter_node_op_t::op_and);
        assert(filter->root.size() == 1u);
        auto* p = dynamic_cast<const ss::filter_item_t*>(filter->root.at(0));
        assert(p);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::bottom, 3};
        assert(*p == expected);
    }

    {
        auto* sh = doc->get_sheet("Above Average");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // root
        //  |
        //  +- item {field 2; v > 150547}

        assert(filter->root.op() == ss::auto_filter_node_op_t::op_and);
        assert(filter->root.size() == 1u);
        auto* p = dynamic_cast<const ss::filter_item_t*>(filter->root.at(0));
        assert(p);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater, 150547};
        assert(*p == expected);
    }

    {
        auto* sh = doc->get_sheet("Below Average");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // root
        //  |
        //  +- item {field 2; v < 150547}

        assert(filter->root.op() == ss::auto_filter_node_op_t::op_and);
        assert(filter->root.size() == 1u);
        auto* p = dynamic_cast<const ss::filter_item_t*>(filter->root.at(0));
        assert(p);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::less, 150547};
        assert(*p == expected);
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
        assert(filter->root.size() == 1u);
        auto* p = dynamic_cast<const ss::filter_item_set_t*>(filter->root.at(0));
        assert(p);

        ss::filter_item_set_t expected{1, {"Japan", "China"}};
        assert(*p == expected);
    }

    {
        auto* sh = doc->get_sheet("Does Not Equal");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 1; v != NV}
        //       |
        //       +- item {field: 1; v != FL}

        auto items = test::excel_field_filter_items::get(*filter, 1);
        assert(items.size() == 2u);
        assert(items.connector == ss::auto_filter_node_op_t::op_and);

        ss::filter_item_t expected1{1, ss::auto_filter_op_t::not_equal, "NV"};
        ss::filter_item_t expected2{1, ss::auto_filter_op_t::not_equal, "FL"};
        assert(items.contains(expected1));
        assert(items.contains(expected2));
    }

    {
        auto* sh = doc->get_sheet("Begins With");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B3:G96"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 1; begins with 'Be'}

        auto items = test::excel_field_filter_items::get(*filter, 1);
        assert(items.size() == 1u);

        ss::filter_item_t expected{1, ss::auto_filter_op_t::begin_with, "Be"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Ends With");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B3:G96"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 1; ends with 'lic'}

        auto items = test::excel_field_filter_items::get(*filter, 1);
        assert(items.size() == 1u);

        ss::filter_item_t expected{1, ss::auto_filter_op_t::end_with, "lic"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Contains");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 0; contains 'ing'}

        auto items = test::excel_field_filter_items::get(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::contain, "ing"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Does Not Contain");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("B4:E18"));

        // root {and}
        //  |
        //  +- field {and}
        //       |
        //       +- item {field: 0; not contain 'an'}

        auto items = test::excel_field_filter_items::get(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::not_contain, "an"};
        assert(items.contains(expected));
    }
}

void test_xlsx_table()
{
    ORCUS_TEST_FUNC_SCOPE;

    std::string_view path(SRCDIR"/test/xlsx/table/table-1.xlsx");
    std::unique_ptr<ss::document> doc = load_doc(path);

    std::string_view name("Table1");
    const ss::table_t* p = doc->get_tables().get(name);
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

    // Check table style.
    const ss::table_style_t& style = p->style;
    assert(style.name == "TableStyleLight9");
    assert(style.show_first_column == false);
    assert(style.show_last_column == false);
    assert(style.show_row_stripes == true);
    assert(style.show_column_stripes == false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
