#!/usr/bin/env python3

import numpy as np
import pandas as pd
import pyarrow as pa


def main():
    df = pd.DataFrame({'one': ["andy", "bruce", "charlie", "david", "edward"],
                       'two': ['foo', 'bar', 'baz', "hmm", "more"],
                       'three': [True, False, True, True, False],
                       'four': [11, 22, 33, 44, 55]},
                       index=list('abcde'))

    print(df)
    table = pa.Table.from_pandas(df)
    import pyarrow.parquet as pq

    pq.write_table(table, 'test.parquet')



if __name__ == "__main__":
    main()

