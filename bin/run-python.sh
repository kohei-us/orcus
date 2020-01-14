#!/usr/bin/env bash

if [ "$1" == "" ]; then
    echo "no input file"
    exit 1
fi

RUNMODE=

if [ "$1" == "gdb" ]; then
    RUNMODE=gdb
    shift
elif [ "$1" == "valgrind" ]; then
    RUNMODE=valgrind
    shift
fi

if [ ! -e "$1" ]; then
    echo "file '$1' does not exist"
    exit 1
fi

PYTHON=$(which python3)
PROGDIR=`dirname $0`
_PYTHONPATH="$PROGDIR/../src/python/.libs:$PROGDIR/../src/python"

export PYTHONPATH=$_PYTHONPATH:$PYTHONPATH
export LD_LIBRARY_PATH="$PROGDIR/../src/liborcus/.libs:$PROGDIR/../src/parser/.libs"
export DYLD_LIBRARY_PATH=$LD_LIBRARY_PATH

EXEC="$1"
shift

case $RUNMODE in
    gdb)
    gdb --args $PYTHON "$PWD/$EXEC" "$@"
        ;;
    valgrind)
    valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes $PYTHON "$PWD/$EXEC" "$@"
        ;;
    *)
        exec "$PWD/$EXEC" "$@"
        ;;
esac


