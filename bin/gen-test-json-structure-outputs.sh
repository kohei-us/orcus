#!/usr/bin/env bash

PROGDIR=`dirname $0`
TESTDIR="$PROGDIR/../test/json-structure"
ORCUS_JSON_EXEC="$PROGDIR/../src/orcus-json"

for _DIR in $(ls $TESTDIR); do
    echo "processing $_DIR..."
    _DIR="$TESTDIR/$_DIR"
    _INPUT="$_DIR/input.json"
    _OUTPUT="$_DIR/check.txt"
    $ORCUS_JSON_EXEC --mode structure -o "$_OUTPUT" "$_INPUT"
done

