# Source this script to set dev environment.

ROOTDIR=$(git rev-parse --show-toplevel)
SRCDIR=$ROOTDIR/src

export PATH=$SRCDIR:$PATH
export PYTHONPATH=$SRCDIR/python:$SRCDIR/python/.libs:$PYTHONPATH

