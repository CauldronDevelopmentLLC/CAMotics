#!/bin/bash

declare -A PAGES

PAGES[main]="about quick-start mission screenshots status donate community "
PAGES[main]+="legal contact"
PAGES[download]="current install run source debug previous"
PAGES[manual]="overview projects simulation layout playback nc-files tools "
PAGES[manual]+="workpiece export docks toolbars"

# Create Menus
for PAGE in ${!PAGES[@]}; do
    "$(dirname "$0")"/create_menu.sh $PAGE ${PAGES[$PAGE]} \
        >jade/$PAGE/menu.jade
done

# Create Templates
for PAGE in ${!PAGES[@]}; do
    "$(dirname "$0")"/create_template.sh $PAGE ${PAGES[$PAGE]} \
        >jade/$PAGE/template.jade
done

mkdir -p http

for PAGE in ${!PAGES[@]}; do
    TEMPLATE=jade/$PAGE/template.jade
    TARGET=http/$PAGE.html
    echo Building $TARGET from $TEMPLATE...
	jade -P -p $TEMPLATE -O "$(cat config.json)" <$TEMPLATE >$TARGET
done

jade -P <jade/notfound.jade >http/notfound.html

ln -sf main.html http/index.html
ln -sf ../css ../js ../images http/

touch http
