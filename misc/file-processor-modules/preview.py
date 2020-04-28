########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import orcus
from orcus.tools.file_processor import config


def process_document(filepath, doc):
    buf = list()
    for sh in doc.sheets:
        try:
            buf.append(f"sheet: {sh.name}")
            for i, row in enumerate(sh.get_rows()):
                if i > 9:
                    # Only display the first 10 rows.
                    buf.append("...")
                    break

                row_s = list()
                for cell in row:
                    v = cell.value if cell.value else ""
                    row_s.append(str(v))
                row_s = ",".join(row_s)
                buf.append(f"row {i}: {row_s}")
        except Exception as e:
            buf.append(f"???: (exception: {e})")

    return buf
