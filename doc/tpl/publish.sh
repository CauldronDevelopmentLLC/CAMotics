#!/bin/bash
TARGET=root@coffland.com:/var/www/tplang.org/http/

rsync -av --exclude=.svn --exclude=publish.sh "$(dirname "$0")"/ $TARGET
