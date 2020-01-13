#!/usr/bin/env bash

PROGDIR=`dirname $0`
TESTDIR="$PROGDIR/../test/xml"
ORCUS_XML_EXEC="$PROGDIR/../src/orcus-xml"

for _DIR in $(ls $TESTDIR); do
    _DIR="$TESTDIR/$_DIR"
    _INPUT="$_DIR/input.xml"
    _OUTPUT="$_DIR/check.txt"

    if [ ! -f "$_INPUT" ]; then
        continue
    fi

    echo "processing $_DIR..."
    $ORCUS_XML_EXEC --mode dump -o "$_OUTPUT" "$_INPUT"
done

