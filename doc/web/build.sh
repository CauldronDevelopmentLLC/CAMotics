#!/bin/bash

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
