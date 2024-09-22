/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_xls_xml_test.hpp"

#include <orcus/spreadsheet/auto_filter.hpp>
#include <ixion/formula_name_resolver.hpp>

#include <set>

namespace ss = orcus::spreadsheet;
namespace test = orcus::test;

namespace {

test::rc_range_resolver to_range(ixion::formula_name_resolver_t::excel_r1c1);

struct filter_items
{
    std::set<ss::filter_item_t> items;
    ss::auto_filter_node_op_t connector;

    bool contains(const ss::filter_item_t& expected) const
    {
        return items.count(expected) > 0;
    }

    std::size_t size() const
    {
        return items.size();
    }
};

filter_items get_filter_items_for_field(
    const ss::auto_filter_t& filter, ss::col_t field_index)
{
    // The root node should have one child node per filtered field,
    // connected by the 'and' operator.
    if (filter.root.op != ss::auto_filter_node_op_t::op_and)
        assert(!"the node operator in the root node should be AND");

    filter_items items;

    for (const auto* field_node : filter.root.children)
    {
        const auto* field = dynamic_cast<const ss::filter_node_t*>(field_node);
        if (!field)
            assert(!"child of the root node should be a field");

        items.connector = field->op;

        for (const auto* item_node : field->children)
        {
            const auto* item = dynamic_cast<const ss::filter_item_t*>(item_node);
            if (!item)
                assert(!"child of a field node should be a filter item");

            if (item->field == field_index)
                items.items.insert(*item); // copy
        }
    }

    return items;
}

}

void test_xls_xml_auto_filter_number()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/table/autofilter-number.xml");
    assert(doc);

    {
        auto* sh = doc->get_sheet("Greater Than");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R96C7"));

        // 1: filter-rule: v > 20; field: 2

        auto items = get_filter_items_for_field(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater, 20};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Greater Than Equal");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R96C7"));

        // 1: filter-rule: v >= 20; field: 2

        auto items = get_filter_items_for_field(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater_equal, 20};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Less Than");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R96C7"));

        // 1: filter-rule: v < 5; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::less, 5};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Less Than Equal");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R96C7"));

        // 1: filter-rule: v <= 10; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::less_equal, 10};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Between");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R96C7"));

        // 1: filter-rule: v >= 10; field: 0
        // 2: filter-rule: v <= 20; field: 0
        // connector: AND

        auto items = get_filter_items_for_field(*filter, 0);
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
        assert(filter->range == to_range("R4C2:R18C5"));

        // 1: filter-rule: top 5; field: 2

        auto items = get_filter_items_for_field(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::top, 5};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Bottom 10");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R4C2:R18C5"));

        // 1: filter-rule: bottom 3; field: 2

        auto items = get_filter_items_for_field(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::bottom, 3};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Above Average");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R4C2:R18C5"));

        // 1: filter-rule: v > 150547; field: 2

        auto items = get_filter_items_for_field(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::greater, 150547};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Below Average");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R4C2:R18C5"));

        // 1: filter-rule: v < 150547; field: 2

        auto items = get_filter_items_for_field(*filter, 2);
        assert(items.size() == 1u);

        ss::filter_item_t expected{2, ss::auto_filter_op_t::less, 150547};
        assert(items.contains(expected));
    }
}

void test_xls_xml_auto_filter_text()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/table/autofilter-text.xml");
    assert(doc);

    {
        auto* sh = doc->get_sheet("Begins With");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R96C7"));

        // 1: filter-rule: begin-with 'Be'; field: 1

        auto items = get_filter_items_for_field(*filter, 1);
        assert(items.size() == 1u);

        ss::filter_item_t expected{1, ss::auto_filter_op_t::begin_with, "Be"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Ends With");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R96C7"));

        // 1: filter-rule: end-with 'lic'; field: 1

        auto items = get_filter_items_for_field(*filter, 1);
        assert(items.size() == 1u);

        ss::filter_item_t expected{1, ss::auto_filter_op_t::end_with, "lic"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Contains");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R4C2:R18C5"));

        // 1: filter-rule: contain 'ing'; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::contain, "ing"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Does Not Contain");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R4C2:R18C5"));

        // 1: filter-rule: not-contain 'an'; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::not_contain, "an"};
        assert(items.contains(expected));
    }
}

void test_xls_xml_auto_filter_wildcard()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/table/autofilter-wildcard.xml");
    assert(doc);

    {
        auto* sh = doc->get_sheet("Wildcard-1");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R23C2"));

        // 1: filter-rule: equal 'W*nd'; field: 0
        // 2: filter-rule: equal 'Q*r'; field: 0
        // connector: OR

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 2u);
        assert(items.connector == ss::auto_filter_node_op_t::op_or);

        ss::filter_item_t expected1{0, ss::auto_filter_op_t::equal, "W.*nd", true};
        ss::filter_item_t expected2{0, ss::auto_filter_op_t::equal, "Q.*r", true};
        assert(items.contains(expected1));
        assert(items.contains(expected2));
    }

    {
        auto* sh = doc->get_sheet("Wildcard-2");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R4C2:R24C2"));

        // 1: filter-rule: equal 'Ca????d'; field: 0
        // 2: filter-rule: equal 'A????'; field: 0
        // connector: OR

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 2u);
        assert(items.connector == ss::auto_filter_node_op_t::op_or);

        ss::filter_item_t expected1{0, ss::auto_filter_op_t::equal, "Ca....d", true};
        ss::filter_item_t expected2{0, ss::auto_filter_op_t::equal, "A....", true};
        assert(items.contains(expected1));
        assert(items.contains(expected2));
    }
}

void test_xls_xml_auto_filter_asterisk()
{
    // NB: Excel escapes a literal '*' with '~', so '~*' should be interpreted
    // as a literal '*'.
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/table/autofilter-asterisk.xml");
    assert(doc);

    {
        auto* sh = doc->get_sheet("Ends With");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C3:R23C3"));

        // 1: filter-rule: equal '*~*'; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::end_with, "*"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Begins With");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R2C2:R22C2"));

        // 1: filter-rule: equal '~**'; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::begin_with, "*"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Contains");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R5C2:R25C2"));

        // 1: filter-rule: equal '*~**'; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::contain, "*"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Does Not Contain");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R5C2:R25C2"));

        // 1: filter-rule: not-equal '*~**'; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::not_contain, "*"};
        assert(items.contains(expected));
    }
}

void test_xls_xml_auto_filter_question()
{
    // NB: Similar to the asterisk test case, Excel escapes a literal '?' with '~'.
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/table/autofilter-question.xml");
    assert(doc);

    {
        auto* sh = doc->get_sheet("Ends With");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R23C2"));

        // 1: filter-rule: equal '*~?'; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::end_with, "?"};
        assert(items.contains(expected));
    }

    {
        auto* sh = doc->get_sheet("Does Not Contain");
        assert(sh);

        auto* filter = sh->get_auto_filter();
        assert(filter);
        assert(filter->range == to_range("R3C2:R23C2"));

        // 1: filter-rule: not-equal '*~?*'; field: 0

        auto items = get_filter_items_for_field(*filter, 0);
        assert(items.size() == 1u);

        ss::filter_item_t expected{0, ss::auto_filter_op_t::not_contain, "?"};
        assert(items.contains(expected));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
