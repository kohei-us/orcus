/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_gnumeric_test.hpp"

namespace ss = orcus::spreadsheet;
namespace test = orcus::test;

namespace {

test::rc_range_resolver to_range(ixion::formula_name_resolver_t::excel_a1);

} // anonymous namespace

void test_gnumeric_auto_filter_multi_rules()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/table/autofilter-multi-rules.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    {
        const ss::sheet* sh = doc->get_sheet("Single");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 0, v == 'A'}
        //  |
        //  +- item {field: 1, v == 1}
        //  |
        //  +- item {field: 2, v == 1}  boolean 'true' imported as numeric 1

        assert(af->range == to_range("B4:D10"));
        assert(af->root.size() == 3);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        // same order as they appear in the source file
        const ss::filter_item_t expected[3] = {
            {2, ss::auto_filter_op_t::equal, 1},
            {1, ss::auto_filter_op_t::equal, 1},
            {0, ss::auto_filter_op_t::equal, "A"},
        };

        for (std::size_t i = 0; i < 3; ++i)
        {
            auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(i));
            assert(item);
            assert(*item == expected[i]);
        }
    }

    {
        const ss::sheet* sh = doc->get_sheet("Multi");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // same order as they appear in the source file
        //
        // root {and}
        //  |
        //  +- node {and}
        //  |   |
        //  |   +- item{field: 1, v >= 3}
        //  |   |
        //  |   +- item{field: 1, v <= 22}
        //  |
        //  +- node {or}
        //      |
        //      +- item{field: 0, v == 'E'}
        //      |
        //      +- item{field: 0, v == 'P'}

        assert(af->range == to_range("B6:C32"));
        assert(af->root.size() == 2);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        {
            auto* node = dynamic_cast<const ss::filter_node_t*>(af->root.at(0));
            assert(node);
            assert(node->op() == ss::auto_filter_node_op_t::op_and);
            assert(node->size() == 2);

            const ss::filter_item_t expected[2] = {
                {1, ss::auto_filter_op_t::greater_equal, 3},
                {1, ss::auto_filter_op_t::less_equal, 22},
            };

            for (std::size_t i = 0; i < 2; ++i)
            {
                auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(i));
                assert(item);
                assert(*item == expected[i]);
            }
        }

        {
            auto* node = dynamic_cast<const ss::filter_node_t*>(af->root.at(1));
            assert(node);
            assert(node->op() == ss::auto_filter_node_op_t::op_or);
            assert(node->size() == 2);

            const ss::filter_item_t expected[2] = {
                {0, ss::auto_filter_op_t::equal, "E"},
                {0, ss::auto_filter_op_t::equal, "P"},
            };

            for (std::size_t i = 0; i < 2; ++i)
            {
                auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(i));
                assert(item);
                assert(*item == expected[i]);
            }
        }
    }
}

