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

    // TODO: continue on
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
