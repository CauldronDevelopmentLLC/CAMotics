#!/bin/bash

MAIN="about downloads screenshots status donate community legal contact"

MANUAL="overview projects simulation layout playback nc-files tools workpiece "
MANIAL+="export docks toolbars"

# Create Menus
"$(dirname "$0")"/create_menu.sh main $MAIN >jade/main/menu.jade
"$(dirname "$0")"/create_menu.sh manual $MANUAL >jade/manual/menu.jade

# Create Templates
"$(dirname "$0")"/create_template.sh main $MAIN >jade/main/template.jade
"$(dirname "$0")"/create_template.sh manual $MANUAL >jade/manual/template.jade

mkdir -p http

for i in manual main; do
    TEMPLATE=jade/$i/template.jade
    TARGET=http/$i.html
    echo Building $TARGET from $TEMPLATE...
	jade -P -p $TEMPLATE -O "$(cat config.json)" <$TEMPLATE >$TARGET
done

ln -sf main.html http/index.html
ln -sf ../css ../js ../images http/

touch http
