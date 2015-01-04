DEST=root@coffland.com:/var/www/tplang.org/http/
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(patsubst %/,%,$(dir $(mkfile_path)))

all: tpl.html tpl.css

%.html: %.jade
	jade -P $<

%.css: %.styl
	stylus <$<>$@

publish: all
	rsync -av --exclude=*~ "$(current_dir)"/ $(DEST)
