#!/bin/bash

PAGE="$1"
shift 1
SECTIONS="$@"

echo "- var page = '$PAGE';"
cat jade/template_head.jade
echo

for i in $SECTIONS; do
    echo "        .docs-section"
    echo "          include $i"
done

echo
cat jade/template_tail.jade
