all:
	scons -C cbang
	scons

clean:
	scons -C cbang -c
	scons -c

# build debian packages in docker images
debs:
	python matrix/build_matrix.py
