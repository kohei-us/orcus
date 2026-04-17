#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import sys
import pytest
from orcus import json


def test_loads():
    s = '[1,2,3,"foo",[4,5,6], {"a": 12.3, "b": 34.4, "c": [true, false, null]}]'
    o = json.loads(s)
    assert isinstance(o, list)
    assert len(o) == 6
    assert o[0] == 1
    assert o[1] == 2
    assert o[2] == 3
    assert o[3] == "foo"

    assert isinstance(o[4], list)
    assert o[4][0] == 4
    assert o[4][1] == 5
    assert o[4][2] == 6

    d = o[5]
    assert isinstance(d, dict)
    assert len(d) == 3
    assert d["a"] == 12.3
    assert d["b"] == 34.4

    l = d["c"]
    assert len(l) == 3
    assert l[0] is True
    assert l[1] is False
    assert l[2] is None


if __name__ == "__main__":
    sys.exit(pytest.main([__file__]))
