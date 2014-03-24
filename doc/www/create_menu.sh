#!/bin/bash

PAGE="$1"
shift 1
SECTIONS="$@"

echo "ul.nav.sidenav"

for i in $SECTIONS; do
    FILE=jade/$PAGE/$i.jade
    REF=$(grep "h1#" $FILE | sed 's/[[:space:]]*h1#\([^[:space:]]*\).*$/\1/')
    TITLE=$(grep "h1#" $FILE | sed 's/[[:space:]]*h1#[^[:space:]]* \(.*\)$/\1/')

    echo "  li"
    echo "    a(href='#$REF') $TITLE"

    # Subsections
    FIRST=true
    grep "h2#" $FILE | while read LINE; do
        if $FIRST; then
            FIRST=false
            echo "    ul.nav"
        fi

        REF=$(echo "$LINE" | sed 's/[[:space:]]*h2#\([^[:space:]]*\).*$/\1/')
        TITLE=$(echo "$LINE" | sed 's/[[:space:]]*h2#[^[:space:]]* \(.*\)$/\1/')

        echo "      li"
        echo "        a(href='#$REF') $TITLE"
    done
done
