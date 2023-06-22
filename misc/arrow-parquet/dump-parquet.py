#!/usr/bin/env python3
########################################################################
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
########################################################################

import argparse
from pathlib import Path

import pyarrow.parquet as pq


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("path", type=Path)
    parser.add_argument("--num", "-n", type=int, default=10, help="Number of rows to print.")
    args = parser.parse_args()

    parquet = pq.ParquetFile(args.path)
    print(f"num-row-groups: {parquet.metadata.num_row_groups}")
    print(f"num-rows: {parquet.metadata.num_rows}")
    print(f"num-columns: {parquet.metadata.num_columns}")
    print("schema:")
    for i, name in enumerate(parquet.metadata.schema.names):
        col = parquet.metadata.schema.column(i)
        print(f"  column {i}:")
        for attr_name in dir(col):
            if attr_name.startswith("_"):
                continue
            attr_value = getattr(col, attr_name)
            if callable(attr_value):
                continue
            print(f"    {attr_name}: {attr_value}")

    for icol, (name, chunked_array) in enumerate(zip(parquet.metadata.schema.names, parquet.read_row_group(0))):
        print(f"column {icol}:")
        print(f"  name: {name}")
        print(f"  type: {chunked_array.type}")
        print(f"  num-chunks: {chunked_array.num_chunks}")
        print(f"  data:")
        for i, v in enumerate(chunked_array.chunks[0]):
            if i == args.num:
                break
            print(f"    - {v}")


if __name__ == "__main__":
    main()

