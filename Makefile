all:
	scons -C cbang
	scons

clean:
	scons -C cbang -c
	scons -c
