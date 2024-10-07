/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "orcus_gnumeric_test.hpp"

void test_gnumeric_auto_filter()
{
    ORCUS_TEST_FUNC_SCOPE;
#if 0 // TODO: fix this
    fs::path filepath = SRCDIR"/test/gnumeric/table/autofilter.gnumeric";
    auto doc = load_doc(filepath);

    assert(doc->get_sheet_count() == 1);
    const ss::sheet* sh = doc->get_sheet(0);
    assert(sh);

    const ss::old::auto_filter_t* af = sh->get_auto_filter_data();
    assert(af);
    ixion::abs_range_t b2_c11{0, 1, 1, 10, 2};
    assert(af->range == b2_c11);
    assert(af->columns.size() == 2);

    auto it = af->columns.begin();
    assert(it->first == 0);
    {
        const ss::old::auto_filter_column_t& afc = it->second;
        assert(afc.match_values.size() == 1);
        assert(*afc.match_values.begin() == "A");
    }

    ++it;
    assert(it->first == 1);
    {
        const ss::old::auto_filter_column_t& afc = it->second;
        assert(afc.match_values.size() == 1);
        assert(*afc.match_values.begin() == "1");
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
