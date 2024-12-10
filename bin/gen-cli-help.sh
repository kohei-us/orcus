#!/usr/bin/env bash

THISDIR=$(dirname $0)
ROOTDIR="$THISDIR/.."

CMDS="
orcus-csv
orcus-gnumeric
orcus-json
orcus-ods
orcus-parquet
orcus-xls-xml
orcus-xlsx
orcus-xml
orcus-yaml
"

for CMD in $CMDS; do
    CMD_PATH="$ROOTDIR/src/$CMD"
    DESTFILE="$ROOTDIR/doc/_static/cli/$CMD-help.txt"
    "$CMD_PATH" -h > "$DESTFILE"
done

