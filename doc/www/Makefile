all: http

http: $(shell find jade) config.json
	./build.sh

clean:
	rm -rf http jade/{manual,main,download}/{template,menu}.jade

publish: http
	rsync -Lav http/ root@openscam.org:/var/www/openscam.org/http/
