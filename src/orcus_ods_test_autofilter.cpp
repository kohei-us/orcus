/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_ods_test.hpp"

#include <orcus/spreadsheet/tables.hpp>
#include <orcus/spreadsheet/table.hpp>

namespace ss = orcus::spreadsheet;
namespace test = orcus::test;

void test_ods_autofilter_multi_conditions()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc(SRCDIR"/test/ods/autofilter/multi-conditions.ods");
    assert(doc);

    test::range_resolver to_range(ixion::formula_name_resolver_t::odf_cra, doc->get_model_context());

    const ss::tables& t = doc->get_tables();

    {
        auto table = t.get("__Anonymous_Sheet_DB__0").lock();
        assert(table);
        assert(table->range == to_range("'OR-AND'.B3:'OR-AND'.G96"));

        const ss::auto_filter_t& filter = table->filter;
        assert(filter.range == ixion::abs_rc_range_t(table->range));

        // root {or}
        //  |
        //  +- node {and}
        //  |    |
        //  |    +- item {field: 0; v < 20}
        //  |    |
        //  |    +- item {field: 0; v > 10}
        //  |
        //  +- node {and}
        //       |
        //       +- item {field: 0; v < 40}
        //       |
        //       +- item {field: 0; v > 30}

        assert(filter.root.op() == ss::auto_filter_node_op_t::op_or);
        assert(filter.root.size() == 2);

        {
            const auto* node = dynamic_cast<const ss::filter_node_t*>(filter.root.at(0));
            assert(node);
            assert(node->op() == ss::auto_filter_node_op_t::op_and);
            assert(node->size() == 2);

            {
                const auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(0));
                assert(item);
                assert(item->field() == 0);
                assert(item->op() == ss::auto_filter_op_t::less);
                assert(item->value() == ss::filter_value_t(20.0));
            }

            {
                const auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(1));
                assert(item);
                assert(item->field() == 0);
                assert(item->op() == ss::auto_filter_op_t::greater);
                assert(item->value() == ss::filter_value_t(10.0));
            }
        }

        {
            const auto* node = dynamic_cast<const ss::filter_node_t*>(filter.root.at(1));
            assert(node);
            assert(node->op() == ss::auto_filter_node_op_t::op_and);
            assert(node->size() == 2);

            {
                const auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(0));
                assert(item);
                assert(item->field() == 0);
                assert(item->op() == ss::auto_filter_op_t::less);
                assert(item->value() == ss::filter_value_t(40.0));
            }

            {
                const auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(1));
                assert(item);
                assert(item->field() == 0);
                assert(item->op() == ss::auto_filter_op_t::greater);
                assert(item->value() == ss::filter_value_t(30.0));
            }
        }
    }

    {
        auto table = t.get("__Anonymous_Sheet_DB__1").lock();
        assert(table);
        assert(table->range == to_range("'AND x 3'.B3:'AND x 3'.G96"));

        const ss::auto_filter_t& filter = table->filter;
        assert(filter.range == ixion::abs_rc_range_t(table->range));

        // root {or}
        //  |
        //  +- node {and}
        //       |
        //       +- item {field: 0; v < 20}
        //       |
        //       +- item {field: 0; v > 10}
        //       |
        //       +- item {field: 5; v >= 5}

        assert(filter.root.op() == ss::auto_filter_node_op_t::op_or);
        assert(filter.root.size() == 1);

        {
            const auto* node = dynamic_cast<const ss::filter_node_t*>(filter.root.at(0));
            assert(node);
            assert(node->op() == ss::auto_filter_node_op_t::op_and);
            assert(node->size() == 3);

            {
                const auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(0));
                assert(item);
                assert(item->field() == 0);
                assert(item->op() == ss::auto_filter_op_t::less);
                assert(item->value() == ss::filter_value_t(20.0));
            }

            {
                const auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(1));
                assert(item);
                assert(item->field() == 0);
                assert(item->op() == ss::auto_filter_op_t::greater);
                assert(item->value() == ss::filter_value_t(10.0));
            }

            {
                const auto* item = dynamic_cast<const ss::filter_item_t*>(node->at(2));
                assert(item);
                assert(item->field() == 5);
                assert(item->op() == ss::auto_filter_op_t::greater_equal);
                assert(item->value() == ss::filter_value_t(20.0));
            }
        }
    }
}

