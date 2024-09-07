/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_xls_xml_test.hpp"

#include <orcus/spreadsheet/auto_filter.hpp>

namespace ss = orcus::spreadsheet;

void test_xls_xml_auto_filter_number()
{
    ORCUS_TEST_FUNC_SCOPE;

    auto doc = load_doc_from_filepath(SRCDIR"/test/xls-xml/table/autofilter-number.xml");
    assert(doc);

    {
        auto* sh = doc->get_sheet("Greater Than");
        assert(sh);

        auto* filter = sh->get_auto_filter_range();
        assert(filter);

        // R3C2:R96C7
        ixion::abs_rc_range_t range_expected;
        range_expected.first.row = 2;
        range_expected.first.column = 1;
        range_expected.last.row = 95;
        range_expected.last.column = 6;
        assert(filter->range == range_expected);

        // 1: filter-rule: > 20; field: 2

        // The root node should have one child node per filtered field,
        // connected by the 'and' operator.  In this case there is only one
        // filtered field so there should only be one child node.
        assert(filter->filter.root.op == ss::auto_filter_node_op_t::op_and);
        assert(filter->filter.root.children.size() == 1);
        auto* field = dynamic_cast<const ss::filter_node_t*>(filter->filter.root.children[0]);
        assert(field);

        assert(field->children.size() == 1);
        auto* item = dynamic_cast<const ss::filter_item_t*>(field->children[0]);
        assert(item);

        assert(item->field == 2);
        assert(item->op == ss::auto_filter_op_t::greater);
        assert(item->value.type() == ss::filter_value_t::value_type::numeric);
        assert(item->value.numeric() == 20);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
