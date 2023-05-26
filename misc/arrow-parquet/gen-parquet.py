#!/usr/bin/env python3

import pandas as pd
import argparse
from pathlib import Path


def gen_str(pos):
    return gen_str.values[pos]


gen_str.values = (
    "ubergeek",
    "thwarter",
    "ironfist",
    "turkoman",
    "mesozoan",
    "seatsale",
    "hardtack",
    "phyllary",
    "hydriads",
    "stranger",
    "cistuses",
    "capelets",
    "headband",
    "dudesses",
    "aminases",
    "eggwhite",
    "boxscore",
    "upsurges",
    "blowlamp",
    "dionysia",
    "rejecter",
    "keratome",
    "diasters",
    "juddocks",
    "gownsman",
    "sweepsaw",
    "chuckeys",
    "partyers",
    "weredogs",
    "exabytes",
)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--destdir", type=Path, default=Path("../../test/parquet/basic"))
    args = parser.parse_args()

    args.destdir.mkdir(parents=True, exist_ok=True)

    row_size = 10
    data = {
        "int32": [v for v in range(row_size)],
        "int64": [v * 10 + v for v in range(row_size)],
        "float32": [-v for v in range(row_size)],
        "float64": [-v - 21 for v in range(row_size)],
        "boolean": [(v & 0x01) != 0 for v in range(row_size)],
        "string": [gen_str(pos) for pos in range(row_size)],
    }
    df = pd.DataFrame(data=data)
    df["int32"] = df["int32"].astype("int32")
    df["int64"] = df["int64"].astype("int64")
    df["float32"] = df["float32"].astype("float32")
    df["float64"] = df["float64"].astype("float64")

    print(df)
    print(df.dtypes)

    df.to_parquet(args.destdir / f"basic-nocomp.parquet", engine="pyarrow", compression=None)
    for comp in ("gzip", "snappy", "zstd"):
        df.to_parquet(args.destdir / f"basic-{comp}.parquet", engine="pyarrow", compression=comp)


if __name__ == "__main__":
    main()