void test_ods_autofilter_text_comparisons()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc(SRCDIR"/test/ods/autofilter/text-comparisons.ods");
    assert(doc);

    test::range_resolver to_range(ixion::formula_name_resolver_t::odf_cra, doc->get_model_context());

    const ss::tables& t = doc->get_tables();

    {
        auto table = t.get("__Anonymous_Sheet_DB__0").lock();
        assert(table);
        assert(table->range == to_range("Table1.B3:Table1.E13"));

        const ss::auto_filter_t& filter = table->filter;
        assert(filter.range == ixion::abs_rc_range_t(table->range));

        // root {and}
        //   |
        //   +- item {field: 2; ends-with 'USA'}

        assert(filter.root.op() == ss::auto_filter_node_op_t::op_and);
        assert(filter.root.size() == 1);

        {
            const auto* item = dynamic_cast<const ss::filter_item_t*>(filter.root.at(0));
            assert(item);
            assert(item->field() == 2);
            assert(item->op() == ss::auto_filter_op_t::end_with);
            assert(item->value() == ss::filter_value_t("USA"));
        }
    }

    {
        auto table = t.get("__Anonymous_Sheet_DB__1").lock();
        assert(table);
        assert(table->range == to_range("Table2.B3:Table2.E13"));

        const ss::auto_filter_t& filter = table->filter;
        assert(filter.range == ixion::abs_rc_range_t(table->range));

        // root {and}
        //   |
        //   +- item {field: 2; does-not-contain 'USA'}
        //   |
        //   +- item {field: 0; begins-with 'Ancient'}

        assert(filter.root.op() == ss::auto_filter_node_op_t::op_and);
        assert(filter.root.size() == 2);

        {
            const auto* item = dynamic_cast<const ss::filter_item_t*>(filter.root.at(0));
            assert(item);
            assert(item->field() == 2);
            assert(item->op() == ss::auto_filter_op_t::not_contain);
            assert(item->value() == ss::filter_value_t("USA"));
        }

        {
            const auto* item = dynamic_cast<const ss::filter_item_t*>(filter.root.at(1));
            assert(item);
            assert(item->field() == 0);
            assert(item->op() == ss::auto_filter_op_t::begin_with);
            assert(item->value() == ss::filter_value_t("Ancient"));
        }
    }

    {
        auto table = t.get("__Anonymous_Sheet_DB__2").lock();
        assert(table);
        assert(table->range == to_range("Table3.B3:Table3.C23"));

        const ss::auto_filter_t& filter = table->filter;
        assert(filter.range == ixion::abs_rc_range_t(table->range));

        // root {or}
        //   |
        //   +- item {field: 1; not-empty}
        //   |
        //   +- item {field: 0; ends-with 'Kim'}

        assert(filter.root.op() == ss::auto_filter_node_op_t::op_or);
        assert(filter.root.size() == 2);

        {
            const auto* item = dynamic_cast<const ss::filter_item_t*>(filter.root.at(0));
            assert(item);
            assert(item->field() == 1);
            assert(item->op() == ss::auto_filter_op_t::not_empty);
        }

        {
            const auto* item = dynamic_cast<const ss::filter_item_t*>(filter.root.at(1));
            assert(item);
            assert(item->field() == 0);
            assert(item->op() == ss::auto_filter_op_t::end_with);
            assert(item->value() == ss::filter_value_t("Kim"));
        }
    }

    {
        auto table = t.get("__Anonymous_Sheet_DB__3").lock();
        assert(table);
        assert(table->range == to_range("Table4.B3:Table4.C23"));

        const ss::auto_filter_t& filter = table->filter;
        assert(filter.range == ixion::abs_rc_range_t(table->range));

        // root {or}
        //   |
        //   +- item {field: 1; empty}
        //   |
        //   +- item {field: 0; begins-with 'A'}

        assert(filter.root.op() == ss::auto_filter_node_op_t::op_or);
        assert(filter.root.size() == 2);

        {
            const auto* item = dynamic_cast<const ss::filter_item_t*>(filter.root.at(0));
            assert(item);
            assert(item->field() == 1);
            assert(item->op() == ss::auto_filter_op_t::empty);
        }

        {
            const auto* item = dynamic_cast<const ss::filter_item_t*>(filter.root.at(1));
            assert(item);
            assert(item->field() == 0);
            assert(item->op() == ss::auto_filter_op_t::begin_with);
            assert(item->value() == ss::filter_value_t("A"));
        }
    }
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