void test_gnumeric_auto_filter_number()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/table/autofilter-number.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    {
        const ss::sheet* sh = doc->get_sheet("Greater Than");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 2, v > 20}

        assert(af->range == to_range("B3:G96"));
        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{2, ss::auto_filter_op_t::greater, 20};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Greater Than Equal");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 2, v >= 20}

        assert(af->range == to_range("B3:G96"));
        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{2, ss::auto_filter_op_t::greater_equal, 20};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Less Than");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 0, v < 5}

        assert(af->range == to_range("B3:G96"));
        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{0, ss::auto_filter_op_t::less, 5};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Less Than Equal");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 0, v <= 10}

        assert(af->range == to_range("B3:G96"));
        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{0, ss::auto_filter_op_t::less_equal, 10};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Between");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- node {and}
        //  |   |
        //  |   +- item{field: 0, v >= 10}
        //  |   |
        //  |   +- item{field: 0, v <= 20}

        assert(af->range == to_range("B3:G96"));
        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* node = dynamic_cast<const ss::filter_node_t*>(af->root.at(0));
        assert(node);
        assert(node->op() == ss::auto_filter_node_op_t::op_and);
        assert(node->size() == 2);

        const ss::filter_item_t expected[2] = {
            {0, ss::auto_filter_op_t::greater_equal, 10},
            {0, ss::auto_filter_op_t::less_equal, 20},
        };

        for (std::size_t i = 0; i < std::size(expected); ++i)
        {
            auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(i));
            assert(item);
            assert(*item == expected[i]);
        }
    }

    {
        const ss::sheet* sh = doc->get_sheet("Top 10");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 2, top 5}

        assert(af->range == to_range("B3:E17"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{2, ss::auto_filter_op_t::top, 5};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Bottom 10");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 2, bottom 3}

        assert(af->range == to_range("B3:E17"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{2, ss::auto_filter_op_t::bottom, 3};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Top %");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 4, top 20% of items}

        assert(af->range == to_range("B3:H50"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{4, ss::auto_filter_op_t::top_percent, 20};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Top %Range");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 4, top 20% of data range}

        assert(af->range == to_range("B3:H50"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{4, ss::auto_filter_op_t::top_percent_range, 20};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Bottom %");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 4, bottom 5% of items}

        assert(af->range == to_range("B3:H50"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{4, ss::auto_filter_op_t::bottom_percent, 5};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Bottom %Range");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 4, bottom 5% of data range}

        assert(af->range == to_range("B3:H50"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{4, ss::auto_filter_op_t::bottom_percent_range, 5};
        assert(*item == expected);
    }
}

void test_gnumeric_auto_filter_text()
{
    ORCUS_TEST_FUNC_SCOPE;

    fs::path filepath = SRCDIR"/test/gnumeric/table/autofilter-text.gnumeric";
    auto doc = load_doc(filepath);
    assert(doc);

    {
        const ss::sheet* sh = doc->get_sheet("Select One");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 1, v == 'FL'}

        assert(af->range == to_range("B3:E17"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{1, ss::auto_filter_op_t::equal, "FL"};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Equals");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- node {or}
        //  |   |
        //  |   +- item{field: 1, v == "Japan"}
        //  |   |
        //  |   +- item{field: 1, v == "China"}

        assert(af->range == to_range("B3:G96"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* node = dynamic_cast<const ss::filter_node_t*>(af->root.at(0));
        assert(node);
        assert(node->op() == ss::auto_filter_node_op_t::op_or);
        assert(node->size() == 2);

        const ss::filter_item_t expected[2] = {
            {1, ss::auto_filter_op_t::equal, "Japan"},
            {1, ss::auto_filter_op_t::equal, "China"},
        };

        for (std::size_t i = 0; i < std::size(expected); ++i)
        {
            auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(i));
            assert(item);
            assert(*item == expected[i]);
        }
    }

    {
        const ss::sheet* sh = doc->get_sheet("Does Not Equal");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- node {and}
        //  |   |
        //  |   +- item{field: 1, v != "NV"}
        //  |   |
        //  |   +- item{field: 1, v != "FL"}

        assert(af->range == to_range("B3:E17"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* node = dynamic_cast<const ss::filter_node_t*>(af->root.at(0));
        assert(node);
        assert(node->op() == ss::auto_filter_node_op_t::op_and);
        assert(node->size() == 2);

        const ss::filter_item_t expected[2] = {
            {1, ss::auto_filter_op_t::not_equal, "NV"},
            {1, ss::auto_filter_op_t::not_equal, "FL"},
        };

        for (std::size_t i = 0; i < std::size(expected); ++i)
        {
            auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(i));
            assert(item);
            assert(*item == expected[i]);
        }
    }

    {
        const ss::sheet* sh = doc->get_sheet("Begins With");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 1, begins with 'Be'}

        assert(af->range == to_range("B3:G96"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{1, ss::auto_filter_op_t::begin_with, "Be"};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Ends With");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 1, ends with 'lic'}

        assert(af->range == to_range("B3:G96"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{1, ss::auto_filter_op_t::end_with, "lic"};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Contains");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 0, contains 'ing'}

        assert(af->range == to_range("B3:E17"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{0, ss::auto_filter_op_t::contain, "ing"};
        assert(*item == expected);
    }

    {
        const ss::sheet* sh = doc->get_sheet("Does Not Contain");
        assert(sh);

        const ss::auto_filter_t* af = sh->get_auto_filter();
        assert(af);

        // root {and}
        //  |
        //  +- item {field: 0, not-contain 'an'}

        assert(af->range == to_range("B3:E17"));

        assert(af->root.size() == 1);
        assert(af->root.op() == ss::auto_filter_node_op_t::op_and);

        auto* item = dynamic_cast<const ss::filter_item_t*>(af->root.at(0));
        assert(item);

        const ss::filter_item_t expected{0, ss::auto_filter_op_t::not_contain, "an"};
        assert(*item == expected);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
