#!/usr/bin/env bash
#
# Script to build lists of extra dist files by category.
# Output of this script should go into Makefile.am at the project root.

PROGDIR=`dirname $0`

function print_list()
{
    local _DIR=$1
    local _NAME=$_DIR"_data"

    echo "$_NAME = \\"
    local _BASE_CMD="git ls-tree --full-tree --name-only -r HEAD -- $_DIR"
    $_BASE_CMD | sed -e 's/\#/\\\#/g' | head -n -1 | sed -e 's/^/\t/g' -e 's/$/\ \\/g'
    $_BASE_CMD | sed -e 's/\#/\\\#/g' | tail -n 1 | sed -e 's/^/\t/g'
    echo ""
}

print_list doc
print_list doc_example
print_list bin
print_list misc
print_list slickedit
print_list test
