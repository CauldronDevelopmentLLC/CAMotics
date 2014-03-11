all: http

http: $(shell find jade) config.json
	./build.sh

clean:
	rm -rf http jade/{manual,main}/{template,menu}.jade

publish: http
	rsync -av http/ root@openscam.org:/var/www/openscam.org/http/
