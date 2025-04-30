all:
	scons -C cbang
	scons

clean:
	scons -C cbang -c
	scons -c
	rm -rf matrix/debs

# build debian packages in docker images
debs:
	python matrix/build_matrix.py
