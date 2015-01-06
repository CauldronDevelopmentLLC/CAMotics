TARGETS=tpl.html tpl.css
DEST=root@coffland.com:/var/www/tplang.org/http/
mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(patsubst %/,%,$(dir $(mkfile_path)))

all: $(TARGETS)

%.html: %.jade
	jade -P $<

%.css: %.styl
	stylus <$<>$@

publish: all
	rsync -av --exclude=*~ "$(current_dir)"/ $(DEST)

tidy:
	rm -f *~

clean: tidy
	rm -f $(TARGETS)
